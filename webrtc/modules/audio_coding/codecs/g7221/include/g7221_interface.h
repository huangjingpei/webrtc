/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_AUDIO_CODING_CODECS_G7221_MAIN_INTERFACE_G7221_INTERFACE_H_
#define MODULES_AUDIO_CODING_CODECS_G7221_MAIN_INTERFACE_G7221_INTERFACE_H_

#include "typedefs.h"
typedef struct G722_1_16_encinst_t_tag   G722_1_16_encinst_t_;
typedef struct G722_1_16_decinst_t_tag   G722_1_16_decinst_t_;
typedef struct G722_1_24_encinst_t_tag   G722_1_24_encinst_t_;
typedef struct G722_1_24_decinst_t_tag   G722_1_24_decinst_t_;
typedef struct G722_1_32_encinst_t_tag   G722_1_32_encinst_t_;
typedef struct G722_1_32_decinst_t_tag   G722_1_32_decinst_t_;
typedef struct G722_1_Inst_t_tag         G722_1_Inst_t_;

/*
 * Comfort noise constants
 */

#define G7221_WEBRTC_SPEECH     1
#define G7221_WEBRTC_CNG        2

#ifdef __cplusplus
extern "C" {
#endif

int16_t WebRtcG7221_CreateEnc16(G722_1_16_encinst_t_** encInst);
int16_t WebRtcG7221_CreateEnc24(G722_1_24_encinst_t_** encInst);
int16_t WebRtcG7221_CreateEnc32(G722_1_32_encinst_t_** encInst);
int16_t WebRtcG7221_CreateDec16(G722_1_16_decinst_t_** decInst);
int16_t WebRtcG7221_CreateDec24(G722_1_24_decinst_t_** decInst);
int16_t WebRtcG7221_CreateDec32(G722_1_32_decinst_t_** decInst);

int16_t WebRtcG7221_FreeEnc16(G722_1_16_encinst_t_** encInst);
int16_t WebRtcG7221_FreeEnc24(G722_1_24_encinst_t_** encInst);
int16_t WebRtcG7221_FreeEnc32(G722_1_32_encinst_t_** encInst);
int16_t WebRtcG7221_FreeDec16(G722_1_16_decinst_t_** decInst);
int16_t WebRtcG7221_FreeDec24(G722_1_24_decinst_t_** decInst);
int16_t WebRtcG7221_FreeDec32(G722_1_32_decinst_t_** decInst);

int16_t WebRtcG7221_EncoderInit16(G722_1_16_encinst_t_* encInst);
int16_t WebRtcG7221_EncoderInit24(G722_1_24_encinst_t_* encInst);
int16_t WebRtcG7221_EncoderInit32(G722_1_32_encinst_t_* encInst);
int16_t WebRtcG7221_DecoderInit16(G722_1_16_decinst_t_* decInst);
int16_t WebRtcG7221_DecoderInit24(G722_1_24_decinst_t_* decInst);
int16_t WebRtcG7221_DecoderInit32(G722_1_32_decinst_t_* decInst);

int16_t WebRtcG7221_Encode16(G722_1_16_encinst_t_* encInst,
                          int16_t* input,
                          int16_t len,
                          int16_t* output);
int16_t WebRtcG7221_Encode24(G722_1_24_encinst_t_* encInst,
                          int16_t* input,
                          int16_t len,
                          int16_t* output);
int16_t WebRtcG7221_Encode32(G722_1_32_encinst_t_* encInst,
                          int16_t* input,
                          int16_t len,
                          int16_t* output);

int16_t WebRtcG7221_Decode16(G722_1_16_decinst_t_* decInst,
                          int16_t* bitstream,
                          int16_t len,
                          int16_t* output);
int16_t WebRtcG7221_Decode24(G722_1_24_decinst_t_* decInst,
                          int16_t* bitstream,
                          int16_t len,
                          int16_t* output);
int16_t WebRtcG7221_Decode32(G722_1_32_decinst_t_* decInst,
                          int16_t* bitstream,
                          int16_t len,
                          int16_t* output);

int16_t WebRtcG7221_DecodePlc16(G722_1_16_decinst_t_* decInst,
                             int16_t* output,
                             int16_t nrLostFrames);
int16_t WebRtcG7221_DecodePlc24(G722_1_24_decinst_t_* decInst,
                             int16_t* output,
                             int16_t nrLostFrames);
int16_t WebRtcG7221_DecodePlc32(G722_1_32_decinst_t_* decInst,
                             int16_t* output,
                             int16_t nrLostFrames);


/****************************************************************************
 * WebRtcG7221_Version(...)
 *
 * Get a string with the current version of the codec
 */

int16_t WebRtcG7221_Version(char *versionStr, short len);


#ifdef __cplusplus
}
#endif


#endif /* MODULES_AUDIO_CODING_CODECS_G722_MAIN_INTERFACE_G722_INTERFACE_H_ */
