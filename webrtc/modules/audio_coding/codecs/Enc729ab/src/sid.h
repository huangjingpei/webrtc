/*
   ITU-T G.729A Annex B     ANSI-C Source Code
   Version 1.1    Last modified: September 1996
   Copyright (c) 1996, France Telecom, Rockwell International,
                       Universite de Sherbrooke.
   All rights reserved.
*/

#define         TRUE 1
#define         FALSE 0
#define         sqr(a)  ((a)*(a))
#define         R_LSFQ 10

void Init_lsfq_noise_enc(void);
void lsfq_noise(Word16 *lsp_new, Word16 *lspq,
                Word16 freq_prev[MA_NP][M], Word16 *idx);

