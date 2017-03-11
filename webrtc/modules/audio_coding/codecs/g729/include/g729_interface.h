#ifndef CODEC_G729_INTERFACE_H
#define CODEC_G729_INTERFACE_H

#ifdef __cplusplus
extern "C"
{
#endif

/*--------------------------------------------------------------------------*
 *       Codec constant parameters (coder, decoder, and postfilter)         *
 *--------------------------------------------------------------------------*/
void InitEnc729(void **pHandle, unsigned int nTimeGap);
void FreeEnc729(void **pHandle);

int Enc729(void *handle,               /* i/o   :  State structure           */
     short *speech8k,                     /* input :  speech samples (at 8 kHz) */
     unsigned char *Vout,                /* output:  output parameters         */
     short vad_enable
);


void InitDec729(void **pHandle, unsigned int nTimeGap);
void FreeDec729(void **pHandle);

int Dec729(void *handle,              /* i/o   :  State structure            */
     unsigned char *bytes,                /* input : packed parameter            */
     short *synth8k                       /* output: synthesis speech            */
);

#ifdef __cplusplus
}
#endif

#endif

