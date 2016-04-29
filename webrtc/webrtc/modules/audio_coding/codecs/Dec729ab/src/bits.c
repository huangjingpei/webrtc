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
static Word16   bin2int(Word16 no_of_bits, Word16 *bitstream);

/*----------------------------------------------------------------------------
 *  bits2prm_ld8k - converts serial received bits to  encoder parameter vector
 *----------------------------------------------------------------------------
 */
void bits2prm_ld8k(
 Word16 bits[],          /* input : serial bits (80)                       */
 Word16   prm[]          /* output: decoded parameters (11 parameters)     */
)
{
  Word16 i;
  Word16 nb_bits;

  nb_bits = *bits++;        /* Number of bits in this frame       */

  if(nb_bits == RATE_8000) {
    prm[1] = 1;
    for (i = 0; i < PRM_SIZE; i++) {
      prm[i+2] = bin2int(bitsno[i], bits);
      bits  += bitsno[i];
    }
  }
  else
#ifndef OCTET_TX_MODE
    if(nb_bits == RATE_SID) {
      prm[1] = 2;
      for (i = 0; i < 4; i++) {
        prm[i+2] = bin2int(bitsno2[i], bits);
        bits += bitsno2[i];
      }
    }
#else
  /* the last bit of the SID bit stream under octet mode will be discarded */
  if(nb_bits == RATE_SID_OCTET) {
    prm[1] = 2;
    for (i = 0; i < 4; i++) {
      prm[i+2] = bin2int(bitsno2[i], bits);
      bits += bitsno2[i];
    }
  }
#endif

  else {
    prm[1] = 0;
  }
  return;

}

/*----------------------------------------------------------------------------
 * bin2int - read specified bits from bit array  and convert to integer value
 *----------------------------------------------------------------------------
 */
static Word16 bin2int(            /* output: decimal value of bit pattern */
 Word16 no_of_bits,        /* input : number of bits to read */
 Word16 *bitstream       /* input : array containing bits */
)
{
   Word16   value, i;
   Word16 bit;

   value = 0;
   for (i = 0; i < no_of_bits; i++)
   {
     value <<= 1;
     bit = *bitstream++;
     if (bit == BIT_1)  value += 1;
   }
   return(value);
}

#if 1
Word16 read_frame(FILE *f_serial, Word16 *parm)
{
  Word16  i;
  Word16  serial[SERIAL_SIZE];          /* Serial stream               */

  if(fread(serial, sizeof(short), 2, f_serial) != 2) {
    return(0);
  }


  if(fread(&serial[2], sizeof(Word16), (size_t)serial[1], f_serial)
     != (size_t)serial[1]) {
    return(0);
  }

        printf("ser[1] = %d\n", serial[1]);

  bits2prm_ld8k(&serial[1], parm);

  /* the hardware detects frame erasures by checking if all bits
     are set to zero */

  parm[0] = 0;           /* No frame erasure */
  for (i=0; i < serial[1]; i++)
    if (serial[i+2] == 0 ) parm[0] = 1;  /* frame erased     */

  if(parm[1] == 1) {
    /* check parity and put 1 in parm[5] if parity error */
    parm[5] = Check_Parity_Pitch(parm[4], parm[5]);
  }

  return(1);
}
#else
Word16 read_frame(Word16 **pBuffer, Word16 *parm)
{
  Word16  i;
//  Word16  serial[SERIAL_SIZE];          /* Serial stream               */
  Word16  *serial = (*pBuffer);

//  if(fread(serial, sizeof(short), 2, f_serial) != 2) {
//    return(0);
//  }
  (*pBuffer) = (*pBuffer) + 2;

//  if(fread(&serial[2], sizeof(Word16), (size_t)serial[1], f_serial)
//     != (size_t)serial[1]) {
//    return(0);
//  }
  (*pBuffer) = (*pBuffer) + serial[1];

  bits2prm_ld8k(&serial[1], parm);

  /* the hardware detects frame erasures by checking if all bits
     are set to zero */

  parm[0] = 0;           /* No frame erasure */
  for (i=0; i < serial[1]; i++)
    if (serial[i+2] == 0 ) parm[0] = 1;  /* frame erased     */

  if(parm[1] == 1) {
    /* check parity and put 1 in parm[5] if parity error */
    parm[5] = Check_Parity_Pitch(parm[4], parm[5]);
  }

  return(1);
}

#endif
