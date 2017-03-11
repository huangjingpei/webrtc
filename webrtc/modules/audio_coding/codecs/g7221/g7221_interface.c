#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include "g7221_interface.h"
#include "g7221c_interface.h"
#include "g722_1/g722_1.h"
#include "typedefs.h"

int16_t WebRtcG7221_CreateEnc16(G722_1_16_encinst_t_** encInst)
{
    *encInst=(G722_1_16_encinst_t_*)malloc(sizeof(g722_1_encode_state_t));
    if (*encInst!=NULL) {
      return(0);
    } else {
      return(-1);
    }
}

int16_t WebRtcG7221_CreateEnc24(G722_1_24_encinst_t_** encInst)
{
    *encInst=(G722_1_24_encinst_t_*)malloc(sizeof(g722_1_encode_state_t));
    if (*encInst!=NULL) {
      return(0);
    } else {
      return(-1);
    }
}

int16_t WebRtcG7221_CreateEnc32(G722_1_32_encinst_t_** encInst)
{
    *encInst=(G722_1_32_encinst_t_*)malloc(sizeof(g722_1_encode_state_t));
    if (*encInst!=NULL) {
      return(0);
    } else {
      return(-1);
    }
}

int16_t WebRtcG7221_CreateDec16(G722_1_16_decinst_t_** decInst)
{
    *decInst=(G722_1_16_decinst_t_*)malloc(sizeof(g722_1_decode_state_t));
    if (*decInst!=NULL) {
      return(0);
    } else {
      return(-1);
    }
}

int16_t WebRtcG7221_CreateDec24(G722_1_24_decinst_t_** decInst)
{
    *decInst=(G722_1_24_decinst_t_*)malloc(sizeof(g722_1_decode_state_t));
    if (*decInst!=NULL) {
      return(0);
    } else {
      return(-1);
    }
}

int16_t WebRtcG7221_CreateDec32(G722_1_32_decinst_t_** decInst)
{
    *decInst=(G722_1_32_decinst_t_*)malloc(sizeof(g722_1_decode_state_t));
    if (*decInst!=NULL) {
      return(0);
    } else {
      return(-1);
    }
}


int16_t WebRtcG7221_FreeEnc16(G722_1_16_encinst_t_** encInst)
{
    return g722_1_encode_release((g722_1_encode_state_t*) *encInst);
}

int16_t WebRtcG7221_FreeEnc24(G722_1_24_encinst_t_** encInst)
{
    return g722_1_encode_release((g722_1_encode_state_t*) *encInst);
}

int16_t WebRtcG7221_FreeEnc32(G722_1_32_encinst_t_** encInst)
{
    return g722_1_encode_release((g722_1_encode_state_t*) *encInst);
}

int16_t WebRtcG7221_FreeDec16(G722_1_16_decinst_t_** decInst)
{
    return g722_1_decode_release((g722_1_decode_state_t*) *decInst);
}

int16_t WebRtcG7221_FreeDec24(G722_1_24_decinst_t_** decInst)
{
    return g722_1_decode_release((g722_1_decode_state_t*) *decInst);
}

int16_t WebRtcG7221_FreeDec32(G722_1_32_decinst_t_** decInst)
{
    return g722_1_decode_release((g722_1_decode_state_t*) *decInst);
}


int16_t WebRtcG7221_EncoderInit16(G722_1_16_encinst_t_* encInst)
{

    encInst = (G722_1_16_encinst_t_ *) g722_1_encode_init((g722_1_encode_state_t*) encInst, 16000, 16000);
    if (encInst == NULL) {
        //printf("%s %s %d\n",__FILE__, __FUNCTION__, __LINE__);
        return -1;
    } else {
        //printf("%s %s %d\n",__FILE__, __FUNCTION__, __LINE__);
        return 0;
    }
}

