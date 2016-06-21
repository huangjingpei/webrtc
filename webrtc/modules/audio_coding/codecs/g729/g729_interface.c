#include <stdint.h>


#include "ld8a.h"
#include "g729ab_user.h" // adt

#include "include/g729_interface.h"
#include <string.h>

//#define TESTVECTOR_MODE

typedef struct _adt_g729enc
{
    G729ENC_ADT_Scratch_t  EncScratch;
    G729ENC_ADT_Instance_t EncState;

    //uint32_t frame_num;
}adt_enc_t;

typedef struct _adt_g729dec
{
    G729DEC_ADT_Scratch_t  DecScratch;
    G729DEC_ADT_Instance_t DecState;

    //uint32_t frame_num;
}adt_dec_t;

static int16_t lsp_old_init[M]={
              30000, 26000, 21000, 15000, 8000, 0, -8000,-15000,-21000,-26000};
static int16_t old_A_init[M+1]={4096,0,0,0,0,0,0,0,0,0,0};
static int16_t past_qua_en_init[4] = { -14336, -14336, -14336, -14336 };


void InitEnc729(void **pHandle, unsigned int nTimeGap)
{
    *pHandle = NULL;

    if (nTimeGap % 10) { // must be dividable by 10(ms)
        return;
    }

#if defined(G729AB)
    adt_enc_t *handle = NULL;
    handle = malloc(sizeof(adt_enc_t));

    if (handle == NULL) {
        return;
    }

    //handle->frame_num  = nTimeGap / 10;
    G729AB_ADT_EncodeInit(&handle->EncState, &handle->EncScratch);

#else
    EncStatus *handle = NULL;
    handle = malloc(sizeof(EncStatus));

    if (handle == NULL) {
        return;
    }

    memset(handle, 0, sizeof(EncStatus));

    handle->frame_num  = nTimeGap / 10;
    handle->vad_enable = 0; // 1; 暂时不支持 VAD-DTX

    memcpy(handle->lsp_old, lsp_old_init, M*sizeof(int16_t));
    memcpy(handle->past_qua_en, past_qua_en_init, 4*sizeof(int16_t));
    memcpy(handle->dtx.old_A, old_A_init, (M+1)*sizeof(int16_t));

    Init_Pre_Process(&(handle->prp));
    Init_Coder_ld8a(handle);

    /* for G.729B */
    Init_Cod_cng(&(handle->dtx));
#endif

    *pHandle = (void *)handle;
}

void FreeEnc729(void **pHandle)
{
    free((*pHandle));

    *pHandle = NULL;
}

int Enc729(void *handle,              /* i/o   :  State structure           */
     short *speech8k,                     /* input :  speech samples (at 8 kHz) */
     unsigned char *bytes,                /* output:  output parameters         */
     short vad_enable
)
{
#if defined(G729AB) 
    ADT_Int16 vad_flag = 0;
    adt_enc_t *pEnc = (adt_enc_t *)handle;

    vad_enable = 0;
    //for (i=0, bitnum=0; i<pEnc->frame_num; i++) TODO
    G729AB_ADT_Encode(&pEnc->EncState, speech8k, bytes, vad_enable, &vad_flag);
    return 10;
    /*if (vad_flag == 1) {
        return 10;
    } else if (vad_flag == 0) {
        return 0;
    } else {
        return 2; // we could not get silcence frame from the encoder, 
                  // so use external DTX and comment these codes
    } */

#else
    int16_t prm[PRM_SIZE+1];        /* Analysis parameters + frame type      */
    int16_t serial[SERIAL_SIZE];    /* Output bitstream buffer               */
    int i, k, bitnum;
    EncStatus *pEnc = (EncStatus *)handle;
    unsigned char *Vout = bytes;
    unsigned char sprm[10];

    for (i=0, bitnum=0; i<pEnc->frame_num; i++)
    {
        if (pEnc->frame == 32767) pEnc->frame = 256;
        else pEnc->frame++;

        memcpy(pEnc->new_speech, speech8k + i*L_FRAME, L_FRAME*sizeof(short));

        pEnc->vad_enable = vad_enable;
        Pre_Process(&(pEnc->prp), pEnc->new_speech, L_FRAME);
        Coder_ld8a(pEnc, prm, pEnc->frame, vad_enable);

#ifndef TESTVECTOR_MODE
        if (prm2bits_ld8k(prm, serial) == -1){
            break;
        }

        memset(sprm, 0, 10);
        for (k = 0 ; k < serial[1]; k++ )
            sprm[k>>3] ^= (serial[k+2] == BIT_1)?(0x80 >> (k & 0x0007)): (0x00) ;

        memcpy(Vout, sprm, (serial[1] >> 3));
        Vout += (serial[1] >> 3); // OCTET TX MODE
        bitnum += serial[1] >>3;
#else
        if (prm2bits_ld8k(prm, serial) == -1){
            break;
        }

        memcpy(Vout, serial, sizeof(int16_t)*(serial[1]+2));
        Vout += (serial[1]+2)*sizeof(int16_t);
        bitnum += serial[1];
#endif
    }

    return bitnum;
#endif
}

