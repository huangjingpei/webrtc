/*
   ITU-T G.729A Speech Coder with Annex B    ANSI-C Source Code
   Version 1.1    Last modified: September 1996

   Copyright (c) 1996,
   AT&T, France Telecom, NTT, Universite de Sherbrooke, Lucent Technologies,
   Rockwell International
   All rights reserved.
*/

/*-----------------------------------------------------------------*
 *   Functions Coder_ld8a and Init_Coder_ld8a                      *
 *             ~~~~~~~~~~     ~~~~~~~~~~~~~~~                      *
 *                                                                 *
 *  Init_Coder_ld8a(void);                                         *
 *                                                                 *
 *   ->Initialization of variables for the coder section.          *
 *                                                                 *
 *                                                                 *
 *  Coder_ld8a(Word16 ana[]);                                      *
 *                                                                 *
 *   ->Main coder function.                                        *
 *                                                                 *
 *                                                                 *
 *  Input:                                                         *
 *                                                                 *
 *    80 speech data should have beee copy to vector new_speech[]. *
 *    This vector is global and is declared in this function.      *
 *                                                                 *
 *  Ouputs:                                                        *
 *                                                                 *
 *    ana[]      ->analysis parameters.                            *
 *                                                                 *
 *-----------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>

#include "typedef.h"
#include "basic_op.h"
#include "ld8a.h"
#include "vad.h"
#include "dtx.h"
#include "sid.h"

/*-----------------------------------------------------------*
 *    Coder constant parameters (defined in "ld8a.h")        *
 *-----------------------------------------------------------*
 *   L_WINDOW    : LPC analysis window size.                 *
 *   L_NEXT      : Samples of next frame needed for autocor. *
 *   L_FRAME     : Frame size.                               *
 *   L_SUBFR     : Sub-frame size.                           *
 *   M           : LPC order.                                *
 *   MP1         : LPC order+1                               *
 *   L_TOTAL     : Total size of speech buffer.              *
 *   PIT_MIN     : Minimum pitch lag.                        *
 *   PIT_MAX     : Maximum pitch lag.                        *
 *   L_INTERPOL  : Length of filter for interpolation        *
 *-----------------------------------------------------------*/

static Word16 lsp_old_init[M]={
              30000, 26000, 21000, 15000, 8000, 0, -8000,-15000,-21000,-26000};
static Word16 old_A_init[M+1]={4096,0,0,0,0,0,0,0,0,0,0};
static Word16 past_qua_en_init[4] = { -14336, -14336, -14336, -14336 };
              
void InitEnc729(void **pHandle, unsigned int nTimeGap)
{
    EncStatus *handle = NULL;

    *pHandle = NULL;

    if (nTimeGap % 10)  // must be dividable by 10(ms)
    {
        return;
    }
    
    handle = (EncStatus *)malloc(sizeof(EncStatus));
    if (handle == NULL)
    {
        return;
    }

    memset(handle, 0, sizeof(EncStatus));

    handle->frame_num  = nTimeGap / 10;
    handle->vad_enable = 0; // 1; 暂时不支持 VAD-DTX

    memcpy(handle->lsp_old, lsp_old_init, M*sizeof(Word16));
    memcpy(handle->past_qua_en, past_qua_en_init, 4*sizeof(Word16));
    memcpy(handle->dtx.old_A, old_A_init, (M+1)*sizeof(Word16));
    
    Init_Pre_Process(&(handle->prp));
    Init_Coder_ld8a(handle);

    /* for G.729B */
    Init_Cod_cng(&(handle->dtx));

    *pHandle = (void *)handle;
}

void FreeEnc729(void **pHandle)
{
    free((*pHandle));
    
    *pHandle = NULL;
}

