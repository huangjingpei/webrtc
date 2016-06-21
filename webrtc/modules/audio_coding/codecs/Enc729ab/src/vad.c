/*
   ITU-T G.729A Annex B     ANSI-C Source Code
   Version 1.1    Last modified: September 1996
   Copyright (c) 1996, France Telecom, Rockwell International,
                       Universite de Sherbrooke.
   All rights reserved.
*/

#include <stdio.h>
#include "typedef.h"
#include "ld8a.h"
#include "basic_op.h"
#include "oper_32b.h"
#include "tab_ld8a.h"
#include "vad.h"
#include "dtx.h"
#include "tab_dtx.h"

/* local function */
static Word16 MakeDec(
               Word16 dSLE,    /* (i)  : differential low band energy */
               Word16 dSE,     /* (i)  : differential full band energy */
               Word16 SD,      /* (i)  : differential spectral distortion */
               Word16 dSZC     /* (i)  : differential zero crossing rate */
);


/*---------------------------------------------------------------------------*
 * Function  vad_init                                                        *
 * ~~~~~~~~~~~~~~~~~~                                                        *
 *                                                                           *
 * -> Initialization of variables for voice activity detection               *
 *                                                                           *
 *---------------------------------------------------------------------------*/
void vad_init(VadStatus *handle)
{
  /* Static vectors to zero */
  Set_zero(handle->MeanLSF, M);

  /* Initialize VAD parameters */
  handle->MeanSE = 0;
  handle->MeanSLE = 0;
  handle->MeanE = 0;
  handle->MeanSZC = 0;
  handle->count_sil = 0;
  handle->count_update = 0;
  handle->count_ext = 0;
  handle->less_count = 0;
  handle->flag = 1;
  handle->Min = MAX_16;
}


/*-----------------------------------------------------------------*
 * Functions vad                                                   *
 *           ~~~                                                   *
 * Input:                                                          *
 *   rc            : reflection coefficient                        *
 *   lsf[]         : unquantized lsf vector                        *
 *   r_h[]         : upper 16-bits of the autocorrelation vector   *
 *   r_l[]         : lower 16-bits of the autocorrelation vector   *
 *   exp_R0        : exponent of the autocorrelation vector        *
 *   sigpp[]       : preprocessed input signal                     *
 *   frm_count     : frame counter                                 *
 *   prev_marker   : VAD decision of the last frame                *
 *   pprev_marker  : VAD decision of the frame before last frame   *
 *                                                                 *
 * Output:                                                         *
 *                                                                 *
 *   marker        : VAD decision of the current frame             * 
 *                                                                 *
 *-----------------------------------------------------------------*/
