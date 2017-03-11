/*******************************************************************
*
* 	File: 	g729ab_user.h
*
* 	Description: 	public header file for G.729AB Coder.
*
*	Copyright(c) 2003 - 2011 by Adaptive Digital Technologies, Inc.
*	All Rights Reserved
********************************************************************/

#ifndef _G729AB_USER_H
#define _G729AB_USER_H

#include <common/include/adt_typedef_user.h>

#define ENCODE_CHAN_SIZE    537
#define ENCODE_SCRATCH_SIZE 664

#define DECODE_CHAN_SIZE    550
#define DECODE_SCRATCH_SIZE 200


typedef struct
{
  	ADT_Int32 encodeData[ENCODE_CHAN_SIZE];
} G729ENC_ADT_Instance_t;

typedef struct 
{
    ADT_Int32 encodeScratch[ENCODE_SCRATCH_SIZE];
} G729ENC_ADT_Scratch_t;

typedef struct 
{
    ADT_Int32 decodeData[DECODE_CHAN_SIZE];
} G729DEC_ADT_Instance_t;

typedef struct 
{
    ADT_Int32 decodeScratch[DECODE_SCRATCH_SIZE];
} G729DEC_ADT_Scratch_t;


// ================ Encoder Interface ==========================

ADT_API void G729AB_ADT_Encode(G729ENC_ADT_Instance_t *Channel, short int *Speech,char *Compressed,short int VADEnable,short int *VADFlag);
ADT_API void G729AB_ADT_EncodeInit(G729ENC_ADT_Instance_t *Channel, G729ENC_ADT_Scratch_t *scratch);
ADT_API void G729AB_ADT_getEncSizes(ADT_MemRec_t *pInstanceMemRec, ADT_MemRec_t *pScratchMemRec);

ADT_API void G729AB_ADT_SetEncPtrs(G729ENC_ADT_Instance_t *pInstance);


// ================ Decoder Interface ==========================

ADT_API void G729AB_ADT_Decode(
	G729DEC_ADT_Instance_t *Channel,
	char *Compressed,
	short int *Speech,
	short int FrameErase,
	short int VADFlag
);

ADT_API void G729AB_ADT_DecodeInit(G729DEC_ADT_Instance_t *Channel, G729DEC_ADT_Scratch_t *scratch);
ADT_API void G729AB_ADT_getDecSizes(ADT_MemRec_t *pInstanceMemRec, ADT_MemRec_t *pScratchMemRec);

ADT_API void G729AB_ADT_SetDecPtrs(G729DEC_ADT_Instance_t *pInstance);


ADT_API void G729AB_ADT_bytesToParam(char *Bytes, ADT_Int16 VADFlag, ADT_Int16 *Param);

ADT_API void G729AB_ADT_paramToBytes(ADT_Int16 *Param, ADT_Int16 VADFlag, char *Bytes);

ADT_API void G729AB_ADT_Encode_To_Params(G729ENC_ADT_Instance_t *Channel,ADT_Int16 *Speech,ADT_Int16 *Params,ADT_Int16 VADEnable,ADT_Int16 *VADFlag);
ADT_API void G729AB_ADT_Decode_From_Param(G729DEC_ADT_Instance_t *Channel,short int *Param,short int *Speech,short int FrameErase,short int VADFlag);

#endif //_G729AB_USER_H