int Enc729(void *handle,              /* i/o   :  State structure           */
     short *speech8k,                     /* input :  speech samples (at 8 kHz) */
     unsigned char *bytes                  /* output:  output parameters         */      
)
{
    Word16 prm[PRM_SIZE+1];        /* Analysis parameters + frame type      */
    Word16 serial[SERIAL_SIZE];    /* Output bitstream buffer               */    
    int i, k, bitnum;
    EncStatus *pEnc = (EncStatus *)handle; 
    unsigned char *Vout = bytes;
    unsigned char sprm[10];
    
    for (i=0, bitnum=0; i<pEnc->frame_num; i++)
    {
        if (pEnc->frame == 32767) pEnc->frame = 256;
        else pEnc->frame++;

        memcpy(pEnc->new_speech, speech8k + i*L_FRAME, L_FRAME*sizeof(short));
        
        Pre_Process(&(pEnc->prp), pEnc->new_speech, L_FRAME);
        Coder_ld8a(pEnc, prm, pEnc->frame, pEnc->vad_enable);
        prm2bits_ld8k(prm, serial);

        memset(sprm, 0, 10);
        for (k = 0 ; k < serial[1]; k++ )
            sprm[k>>3] ^= serial[k+2] << (k & 0x0007) ;

        memcpy(Vout, sprm, (serial[1] >> 3));
        Vout += (serial[1] >> 3); // OCTET TX MODE
        
        bitnum += serial[1];
    }

    return bitnum;
}

/*-----------------------------------------------------------------*
 *   Function  Init_Coder_ld8a                                     *
 *            ~~~~~~~~~~~~~~~                                      *
 *                                                                 *
 *  Init_Coder_ld8a(void);                                         *
 *                                                                 *
 *   ->Initialization of variables for the coder section.          *
 *       - initialize pointers to speech buffer                    *
 *       - initialize static  pointers                             *
 *       - set static vectors to zero                              *
 *                                                                 *
 *-----------------------------------------------------------------*/

void Init_Coder_ld8a(EncStatus *handle)
{

  /*----------------------------------------------------------------------*
  *      Initialize pointers to speech vector.                            *
  *                                                                       *
  *                                                                       *
  *   |--------------------|-------------|-------------|------------|     *
  *     previous speech           sf1           sf2         L_NEXT        *
  *                                                                       *
  *   <----------------  Total speech vector (L_TOTAL)   ----------->     *
  *   <----------------  LPC analysis window (L_WINDOW)  ----------->     *
  *   |                   <-- present frame (L_FRAME) -->                 *
  * old_speech            |              <-- new speech (L_FRAME) -->     *
  * p_window              |              |                                *
  *                     speech           |                                *
  *                             new_speech                                *
  *-----------------------------------------------------------------------*/

  handle->new_speech = handle->old_speech + L_TOTAL - L_FRAME;         /* New speech     */
  handle->speech     = handle->new_speech - L_NEXT;                    /* Present frame  */
  handle->p_window   = handle->old_speech + L_TOTAL - L_WINDOW;        /* For LPC window */

  /* Initialize static pointers */

  handle->wsp    = handle->old_wsp + PIT_MAX;
  handle->exc    = handle->old_exc + PIT_MAX + L_INTERPOL;

  /* Static vectors to zero */

  Set_zero(handle->old_speech, L_TOTAL);
  Set_zero(handle->old_exc, PIT_MAX+L_INTERPOL);
  Set_zero(handle->old_wsp, PIT_MAX);
  Set_zero(handle->mem_w,   M);
  Set_zero(handle->mem_w0,  M);
  Set_zero(handle->mem_zero, M);
  handle->sharp = SHARPMIN;

  /* Initialize lsp_old_q[] */

  Copy(handle->lsp_old, handle->lsp_old_q, M);
  Lsp_encw_reset(handle);
  Init_exc_err(handle->dtx.L_exc_err);

  /* For G.729B */
  /* Initialize VAD/DTX parameters */
  handle->pastVad = 1;
  handle->ppastVad = 1;
  handle->seed = INIT_SEED;
  vad_init(&(handle->vad));
  Init_lsfq_noise_enc();

  return;
}