void vad(
         VadStatus *handle,
         Word16 rc,
         Word16 *lsf, 
         Word16 *r_h,
         Word16 *r_l, 
         Word16 exp_R0,
         Word16 *sigpp,
         Word16 frm_count,
         Word16 prev_marker,
         Word16 pprev_marker,
         Word16 *marker)
{
 /* scalar */
  Word32 acc0;
  Word16 i, j, exp, frac;
  Word16 ENERGY, ENERGY_low, SD, ZC, dSE, dSLE, dSZC;
  Word16 COEF, C_COEF, COEFZC, C_COEFZC, COEFSD, C_COEFSD;

  /* compute the frame energy */
  acc0 = L_Comp(r_h[0], r_l[0]);
  Log2(acc0, &exp, &frac);
  acc0 = Mpy_32_16(exp, frac, 9864);
  i = sub(exp_R0, 1);  
  i = sub(i, 1);
  acc0 = L_mac(acc0, 9864, i);
  acc0 = L_shl(acc0, 11);
  ENERGY = extract_h(acc0);
  ENERGY = sub(ENERGY, 4875);

  /* compute the low band energy */
  acc0 = 0;
  for (i=1; i<=NP; i++)
    acc0 = L_mac(acc0, r_h[i], lbf_corr[i]);
  acc0 = L_shl(acc0, 1);
  acc0 = L_mac(acc0, r_h[0], lbf_corr[0]);
  Log2(acc0, &exp, &frac);
  acc0 = Mpy_32_16(exp, frac, 9864);
  i = sub(exp_R0, 1);  
  i = sub(i, 1);
  acc0 = L_mac(acc0, 9864, i);
  acc0 = L_shl(acc0, 11);
  ENERGY_low = extract_h(acc0);
  ENERGY_low = sub(ENERGY_low, 4875);
  
  /* compute SD */
  acc0 = 0;
  for (i=0; i<M; i++){
    j = sub(lsf[i], handle->MeanLSF[i]);
    acc0 = L_mac(acc0, j, j);
  }
  SD = extract_h(acc0);      /* Q15 */
  
  /* compute # zero crossing */
  ZC = 0;
  for (i=ZC_START+1; i<=ZC_END; i++)
    if (mult(sigpp[i-1], sigpp[i]) < 0)
      ZC = add(ZC, 410);     /* Q15 */

  /* Initialize and update Mins */
  if(frm_count < 129){
    if (ENERGY < handle->Min){
      handle->Min = ENERGY;
      handle->Prev_Min = ENERGY;
    }
    
    if((frm_count & 0x0007) == 0){
      i = sub(shr(frm_count,3),1);
      handle->Min_buffer[i] = handle->Min;
      handle->Min = MAX_16;
    }
  }

  if((frm_count & 0x0007) == 0){
    handle->Prev_Min = handle->Min_buffer[0];
    for (i=1; i<16; i++){
      if (handle->Min_buffer[i] < handle->Prev_Min)
        handle->Prev_Min = handle->Min_buffer[i];
    }
  }
  
  if(frm_count >= 129){
    if(((frm_count & 0x0007) ^ (0x0001)) == 0){
      handle->Min = handle->Prev_Min;
      handle->Next_Min = MAX_16;
    }
    if (ENERGY < handle->Min)
      handle->Min = ENERGY;
    if (ENERGY < handle->Next_Min)
      handle->Next_Min = ENERGY;
    
    if((frm_count & 0x0007) == 0){
      for (i=0; i<15; i++)
        handle->Min_buffer[i] = handle->Min_buffer[i+1]; 
      handle->Min_buffer[15] = handle->Next_Min; 
      handle->Prev_Min = handle->Min_buffer[0];
      for (i=1; i<16; i++) 
        if (handle->Min_buffer[i] < handle->Prev_Min)
          handle->Prev_Min = handle->Min_buffer[i];
    }
    
  }

  if (frm_count <= INIT_FRAME)
  {
    if(ENERGY < 3072){
      *marker = NOISE;
      handle->less_count++;
    }
    else{
      *marker = VOICE;
      acc0 = L_deposit_h(handle->MeanE);
      acc0 = L_mac(acc0, ENERGY, 1024);
      handle->MeanE = extract_h(acc0);
      acc0 = L_deposit_h(handle->MeanSZC);
      acc0 = L_mac(acc0, ZC, 1024);
      handle->MeanSZC = extract_h(acc0);
      for (i=0; i<M; i++){
        acc0 = L_deposit_h(handle->MeanLSF[i]);
        acc0 = L_mac(acc0, lsf[i], 1024);
        handle->MeanLSF[i] = extract_h(acc0);
      }
    }
  }
  if (frm_count >= INIT_FRAME){
    if (frm_count == INIT_FRAME){
      acc0 = L_mult(handle->MeanE, factor_fx[handle->less_count]);
      acc0 = L_shl(acc0, shift_fx[handle->less_count]);
      handle->MeanE = extract_h(acc0);

      acc0 = L_mult(handle->MeanSZC, factor_fx[handle->less_count]);
      acc0 = L_shl(acc0, shift_fx[handle->less_count]);
      handle->MeanSZC = extract_h(acc0);

      for (i=0; i<M; i++){
        acc0 = L_mult(handle->MeanLSF[i], factor_fx[handle->less_count]);
        acc0 = L_shl(acc0, shift_fx[handle->less_count]);
        handle->MeanLSF[i] = extract_h(acc0);
      }

      if (handle->MeanE <= 10240){  /* 5.0 in Q11 */
        handle->MeanSE = handle->MeanE; 
        handle->MeanSLE = sub(handle->MeanE, 410);  /* 0.2 in Q11 */
      }
      else 
        if(handle->MeanE < 11264){
          handle->MeanSE = sub(handle->MeanE, 1024);   /* Q11 */
          handle->MeanSLE = sub(handle->MeanE, 1434);  /* Q11 */
        }
        else{
          handle->MeanSE = sub(handle->MeanE, 2048);   /* Q11 */
          handle->MeanSLE = sub(handle->MeanE, 2458);  /* Q11 */
        }
    }

    dSE = sub(handle->MeanSE, ENERGY);
    dSLE = sub(handle->MeanSLE, ENERGY_low);
    dSZC = sub(handle->MeanSZC, ZC);

    if(ENERGY < 3072)
      *marker = NOISE;
    else 
      *marker = MakeDec(dSLE, dSE, SD, dSZC);

    handle->v_flag = 0;
    if((prev_marker==VOICE) && (*marker==NOISE) && (add(dSE,410) < 0) 
       && (ENERGY > 3072)){
      *marker = VOICE;
      handle->v_flag = 1;
    }

    if(handle->flag == 1){
      if((pprev_marker == VOICE) && 
         (prev_marker == VOICE) && 
         (*marker == NOISE) && 
         (abs_s(sub(handle->prev_energy,ENERGY)) <= 614)){
        handle->count_ext++;
        *marker = VOICE;
        handle->v_flag = 1;
        if(handle->count_ext <= 4)
          handle->flag=1;
        else{
          handle->count_ext=0;
          handle->flag=0;
        }
      }
    }
    else
      handle->flag=1;
    
    if(*marker == NOISE)
      handle->count_sil++;

    if((*marker == VOICE) && (handle->count_sil > 10) && 
       (sub(ENERGY,handle->prev_energy) <= 614)){
      *marker = NOISE;
      handle->count_sil=0;
    }

    if(*marker == VOICE)
      handle->count_sil=0;

    if ((sub(ENERGY, 614) < handle->MeanSE) && (frm_count > 128)
        && (!handle->v_flag) && (rc < 19661))
      *marker = NOISE;

    if ((sub(ENERGY,614) < handle->MeanSE) && (rc < 24576)
        && (SD < 83)){ 
      handle->count_update++;
      if (handle->count_update < INIT_COUNT){
        COEF = 24576;
        C_COEF = 8192;
        COEFZC = 26214;
        C_COEFZC = 6554;
        COEFSD = 19661;
        C_COEFSD = 13017;
      } 
      else
        if (handle->count_update < (INIT_COUNT+10)){
          COEF = 31130;
          C_COEF = 1638;
          COEFZC = 30147;
          C_COEFZC = 2621;
          COEFSD = 21299;
          C_COEFSD = 11469;
        }
        else
          if (handle->count_update < (INIT_COUNT+20)){
            COEF = 31785;
            C_COEF = 983;
            COEFZC = 30802;
            C_COEFZC = 1966;
            COEFSD = 22938;
            C_COEFSD = 9830;
          }
          else
            if (handle->count_update < (INIT_COUNT+30)){
              COEF = 32440;
              C_COEF = 328;
              COEFZC = 31457;
              C_COEFZC = 1311;
              COEFSD = 24576;
              C_COEFSD = 8192;
            }
            else
              if (handle->count_update < (INIT_COUNT+40)){
                COEF = 32604;
                C_COEF = 164;
                COEFZC = 32440;
                C_COEFZC = 328;
                COEFSD = 24576;
                C_COEFSD = 8192;
              }
              else{
                COEF = 32604;
                C_COEF = 164;
                COEFZC = 32702;
                C_COEFZC = 66;
                COEFSD = 24576;
                C_COEFSD = 8192;
              }
      

      /* compute MeanSE */
      acc0 = L_mult(COEF, handle->MeanSE);
      acc0 = L_mac(acc0, C_COEF, ENERGY);
      handle->MeanSE = extract_h(acc0);

      /* compute MeanSLE */
      acc0 = L_mult(COEF, handle->MeanSLE);
      acc0 = L_mac(acc0, C_COEF, ENERGY_low);
      handle->MeanSLE = extract_h(acc0);

      /* compute MeanSZC */
      acc0 = L_mult(COEFZC, handle->MeanSZC);
      acc0 = L_mac(acc0, C_COEFZC, ZC);
      handle->MeanSZC = extract_h(acc0);
      
      /* compute MeanLSF */
      for (i=0; i<M; i++){
        acc0 = L_mult(COEFSD, handle->MeanLSF[i]);
        acc0 = L_mac(acc0, C_COEFSD, lsf[i]);
        handle->MeanLSF[i] = extract_h(acc0);
      }
    }

    if((frm_count > 128) && (handle->MeanSE < handle->Min) && (SD < 83)){ 
      handle->MeanSE = handle->Min;
      handle->count_update = 0;
    }

  }
  
  handle->prev_energy = ENERGY;

}

