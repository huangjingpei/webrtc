/*
   ITU-T G.729A Annex B     ANSI-C Source Code
   Version 1.1    Last modified: September 1996
   Copyright (c) 1996, France Telecom, Rockwell International,
                       Universite de Sherbrooke.
   All rights reserved.
*/

/*
**
** File:            "dec_cng.c"
**
** Description:     Comfort noise generation
**                  performed at the decoder part
**
*/
/**** Fixed point version ***/

#include <stdio.h>
#include <stdlib.h>
#include "typedef.h"
#include "ld8a.h"
#include "tab_ld8a.h"
#include "basic_op.h"
#include "vad.h"
#include "dtx.h"
#include "sid.h"
#include "tab_dtx.h"


/*---------------------------------------------------------------------------*
 * Function  Init_lsfq_noise_enc                                                 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~                                                 *
 *                                                                           *
 * -> Initialization of variables for the lsf quantization in the SID        *
 *                                                                           *
 *---------------------------------------------------------------------------*/
void Init_lsfq_noise_enc(void)
{
  Word16 i, j;
  Word32 acc0;

  /* initialize the noise_fg */
  for (i=0; i<4; i++)
    Copy(fg[0][i], noise_fg[0][i], M);
  
  for (i=0; i<4; i++)
    for (j=0; j<M; j++){
      acc0 = L_mult(fg[0][i][j], 19660);
      acc0 = L_mac(acc0, fg[1][i][j], 13107);
      noise_fg[1][i][j] = extract_h(acc0);
    }
}