int16_t WebRtcG7221_EncoderInit24(G722_1_24_encinst_t_* encInst)
{
    encInst = (G722_1_16_encinst_t_ *) g722_1_encode_init((g722_1_encode_state_t*) encInst, 24000, 16000);
    if (encInst == NULL) {
        //printf("%s %s %d\n",__FILE__, __FUNCTION__, __LINE__);
        return -1;
    } else {
        //printf("%s %s %d\n",__FILE__, __FUNCTION__, __LINE__);
        return 0;
    }
}

int16_t WebRtcG7221_EncoderInit32(G722_1_32_encinst_t_* encInst)
{
    encInst = (G722_1_16_encinst_t_ *) g722_1_encode_init((g722_1_encode_state_t*) encInst, 32000, 16000);
    if (encInst == NULL) {
        //printf("%s %s %d\n",__FILE__, __FUNCTION__, __LINE__);
        return -1;
    } else {
        //printf("%s %s %d\n",__FILE__, __FUNCTION__, __LINE__);
        return 0;
    }
}

int16_t WebRtcG7221_DecoderInit16(G722_1_16_decinst_t_* decInst)
{
    decInst = (G722_1_16_decinst_t_ *) g722_1_decode_init((g722_1_decode_state_t*) decInst, 16000, 16000);
    if (decInst == NULL) {
        //printf("%s %s %d\n",__FILE__, __FUNCTION__, __LINE__);
        return -1;
    } else {
        //printf("%s %s %d\n",__FILE__, __FUNCTION__, __LINE__);
        return 0;
    }
}

int16_t WebRtcG7221_DecoderInit24(G722_1_24_decinst_t_* decInst)
{
    decInst = (G722_1_16_decinst_t_ *) g722_1_decode_init((g722_1_decode_state_t*) decInst, 24000, 16000);
    if (decInst == NULL) {
        //printf("%s %s %d\n",__FILE__, __FUNCTION__, __LINE__);
        return -1;
    } else {
        //printf("%s %s %d\n",__FILE__, __FUNCTION__, __LINE__);
        return 0;
    }
}

int16_t WebRtcG7221_DecoderInit32(G722_1_32_decinst_t_* decInst)
{
    decInst = (G722_1_16_decinst_t_ *) g722_1_decode_init((g722_1_decode_state_t*) decInst, 32000, 16000);
    if (decInst == NULL) {
        //printf("%s %s %d\n",__FILE__, __FUNCTION__, __LINE__);
        return -1;
    } else {
        //printf("%s %s %d\n",__FILE__, __FUNCTION__, __LINE__);
        return 0;
    }
}


int16_t WebRtcG7221_Encode16(G722_1_16_encinst_t_* encInst,
                          int16_t* input,
                          int16_t len,
                          int16_t* output)
{
    int byte = g722_1_encode((g722_1_encode_state_t *)encInst, output, input, len);


    //printf("%s %s %d    %d\n",__FILE__, __FUNCTION__, __LINE__,byte);
    return byte;
}
#if 0
static FILE *_fp1 = NULL;
static FILE *_fp2 = NULL;
#endif
int16_t WebRtcG7221_Encode24(G722_1_24_encinst_t_* encInst,
                          int16_t* input,
                          int16_t len,
                          int16_t* output)
{
#if 0
    if (_fp1 == NULL) {
        _fp1 = fopen("/mnt/pcm.pcm", "wb");
    }
    if (_fp2 == NULL) {
        _fp2 = fopen("/mnt/g7221.g7221", "wb");
    }
#endif

    int byte =  g722_1_encode((g722_1_encode_state_t *)encInst, output, input, len);

#if 0
    if (_fp1 != NULL) {
        fwrite(input, 2, len, _fp1);
        fflush(_fp1);
    }
    if (_fp2 != NULL) {
        fwrite(output, 1, byte, _fp2);
        fflush(_fp2);
    }
#endif
    return byte;
}