/* local function */  
static Word16 MakeDec(
               Word16 dSLE,    /* (i)  : differential low band energy */
               Word16 dSE,     /* (i)  : differential full band energy */
               Word16 SD,      /* (i)  : differential spectral distortion */
               Word16 dSZC     /* (i)  : differential zero crossing rate */
               )
{
  Word32 acc0;
  
  /* SD vs dSZC */
  acc0 = L_mult(dSZC, -14680);          /* Q15*Q23*2 = Q39 */  
  acc0 = L_mac(acc0, 8192, -28521);     /* Q15*Q23*2 = Q39 */
  acc0 = L_shr(acc0, 8);                /* Q39 -> Q31 */
  acc0 = L_add(acc0, L_deposit_h(SD));
  if (acc0 > 0) return(VOICE);

  acc0 = L_mult(dSZC, 19065);           /* Q15*Q22*2 = Q38 */
  acc0 = L_mac(acc0, 8192, -19446);     /* Q15*Q22*2 = Q38 */
  acc0 = L_shr(acc0, 7);                /* Q38 -> Q31 */
  acc0 = L_add(acc0, L_deposit_h(SD));
  if (acc0 > 0) return(VOICE);

  /* dSE vs dSZC */
  acc0 = L_mult(dSZC, 20480);           /* Q15*Q13*2 = Q29 */
  acc0 = L_mac(acc0, 8192, 16384);      /* Q13*Q15*2 = Q29 */
  acc0 = L_shr(acc0, 2);                /* Q29 -> Q27 */
  acc0 = L_add(acc0, L_deposit_h(dSE));
  if (acc0 < 0) return(VOICE);

  acc0 = L_mult(dSZC, -16384);          /* Q15*Q13*2 = Q29 */
  acc0 = L_mac(acc0, 8192, 19660);      /* Q13*Q15*2 = Q29 */
  acc0 = L_shr(acc0, 2);                /* Q29 -> Q27 */
  acc0 = L_add(acc0, L_deposit_h(dSE));
  if (acc0 < 0) return(VOICE);
 
  acc0 = L_mult(dSE, 32767);            /* Q11*Q15*2 = Q27 */
  acc0 = L_mac(acc0, 1024, 30802);      /* Q10*Q16*2 = Q27 */
  if (acc0 < 0) return(VOICE);
  
  /* dSE vs SD */
  acc0 = L_mult(SD, -28160);            /* Q15*Q5*2 = Q22 */
  acc0 = L_mac(acc0, 64, 19988);        /* Q6*Q14*2 = Q22 */
  acc0 = L_mac(acc0, dSE, 512);         /* Q11*Q9*2 = Q22 */
  if (acc0 < 0) return(VOICE);

  acc0 = L_mult(SD, 32767);             /* Q15*Q15*2 = Q31 */
  acc0 = L_mac(acc0, 32, -30199);       /* Q5*Q25*2 = Q31 */
  if (acc0 > 0) return(VOICE);

  /* dSLE vs dSZC */
  acc0 = L_mult(dSZC, -20480);          /* Q15*Q13*2 = Q29 */
  acc0 = L_mac(acc0, 8192, 22938);      /* Q13*Q15*2 = Q29 */
  acc0 = L_shr(acc0, 2);                /* Q29 -> Q27 */
  acc0 = L_add(acc0, L_deposit_h(dSE));
  if (acc0 < 0) return(VOICE);

  acc0 = L_mult(dSZC, 23831);           /* Q15*Q13*2 = Q29 */
  acc0 = L_mac(acc0, 4096, 31576);      /* Q12*Q16*2 = Q29 */
  acc0 = L_shr(acc0, 2);                /* Q29 -> Q27 */
  acc0 = L_add(acc0, L_deposit_h(dSE));
  if (acc0 < 0) return(VOICE);
  
  acc0 = L_mult(dSE, 32767);            /* Q11*Q15*2 = Q27 */
  acc0 = L_mac(acc0, 2048, 17367);      /* Q11*Q15*2 = Q27 */
  if (acc0 < 0) return(VOICE);
  
  /* dSLE vs SD */
  acc0 = L_mult(SD, -22400);            /* Q15*Q4*2 = Q20 */
  acc0 = L_mac(acc0, 32, 25395);        /* Q5*Q14*2 = Q20 */
  acc0 = L_mac(acc0, dSLE, 256);        /* Q11*Q8*2 = Q20 */
  if (acc0 < 0) return(VOICE);
  
  /* dSLE vs dSE */
  acc0 = L_mult(dSE, -30427);           /* Q11*Q15*2 = Q27 */
  acc0 = L_mac(acc0, 256, -29959);      /* Q8*Q18*2 = Q27 */
  acc0 = L_add(acc0, L_deposit_h(dSLE));
  if (acc0 > 0) return(VOICE);

  acc0 = L_mult(dSE, -23406);           /* Q11*Q15*2 = Q27 */
  acc0 = L_mac(acc0, 512, 28087);       /* Q19*Q17*2 = Q27 */
  acc0 = L_add(acc0, L_deposit_h(dSLE));
  if (acc0 < 0) return(VOICE);

  acc0 = L_mult(dSE, 24576);            /* Q11*Q14*2 = Q26 */
  acc0 = L_mac(acc0, 1024, 29491);      /* Q10*Q15*2 = Q26 */
  acc0 = L_mac(acc0, dSLE, 16384);      /* Q11*Q14*2 = Q26 */
  if (acc0 < 0) return(VOICE);

  return (NOISE);
}