/*-----------------------------------------------------------------*
 *   Functions Coder_ld8a                                          *
 *            ~~~~~~~~~~                                           *
 *  Coder_ld8a(Word16 ana[]);                                      *
 *                                                                 *
 *   ->Main coder function.                                        *
 *                                                                 *
 *                                                                 *
 *  Input:                                                         *
 *                                                                 *
 *    80 speech data should have beee copy to vector new_speech[]. *
 *    This vector is global and is declared in this function.      *
 *                                                                 *
 *  Ouputs:                                                        *
 *                                                                 *
 *    ana[]      ->analysis parameters.                            *
 *                                                                 *
 *-----------------------------------------------------------------*/

void Coder_ld8a(
     EncStatus *handle,
     Word16 ana[],       /* output  : Analysis parameters */
     Word16 frame,       /* input   : frame counter       */
     Word16 vad_enable   /* input   : VAD enable flag     */
)
{
  /* LPC analysis */

  Word16 Aq_t[(MP1)*2];         /* A(z)   quantized for the 2 subframes */
  Word16 Ap_t[(MP1)*2];         /* A(z/gamma)       for the 2 subframes */
  Word16 *Aq, *Ap;              /* Pointer on Aq_t and Ap_t             */

  /* Other vectors */

  Word16 h1[L_SUBFR];            /* Impulse response h1[]              */
  Word16 xn[L_SUBFR];            /* Target vector for pitch search     */
  Word16 xn2[L_SUBFR];           /* Target vector for codebook search  */
  Word16 code[L_SUBFR];          /* Fixed codebook excitation          */
  Word16 y1[L_SUBFR];            /* Filtered adaptive excitation       */
  Word16 y2[L_SUBFR];            /* Filtered fixed codebook excitation */
  Word16 g_coeff[4];             /* Correlations between xn & y1       */

  Word16 g_coeff_cs[5];
  Word16 exp_g_coeff_cs[5];      /* Correlations between xn, y1, & y2
                                     <y1,y1>, -2<xn,y1>,
                                          <y2,y2>, -2<xn,y2>, 2<y1,y2> */

  /* Scalars */

  Word16 i, j, k, i_subfr;
  Word16 T_op, T0, T0_min, T0_max, T0_frac;
  Word16 gain_pit, gain_code, index;
  Word16 temp, taming;
  Word32 L_temp;

/*------------------------------------------------------------------------*
 *  - Perform LPC analysis:                                               *
 *       * autocorrelation + lag windowing                                *
 *       * Levinson-durbin algorithm to find a[]                          *
 *       * convert a[] to lsp[]                                           *
 *       * quantize and code the LSPs                                     *
 *       * find the interpolated LSPs and convert to a[] for the 2        *
 *         subframes (both quantized and unquantized)                     *
 *------------------------------------------------------------------------*/
  {
     /* Temporary vectors */
    Word16 r_l[NP+1], r_h[NP+1];     /* Autocorrelations low and hi          */
    Word16 rc[M];                    /* Reflection coefficients.             */
    Word16 lsp_new[M], lsp_new_q[M]; /* LSPs at 2th subframe                 */

    /* For G.729B */
    Word16 rh_nbe[MP1];             
    Word16 lsf_new[M];
    Word16 lsfq_mem[MA_NP][M];
    Word16 exp_R0, Vad;

    /* LP analysis */
    Autocorr(handle->p_window, NP, r_h, r_l, &exp_R0);     /* Autocorrelations */
    Copy(r_h, rh_nbe, MP1);
    Lag_window(NP, r_h, r_l);                      /* Lag windowing    */
    Levinson(&(handle->dtx), r_h, r_l, Ap_t, rc, &temp);          /* Levinson Durbin  */
    Az_lsp(Ap_t, lsp_new, handle->lsp_old);               /* From A(z) to lsp */

    /* For G.729B */
    /* ------ VAD ------- */
    Lsp_lsf(lsp_new, lsf_new, M);
    vad(&(handle->vad), rc[1], lsf_new, r_h, r_l, exp_R0, handle->p_window, frame, 
        handle->pastVad, handle->ppastVad, &Vad);

    Update_cng(&(handle->dtx), rh_nbe, exp_R0, Vad);
    
    /* ---------------------- */
    /* Case of Inactive frame */
    /* ---------------------- */

    if ((Vad == 0) && (vad_enable == 1)){

      Get_freq_prev(handle, lsfq_mem);
      Cod_cng(&(handle->dtx), handle->exc, handle->pastVad, handle->lsp_old_q, Aq_t, ana, lsfq_mem, &(handle->seed));
      Update_freq_prev(handle, lsfq_mem);
      handle->ppastVad = handle->pastVad;
      handle->pastVad = Vad;

      /* Update wsp, mem_w and mem_w0 */
      Aq = Aq_t;
      for(i_subfr=0; i_subfr < L_FRAME; i_subfr += L_SUBFR) {
        
        /* Residual signal in xn */
        Residu(Aq, &(handle->speech[i_subfr]), xn, L_SUBFR);
        
        Weight_Az(Aq, GAMMA1, M, Ap_t);
        
        /* Compute wsp and mem_w */
        Ap = Ap_t + MP1;
        Ap[0] = 4096;
        for(i=1; i<=M; i++)    /* Ap[i] = Ap_t[i] - 0.7 * Ap_t[i-1]; */
          Ap[i] = sub(Ap_t[i], mult(Ap_t[i-1], 22938));
        Syn_filt(Ap, xn, &(handle->wsp[i_subfr]), L_SUBFR, handle->mem_w, 1);
        
        /* Compute mem_w0 */
        for(i=0; i<L_SUBFR; i++) {
          xn[i] = sub(xn[i], handle->exc[i_subfr+i]);  /* residu[] - exc[] */
        }
        Syn_filt(Ap_t, xn, xn, L_SUBFR, handle->mem_w0, 1);
                
        Aq += MP1;
      }
      
      
      handle->sharp = SHARPMIN;
      
      /* Update memories for next frames */
      Copy(&(handle->old_speech[L_FRAME]), &(handle->old_speech[0]), L_TOTAL-L_FRAME);
      Copy(&(handle->old_wsp[L_FRAME]), &(handle->old_wsp[0]), PIT_MAX);
      Copy(&(handle->old_exc[L_FRAME]), &(handle->old_exc[0]), PIT_MAX+L_INTERPOL);
      
      return;
    }  /* End of inactive frame case */
    


    /* -------------------- */
    /* Case of Active frame */
    /* -------------------- */
    
    /* Case of active frame */
    *ana++ = 1;
    handle->seed = INIT_SEED;
    handle->ppastVad = handle->pastVad;
    handle->pastVad = Vad;

    /* LSP quantization */
    Qua_lsp(handle, lsp_new, lsp_new_q, ana);
    ana += 2;                         /* Advance analysis parameters pointer */

    /*--------------------------------------------------------------------*
     * Find interpolated LPC parameters in all subframes                  *
     * The interpolated parameters are in array Aq_t[].                   *
     *--------------------------------------------------------------------*/

    Int_qlpc(handle->lsp_old_q, lsp_new_q, Aq_t);

    /* Compute A(z/gamma) */

    Weight_Az(&Aq_t[0],   GAMMA1, M, &Ap_t[0]);
    Weight_Az(&Aq_t[MP1], GAMMA1, M, &Ap_t[MP1]);

    /* update the LSPs for the next frame */

    Copy(lsp_new,   handle->lsp_old,   M);
    Copy(lsp_new_q, handle->lsp_old_q, M);
  }

 /*----------------------------------------------------------------------*
  * - Find the weighted input speech w_sp[] for the whole speech frame   *
  * - Find the open-loop pitch delay                                     *
  *----------------------------------------------------------------------*/

  Residu(&Aq_t[0], &(handle->speech[0]), &(handle->exc[0]), L_SUBFR);
  Residu(&Aq_t[MP1], &(handle->speech[L_SUBFR]), &(handle->exc[L_SUBFR]), L_SUBFR);

  {
    Word16 Ap1[MP1];

    Ap = Ap_t;
    Ap1[0] = 4096;
    for(i=1; i<=M; i++)    /* Ap1[i] = Ap[i] - 0.7 * Ap[i-1]; */
       Ap1[i] = sub(Ap[i], mult(Ap[i-1], 22938));
    Syn_filt(Ap1, &(handle->exc[0]), &(handle->wsp[0]), L_SUBFR, handle->mem_w, 1);

    Ap += MP1;
    for(i=1; i<=M; i++)    /* Ap1[i] = Ap[i] - 0.7 * Ap[i-1]; */
       Ap1[i] = sub(Ap[i], mult(Ap[i-1], 22938));
    Syn_filt(Ap1, &(handle->exc[L_SUBFR]), &(handle->wsp[L_SUBFR]), L_SUBFR, handle->mem_w, 1);
  }

  /* Find open loop pitch lag */

  T_op = Pitch_ol_fast(handle->wsp, PIT_MAX, L_FRAME);

  /* Range for closed loop pitch search in 1st subframe */

  T0_min = sub(T_op, 3);
  if (T0_min < PIT_MIN) {
    T0_min = PIT_MIN;
  }

  T0_max = add(T0_min, 6);
  if (T0_max > PIT_MAX)
  {
     T0_max = PIT_MAX;
     T0_min = sub(T0_max, 6);
  }


 /*------------------------------------------------------------------------*
  *          Loop for every subframe in the analysis frame                 *
  *------------------------------------------------------------------------*
  *  To find the pitch and innovation parameters. The subframe size is     *
  *  L_SUBFR and the loop is repeated 2 times.                             *
  *     - find the weighted LPC coefficients                               *
  *     - find the LPC residual signal res[]                               *
  *     - compute the target signal for pitch search                       *
  *     - compute impulse response of weighted synthesis filter (h1[])     *
  *     - find the closed-loop pitch parameters                            *
  *     - encode the pitch delay                                           *
  *     - find target vector for codebook search                           *
  *     - codebook search                                                  *
  *     - VQ of pitch and codebook gains                                   *
  *     - update states of weighting filter                                *
  *------------------------------------------------------------------------*/

  Aq = Aq_t;    /* pointer to interpolated quantized LPC parameters */
  Ap = Ap_t;    /* pointer to weighted LPC coefficients             */

  for (i_subfr = 0;  i_subfr < L_FRAME; i_subfr += L_SUBFR)
  {

    /*---------------------------------------------------------------*
     * Compute impulse response, h1[], of weighted synthesis filter  *
     *---------------------------------------------------------------*/

    h1[0] = 4096;
    Set_zero(&h1[1], L_SUBFR-1);
    Syn_filt(Ap, h1, h1, L_SUBFR, &h1[1], 0);

   /*----------------------------------------------------------------------*
    *  Find the target vector for pitch search:                            *
    *----------------------------------------------------------------------*/

    Syn_filt(Ap, &(handle->exc[i_subfr]), xn, L_SUBFR, handle->mem_w0, 0);

    /*---------------------------------------------------------------------*
     *                 Closed-loop fractional pitch search                 *
     *---------------------------------------------------------------------*/

    T0 = Pitch_fr3_fast(&(handle->exc[i_subfr]), xn, h1, L_SUBFR, T0_min, T0_max,
                    i_subfr, &T0_frac);

    index = Enc_lag3(T0, T0_frac, &T0_min, &T0_max,PIT_MIN,PIT_MAX,i_subfr);

    *ana++ = index;

    if (i_subfr == 0) {
      *ana++ = Parity_Pitch(index);
    }

   /*-----------------------------------------------------------------*
    *   - find filtered pitch exc                                     *
    *   - compute pitch gain and limit between 0 and 1.2              *
    *   - update target vector for codebook search                    *
    *-----------------------------------------------------------------*/

    Syn_filt(Ap, &(handle->exc[i_subfr]), y1, L_SUBFR, handle->mem_zero, 0);

    gain_pit = G_pitch(xn, y1, g_coeff, L_SUBFR);

    /* clip pitch gain if taming is necessary */

    taming = test_err(handle->dtx.L_exc_err, T0, T0_frac);

    if( taming == 1){
      if (gain_pit > GPCLIP) {
        gain_pit = GPCLIP;
      }
    }

    /* xn2[i]   = xn[i] - y1[i] * gain_pit  */

    for (i = 0; i < L_SUBFR; i++)
    {
      L_temp = L_mult(y1[i], gain_pit);
      L_temp = L_shl(L_temp, 1);               /* gain_pit in Q14 */
      xn2[i] = sub(xn[i], extract_h(L_temp));
    }


   /*-----------------------------------------------------*
    * - Innovative codebook search.                       *
    *-----------------------------------------------------*/

    index = ACELP_Code_A(xn2, h1, T0, handle->sharp, code, y2, &i);

    *ana++ = index;        /* Positions index */
    *ana++ = i;            /* Signs index     */


   /*-----------------------------------------------------*
    * - Quantization of gains.                            *
    *-----------------------------------------------------*/

    g_coeff_cs[0]     = g_coeff[0];            /* <y1,y1> */
    exp_g_coeff_cs[0] = negate(g_coeff[1]);    /* Q-Format:XXX -> JPN */
    g_coeff_cs[1]     = negate(g_coeff[2]);    /* (xn,y1) -> -2<xn,y1> */
    exp_g_coeff_cs[1] = negate(add(g_coeff[3], 1)); /* Q-Format:XXX -> JPN */

    Corr_xy2( xn, y1, y2, g_coeff_cs, exp_g_coeff_cs );  /* Q0 Q0 Q12 ^Qx ^Q0 */
                         /* g_coeff_cs[3]:exp_g_coeff_cs[3] = <y2,y2>   */
                         /* g_coeff_cs[4]:exp_g_coeff_cs[4] = -2<xn,y2> */
                         /* g_coeff_cs[5]:exp_g_coeff_cs[5] = 2<y1,y2>  */

    *ana++ = Qua_gain(handle->past_qua_en, code, g_coeff_cs, exp_g_coeff_cs,
                         L_SUBFR, &gain_pit, &gain_code, taming);


   /*------------------------------------------------------------*
    * - Update pitch sharpening "sharp" with quantized gain_pit  *
    *------------------------------------------------------------*/

    handle->sharp = gain_pit;
    if (handle->sharp > SHARPMAX) { handle->sharp = SHARPMAX;         }
    if (handle->sharp < SHARPMIN) { handle->sharp = SHARPMIN;         }

   /*------------------------------------------------------*
    * - Find the total excitation                          *
    * - update filters memories for finding the target     *
    *   vector in the next subframe                        *
    *------------------------------------------------------*/

    for (i = 0; i < L_SUBFR;  i++)
    {
      /* exc[i] = gain_pit*exc[i] + gain_code*code[i]; */
      /* exc[i]  in Q0   gain_pit in Q14               */
      /* code[i] in Q13  gain_cod in Q1                */

      L_temp = L_mult(handle->exc[i+i_subfr], gain_pit);
      L_temp = L_mac(L_temp, code[i], gain_code);
      L_temp = L_shl(L_temp, 1);
      handle->exc[i+i_subfr] = round(L_temp);
    }

    update_exc_err(handle->dtx.L_exc_err, gain_pit, T0);

    for (i = L_SUBFR-M, j = 0; i < L_SUBFR; i++, j++)
    {
      temp       = extract_h(L_shl( L_mult(y1[i], gain_pit),  1) );
      k          = extract_h(L_shl( L_mult(y2[i], gain_code), 2) );
      handle->mem_w0[j]  = sub(xn[i], add(temp, k));
    }

    Aq += MP1;           /* interpolated LPC parameters for next subframe */
    Ap += MP1;

  }

 /*--------------------------------------------------*
  * Update signal for next frame.                    *
  * -> shift to the left by L_FRAME:                 *
  *     speech[], wsp[] and  exc[]                   *
  *--------------------------------------------------*/

  Copy(&(handle->old_speech[L_FRAME]), &(handle->old_speech[0]), L_TOTAL-L_FRAME);
  Copy(&(handle->old_wsp[L_FRAME]), &(handle->old_wsp[0]), PIT_MAX);
  Copy(&(handle->old_exc[L_FRAME]), &(handle->old_exc[0]), PIT_MAX+L_INTERPOL);

  return;
}
