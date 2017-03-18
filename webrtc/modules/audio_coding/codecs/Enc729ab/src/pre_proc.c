/*
   ITU-T G.729A Speech Coder    ANSI-C Source Code
   Version 1.1    Last modified: September 1996

   Copyright (c) 1996,
   AT&T, France Telecom, NTT, Universite de Sherbrooke, Lucent Technologies
   All rights reserved.
*/

/*------------------------------------------------------------------------*
 * Function Pre_Process()                                                 *
 *                                                                        *
 * Preprocessing of input speech.                                         *
 *   - 2nd order high pass filter with cut off frequency at 140 Hz.       *
 *   - Divide input by two.                                               *
 *-----------------------------------------------------------------------*/

#include "typedef.h"
#include "basic_op.h"
#include "oper_32b.h"

#include "ld8a.h"
#include "tab_ld8a.h"

/*------------------------------------------------------------------------*
 * 2nd order high pass filter with cut off frequency at 140 Hz.           *
 * Designed with SPPACK efi command -40 dB att, 0.25 ri.                  *
 *                                                                        *
 * Algorithm:                                                             *
 *                                                                        *
 *  y[i] = b[0]*x[i]/2 + b[1]*x[i-1]/2 + b[2]*x[i-2]/2                    *
 *                     + a[1]*y[i-1]   + a[2]*y[i-2];                     *
 *                                                                        *
 *     b[3] = {0.92727435E+00, -0.18544941E+01, 0.92727435E+00};          *
 *     a[3] = {0.10000000E+01, 0.19059465E+01, -0.91140240E+00};          *
 *                                                                        *
 *  Input are divided by two in the filtering process.                    *
 *-----------------------------------------------------------------------*/


/* Static values to be preserved between calls */
/* y[] values is keep in double precision      */



/* Initialization of static values */

void Init_Pre_Process(PrpStatus *handle)
{
  handle->y2_hi = 0;
  handle->y2_lo = 0;
  handle->y1_hi = 0;
  handle->y1_lo = 0;
  handle->x0   = 0;
  handle->x1   = 0;
}


void Pre_Process(
  PrpStatus *handle,
  Word16 signal[],    /* input/output signal */
  Word16 lg)          /* length of signal    */
{
  Word16 i, x2;
  Word32 L_tmp;

  for(i=0; i<lg; i++)
  {
     x2 = handle->x1;
     handle->x1 = handle->x0;
     handle->x0 = signal[i];

     /*  y[i] = b[0]*x[i]/2 + b[1]*x[i-1]/2 + b140[2]*x[i-2]/2  */
     /*                     + a[1]*y[i-1] + a[2] * y[i-2];      */

     L_tmp     = Mpy_32_16(handle->y1_hi, handle->y1_lo, a140[1]);
     L_tmp     = L_add(L_tmp, Mpy_32_16(handle->y2_hi, handle->y2_lo, a140[2]));
     L_tmp     = L_mac(L_tmp, handle->x0, b140[0]);
     L_tmp     = L_mac(L_tmp, handle->x1, b140[1]);
     L_tmp     = L_mac(L_tmp, x2, b140[2]);
     L_tmp     = L_shl(L_tmp, 3);      /* Q28 --> Q31 (Q12 --> Q15) */
     signal[i] = round(L_tmp);

     handle->y2_hi = handle->y1_hi;
     handle->y2_lo = handle->y1_lo;
     L_Extract(L_tmp, &(handle->y1_hi), &(handle->y1_lo));
  }
  return;
}