int16_t WebRtcG7221_Encode32(G722_1_32_encinst_t_* encInst,
                          int16_t* input,
                          int16_t len,
                          int16_t* output)
{
    int byte =  g722_1_encode((g722_1_encode_state_t *)encInst, output, input, len);
    //printf("%s %s %d    %d\n",__FILE__, __FUNCTION__, __LINE__,byte);
    return byte;
}


int16_t WebRtcG7221_Decode16(G722_1_16_decinst_t_* decInst,
                          int16_t* bitstream,
                          int16_t len,
                          int16_t* output)
{
    //printf("%s %s %d\n",__FILE__, __FUNCTION__, __LINE__);
    return g722_1_decode((g722_1_decode_state_t *)decInst, output, bitstream, len);
}

int16_t WebRtcG7221_Decode24(G722_1_24_decinst_t_* decInst,
                          int16_t* bitstream,
                          int16_t len,
                          int16_t* output)
{
    //printf("%s %s %d\n",__FILE__, __FUNCTION__, __LINE__);
    return g722_1_decode((g722_1_decode_state_t *)decInst, output, bitstream, len);
}

int16_t WebRtcG7221_Decode32(G722_1_32_decinst_t_* decInst,
                          int16_t* bitstream,
                          int16_t len,
                          int16_t* output)
{
    //printf("%s %s %d\n",__FILE__, __FUNCTION__, __LINE__);
    return g722_1_decode((g722_1_decode_state_t *)decInst, output, bitstream, len);
}


int16_t WebRtcG7221_DecodePlc16(G722_1_16_decinst_t_* decInst,
                             int16_t* output,
                             int16_t nrLostFrames)
{
    return 0;
}

int16_t WebRtcG7221_DecodePlc24(G722_1_24_decinst_t_* decInst,
                             int16_t* output,
                             int16_t nrLostFrames)
{
    return 0;//g722_1_fillin((g722_1_decode_state_t *)decInst, int16_t amp[], const uint8_t g722_1_data[], int len);;
}

int16_t WebRtcG7221_DecodePlc32(G722_1_32_decinst_t_* decInst,
                             int16_t* output,
                             int16_t nrLostFrames)
{
    return 0;
}