////////////////////////////////////////

void InitDec729(void **pHandle, unsigned int nTimeGap)
{
    *pHandle = NULL;

    if (nTimeGap % 10)  // must be dividable by 10(ms)
    {
        return;
    }
#if defined(G729AB)
    adt_dec_t *handle = NULL;
    handle = malloc(sizeof(adt_dec_t));

    if (handle == NULL) {
        return;
    }

    //handle->frame_num  = nTimeGap / 10;
    G729AB_ADT_DecodeInit(&handle->DecState, &handle->DecScratch);
#else
    DecStatus *handle = NULL;
    handle = (DecStatus *)malloc(sizeof(DecStatus));
    if (handle == NULL) {
        return;
    }

    memset(handle, 0, sizeof(DecStatus));

    handle->bad_lsf = 0;          /* Initialize bad LSF indicator */
    handle->frame_num  = nTimeGap / 10;

    memcpy(handle->lsp_old, lsp_old_init, sizeof(int16_t)*M);
    memcpy(handle->past_qua_en, past_qua_en_init, sizeof(int16_t)*4);

    Init_Decod_ld8a(handle);
    Init_Post_Filter(&(handle->pof));
    Init_Post_Process(&(handle->pop));

    /* for G.729b */
    Init_Dec_cng(&(handle->sid));
#endif

    *pHandle = (void *)handle;
}

void FreeDec729(void **pHandle)
{
    free((*pHandle));

    *pHandle = NULL;
}

int Dec729(void *handle,              /* i/o   :  State structure            */
     unsigned char *bytes,               /* input : packed parameter            */
     short *synth8k                      /* output: synthesis speech              */
)
{
#if defined(G729AB)
    ADT_Int16 frameErase = 0;
    ADT_Int16 vad_flag = 1; // ought set by RTP layer
    adt_dec_t *pDec = (adt_dec_t *)handle;

    //for (i=0, bitnum=0; i<pEnc->frame_num; i++) TODO
    G729AB_ADT_Decode(&pDec->DecState, bytes, synth8k, frameErase, vad_flag);

    return 80;
    /*if (vad_flag == 1) { // normal
        return 80;
    } else if (vad_flag == 0) {
        return 0;
    } else {
        return 15; // ?
    } */
#else
    int16_t i, Vad;
    int k, offset;
    int16_t serial[SERIAL_SIZE];
    int16_t *synth;
    unsigned char *pcode = bytes;
    DecStatus *pDec = (DecStatus *)handle;

    synth = pDec->synth_buf + M;

    for (k=0, offset=0; k<pDec->frame_num; k++)
    {
        /* do 'read_frame' job */
#ifdef TESTVECTOR_MODE
        memcpy(serial, bytes+offset, 2*sizeof(short));
        offset += (2*sizeof(short));

        memcpy(serial+2, bytes+offset, serial[1]*sizeof(short));
        offset += (serial[1]*sizeof(short));

        bits2prm_ld8k(&serial[1], pDec->parm);
#else
        serial[0] = SYNC_WORD;
        serial[1] = 80;     // do not support VAD/CNG/DTX now
        for (i = 0 ; i <80; i ++ )
            serial[i+2] = (( pcode[i>>3] << (i & (int16_t)0x0007) ) & (int16_t)0x0080)? BIT_1 : BIT_0;

        offset += 80;
        bits2prm_ld8k(&serial[1], pDec->parm);
#endif
        // could be in decoder itself
        pDec->parm[0] = 0;           /* No frame erasure */
        for (i=0; i < serial[1]; i++)
        if (serial[i+2] == 0 ) pDec->parm[0] = 1;  /* frame erased     */

        if(pDec->parm[1] == 1) {
        /* check parity and put 1 in parm[5] if parity error */
        pDec->parm[5] = Check_Parity_Pitch(pDec->parm[4], pDec->parm[5]);
        }
        /* 'read_frame' job end */

        Decod_ld8a(pDec, pDec->parm, synth, pDec->Az_dec, pDec->T2, &Vad);
        Post_Filter(&(pDec->pof), synth, pDec->Az_dec, pDec->T2, Vad);        /* Post-filter */
        Post_Process(&(pDec->pop), synth, L_FRAME);

        memcpy(synth8k+k*L_FRAME, synth, sizeof(short)*L_FRAME);

        pcode += 10;
    }

    return (offset);
#endif
}
