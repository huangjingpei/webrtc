/*
   ITU-T G.729A Speech Coder    ANSI-C Source Code
   Version 1.1    Last modified: September 1996

   Copyright (c) 1996,
   AT&T, France Telecom, NTT, Universite de Sherbrooke, Lucent Technologies
   All rights reserved.
*/

/**************************************************************************
 * Taming functions.                                                      *
 **************************************************************************/

#include "typedef.h"
#include "basic_op.h"
#include "oper_32b.h"
#include "ld8a.h"
#include "tab_ld8a.h"


/**************************************************************************
 *routine update_exc_err - maintains the memory used to compute the error *
 * function due to an adaptive codebook mismatch between encoder and      *
 * decoder                                                                *
 **************************************************************************/

void update_exc_err(
 Word32 *L_exc_err,
 Word16 gain_pit,      /* (i) pitch gain */
 Word16 T0             /* (i) integer part of pitch delay */
)
 {

    Word16 i, zone1, zone2, n;
    Word32 L_worst, L_temp, L_acc;
    Word16 hi, lo;

    L_worst = -1L;
    n = sub(T0, L_SUBFR);

    if(n < 0) {
        L_Extract(L_exc_err[0], &hi, &lo);
        L_temp = Mpy_32_16(hi, lo, gain_pit);
        L_temp = L_shl(L_temp, 1);
        L_temp = L_add(0x00004000L, L_temp);
        L_acc = L_sub(L_temp, L_worst);
        if(L_acc > 0L) {
                L_worst = L_temp;
        }
        L_Extract(L_temp, &hi, &lo);
        L_temp = Mpy_32_16(hi, lo, gain_pit);
        L_temp = L_shl(L_temp, 1);
        L_temp = L_add(0x00004000L, L_temp);
        L_acc = L_sub(L_temp, L_worst);
        if(L_acc > 0L) {
                L_worst = L_temp;
        }
    }

    else {

        zone1 = tab_zone[n];

        i = sub(T0, 1);
        zone2 = tab_zone[i];

        for(i = zone1; i <= zone2; i++) {
                L_Extract(L_exc_err[i], &hi, &lo);
                L_temp = Mpy_32_16(hi, lo, gain_pit);
                L_temp = L_shl(L_temp, 1);
                L_temp = L_add(0x00004000L, L_temp);
                L_acc = L_sub(L_temp, L_worst);
                if(L_acc > 0L) L_worst = L_temp;
        }
    }

    for(i=3; i>=1; i--) {
        L_exc_err[i] = L_exc_err[i-1];
    }
    L_exc_err[0] = L_worst;

    return;
}

