/*
   ITU-T G.729A Annex B     ANSI-C Source Code
   Version 1.1    Last modified: September 1996
   Copyright (c) 1996, France Telecom, Rockwell International,
                       Universite de Sherbrooke.
   All rights reserved.
*/

/* VAD constants */
extern Word16 lbf_corr[NP+1];
extern Word16 shift_fx[32];
extern Word16 factor_fx[32];

/* SID LSF quantization */
extern Word16 noise_fg[MODE][MA_NP][M];
extern Word16 noise_fg_sum[MODE][M];
extern Word16 noise_fg_sum_inv[MODE][M];
extern Word16 PtrTab_1[32];
extern Word16 PtrTab_2[2][16];
extern Word16 Mp[MODE];

/* SID gain quantization */
extern Word16 fact[NB_GAIN+1];
extern Word16 marg[NB_GAIN+1];
extern Word16 tab_Sidgain[32];