int16_t WebRtcG7221_Version(char *versionStr, short len)
{
    // Get version string
    char version[30] = "2.0.0\n";
    if (strlen(version) < (unsigned int)len)
    {
        strcpy(versionStr, version);
        return 0;
    }
    else
    {
        return -1;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///

int16_t WebRtcG7221C_CreateEnc24(G722_1C_24_encinst_t_** encInst)
{
    *encInst=(G722_1C_24_encinst_t_*)malloc(sizeof(g722_1_encode_state_t));
    if (*encInst!=NULL) {
      return(0);
    } else {
      return(-1);
    }
}
int16_t WebRtcG7221C_CreateEnc32(G722_1C_32_encinst_t_** encInst)
{
    *encInst=(G722_1C_32_encinst_t_*)malloc(sizeof(g722_1_encode_state_t));
    if (*encInst!=NULL) {
      return(0);
    } else {
      return(-1);
    }
}
int16_t WebRtcG7221C_CreateEnc48(G722_1C_48_encinst_t_** encInst)
{
    *encInst=(G722_1C_48_encinst_t_*)malloc(sizeof(g722_1_encode_state_t));
    if (*encInst!=NULL) {
      return(0);
    } else {
      return(-1);
    }
}
int16_t WebRtcG7221C_CreateDec24(G722_1C_24_decinst_t_** decInst)
{
    *decInst=(G722_1C_24_decinst_t_*)malloc(sizeof(g722_1_decode_state_t));
    if (*decInst!=NULL) {
      return(0);
    } else {
      return(-1);
    }
}
int16_t WebRtcG7221C_CreateDec32(G722_1C_32_decinst_t_** decInst)
{
    *decInst=(G722_1C_24_decinst_t_*)malloc(sizeof(g722_1_decode_state_t));
    if (*decInst!=NULL) {
      return(0);
    } else {
      return(-1);
    }
}
int16_t WebRtcG7221C_CreateDec48(G722_1C_48_decinst_t_** decInst)
{
    *decInst=(G722_1C_24_decinst_t_*)malloc(sizeof(g722_1_decode_state_t));
    if (*decInst!=NULL) {
      return(0);
    } else {
      return(-1);
    }
}

int16_t WebRtcG7221C_FreeEnc24(G722_1C_24_encinst_t_** encInst)
{
    return g722_1_encode_release((g722_1_encode_state_t*) *encInst);
}
int16_t WebRtcG7221C_FreeEnc32(G722_1C_32_encinst_t_** encInst)
{
    return g722_1_encode_release((g722_1_encode_state_t*) *encInst);
}
int16_t WebRtcG7221C_FreeEnc48(G722_1C_48_encinst_t_** encInst)
{
    return g722_1_encode_release((g722_1_encode_state_t*) *encInst);
}
int16_t WebRtcG7221C_FreeDec24(G722_1C_24_decinst_t_** decInst)
{
    return g722_1_decode_release((g722_1_decode_state_t*) *decInst);
}
int16_t WebRtcG7221C_FreeDec32(G722_1C_32_decinst_t_** decInst)
{
    return g722_1_decode_release((g722_1_decode_state_t*) *decInst);
}
int16_t WebRtcG7221C_FreeDec48(G722_1C_48_decinst_t_** decInst)
{
    return g722_1_decode_release((g722_1_decode_state_t*) *decInst);
}

int16_t WebRtcG7221C_EncoderInit24(G722_1C_24_encinst_t_* encInst)
{
    encInst = (G722_1C_24_encinst_t_ *) g722_1_encode_init((g722_1_encode_state_t*) encInst, 24000, 32000);
    if (encInst == NULL) {
        //printf("%s %s %d\n",__FILE__, __FUNCTION__, __LINE__);
        return -1;
    } else {
        //printf("%s %s %d\n",__FILE__, __FUNCTION__, __LINE__);
        return 0;
    }
}
int16_t WebRtcG7221C_EncoderInit32(G722_1C_32_encinst_t_* encInst)
{
    encInst = (G722_1C_32_encinst_t_ *) g722_1_encode_init((g722_1_encode_state_t*) encInst, 32000, 32000);
    if (encInst == NULL) {
        //printf("%s %s %d\n",__FILE__, __FUNCTION__, __LINE__);
        return -1;
    } else {
        //printf("%s %s %d\n",__FILE__, __FUNCTION__, __LINE__);
        return 0;
    }
}
int16_t WebRtcG7221C_EncoderInit48(G722_1C_48_encinst_t_* encInst)
{
    encInst = (G722_1C_48_encinst_t_ *) g722_1_encode_init((g722_1_encode_state_t*) encInst, 48000, 32000);
    if (encInst == NULL) {
        //printf("%s %s %d\n",__FILE__, __FUNCTION__, __LINE__);
        return -1;
    } else {
        //printf("%s %s %d\n",__FILE__, __FUNCTION__, __LINE__);
        return 0;
    }
}
int16_t WebRtcG7221C_DecoderInit24(G722_1C_24_decinst_t_* decInst)
{
    decInst = (G722_1C_24_decinst_t_ *) g722_1_decode_init((g722_1_decode_state_t*) decInst, 24000, 32000);
    if (decInst == NULL) {
        //printf("%s %s %d\n",__FILE__, __FUNCTION__, __LINE__);
        return -1;
    } else {
        //printf("%s %s %d\n",__FILE__, __FUNCTION__, __LINE__);
        return 0;
    }
}
int16_t WebRtcG7221C_DecoderInit32(G722_1C_32_decinst_t_* decInst)
{
    decInst = (G722_1C_32_decinst_t_ *) g722_1_decode_init((g722_1_decode_state_t*) decInst, 32000, 32000);
    if (decInst == NULL) {
        //printf("%s %s %d\n",__FILE__, __FUNCTION__, __LINE__);
        return -1;
    } else {
        //printf("%s %s %d\n",__FILE__, __FUNCTION__, __LINE__);
        return 0;
    }
}
int16_t WebRtcG7221C_DecoderInit48(G722_1C_48_decinst_t_* decInst)
{
    decInst = (G722_1C_48_decinst_t_ *) g722_1_decode_init((g722_1_decode_state_t*) decInst, 48000, 32000);
    if (decInst == NULL) {
        //printf("%s %s %d\n",__FILE__, __FUNCTION__, __LINE__);
        return -1;
    } else {
        //printf("%s %s %d\n",__FILE__, __FUNCTION__, __LINE__);
        return 0;
    }
}

int16_t WebRtcG7221C_Encode24(G722_1C_24_encinst_t_* encInst,
                           int16_t* input,
                           int16_t len,
                           int16_t* output)
{
    //printf("%s %s %d\n",__FILE__, __FUNCTION__, __LINE__);
    return g722_1_encode((g722_1_encode_state_t *)encInst, output, input, len);
}
int16_t WebRtcG7221C_Encode32(G722_1C_32_encinst_t_* encInst,
                           int16_t* input,
                           int16_t len,
                           int16_t* output)
{
    //printf("%s %s %d\n",__FILE__, __FUNCTION__, __LINE__);
    return g722_1_encode((g722_1_encode_state_t *)encInst, output, input, len);
}
int16_t WebRtcG7221C_Encode48(G722_1C_48_encinst_t_* encInst,
                           int16_t* input,
                           int16_t len,
                           int16_t* output)

{
    //printf("%s %s %d\n",__FILE__, __FUNCTION__, __LINE__);
    return g722_1_encode((g722_1_encode_state_t *)encInst, output, input, len);
}

int16_t WebRtcG7221C_Decode24(G722_1C_24_decinst_t_* decInst,
                           int16_t* bitstream,
                           int16_t len,
                           int16_t* output)

{
    //printf("%s %s %d\n",__FILE__, __FUNCTION__, __LINE__);
    return g722_1_decode((g722_1_decode_state_t *)decInst, output, bitstream, len);
}
int16_t WebRtcG7221C_Decode32(G722_1C_32_decinst_t_* decInst,
                           int16_t* bitstream,
                           int16_t len,
                           int16_t* output)
{
    //printf("%s %s %d\n",__FILE__, __FUNCTION__, __LINE__);
    return g722_1_decode((g722_1_decode_state_t *)decInst, output, bitstream, len);
}
int16_t WebRtcG7221C_Decode48(G722_1C_48_decinst_t_* decInst,
                           int16_t* bitstream,
                          int16_t len,
                           int16_t* output)

{
    //printf("%s %s %d\n",__FILE__, __FUNCTION__, __LINE__);
    return g722_1_decode((g722_1_decode_state_t *)decInst, output, bitstream, len);
}

int16_t WebRtcG7221C_DecodePlc24(G722_1C_24_decinst_t_* decInst,
                              int16_t* output,
                              int16_t nrLostFrames)
{
    return 0;
}
int16_t WebRtcG7221C_DecodePlc32(G722_1C_32_decinst_t_* decInst,
                              int16_t* output,
                              int16_t nrLostFrames)
{
    return 0;
}
int16_t WebRtcG7221C_DecodePlc48(G722_1C_48_decinst_t_* decInst,
                              int16_t* output,
                              int16_t nrLostFrames)
{
    return 0;
}


/****************************************************************************
 * WebRtcG7221_Version(...)
 *
 * Get a string with the current version of the codec
 */

int16_t WebRtcG7221C_Version(char *versionStr, short len)
{
    // Get version string
    char version[30] = "1.0.0\n";
    if (strlen(version) < (unsigned int)len)
    {
        strcpy(versionStr, version);
        return 0;
    }
    else
    {
        return -1;
    }
}


