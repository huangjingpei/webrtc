/*
   ITU-T G.729A Speech Coder with Annex B    ANSI-C Source Code
   Version 1.1    Last modified: September 1996

   Copyright (c) 1996,
   AT&T, France Telecom, NTT, Universite de Sherbrooke, Lucent Technologies,
   Rockwell International
   All rights reserved.
*/

/*****************************************************************************/
/* bit stream manipulation routines                                          */
/*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "typedef.h"
#include "ld8a.h"
#include "tab_ld8a.h"
#include "vad.h"
#include "dtx.h"
#include "tab_dtx.h"
#include "octet.h"

/* prototypes for local functions */
static void  int2bin(Word16 value, Word16 no_of_bits, Word16 *bitstream);

/*----------------------------------------------------------------------------
 * prm2bits_ld8k -converts encoder parameter vector into vector of serial bits
 * bits2prm_ld8k - converts serial received bits to  encoder parameter vector
 *
 * The transmitted parameters are:
 *
 *     LPC:     1st codebook           7+1 bit
 *              2nd codebook           5+5 bit
 *
 *     1st subframe:
 *          pitch period                 8 bit
 *          parity check on 1st period   1 bit
 *          codebook index1 (positions) 13 bit
 *          codebook index2 (signs)      4 bit
 *          pitch and codebook gains   4+3 bit
 *
 *     2nd subframe:
 *          pitch period (relative)      5 bit
 *          codebook index1 (positions) 13 bit
 *          codebook index2 (signs)      4 bit
 *          pitch and codebook gains   4+3 bit
 *----------------------------------------------------------------------------
 */
void prm2bits_ld8k(
 Word16   prm[],           /* input : encoded parameters  (PRM_SIZE parameters)  */
  Word16 bits[]           /* output: serial bits (SERIAL_SIZE ) bits[0] = bfi
                                    bits[1] = 80 */
)
{
  Word16 i;
  *bits++ = SYNC_WORD;     /* bit[0], at receiver this bits indicates BFI */

  switch(prm[0]){

    /* not transmitted */
  case 0 : {
    *bits = RATE_0;
    break;
  }

  case 1 : {
    *bits++ = RATE_8000;
    for (i = 0; i < PRM_SIZE; i++) {
      int2bin(prm[i+1], bitsno[i], bits);
      bits += bitsno[i];
    }
    break;
  }

  case 2 : {

#ifndef OCTET_TX_MODE
    *bits++ = RATE_SID;
    for (i = 0; i < 4; i++) {
      int2bin(prm[i+1], bitsno2[i], bits);
      bits += bitsno2[i];
    }
#else
    *bits++ = RATE_SID_OCTET;
    for (i = 0; i < 4; i++) {
      int2bin(prm[i+1], bitsno2[i], bits);
      bits += bitsno2[i];
    }
    *bits++ = 0; //BIT_0;
#endif

    break;
  }

  default : {
    printf("Unrecognized frame type\n");
    exit(-1);
  }

  }

  return;
}

/*----------------------------------------------------------------------------
 * int2bin convert integer to binary and write the bits bitstream array
 *----------------------------------------------------------------------------
 */
static void int2bin(
 Word16 value,             /* input : decimal value */
 Word16 no_of_bits,        /* input : number of bits to use */
 Word16 *bitstream       /* output: bitstream  */
)
{
   Word16 *pt_bitstream;
   Word16   i, bit;

   pt_bitstream = bitstream + no_of_bits;

   for (i = 0; i < no_of_bits; i++)
   {
     bit = value & (Word16)0x0001;      /* get lsb */
     if (bit == 0)
         *--pt_bitstream = 0; //BIT_0;
     else
         *--pt_bitstream = 1; //BIT_1;
     value >>= 1;
   }
}

