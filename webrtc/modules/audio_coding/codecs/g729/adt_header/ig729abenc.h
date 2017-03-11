/*
//============================================================================
//
//    FILE NAME : IG729ABENC.h
//
//    ALGORITHM : G729ABENC
//
//    PURPOSE   : IG729ABENC Interface Header
//
// 	  Copyright (c) 2011 by Adaptive Digital Technologies, Inc.
//
//============================================================================
*/

#ifndef IG729ABENC_
#define IG729ABENC_

#include <ti/xdais/std.h>
#include <ti/xdais/xdas.h>
#include <ti/xdais/ialg.h>
#include <ti/xdais/dm/isphenc1.h>
#include <common/include/adt_typedef_user.h>
/*
// ===========================================================================
// IG729ABENC_Handle
//
// This handle is used to reference all G729ABENC instance objects
*/
typedef struct IG729ABENC_Obj *IG729ABENC_Handle;

/*
// ===========================================================================
// IG729ABENC_Obj
//
// This structure must be the first field of all G729ABENC instance objects
*/
typedef struct IG729ABENC_Obj {
    struct IG729ABENC_Fxns *fxns;
} IG729ABENC_Obj;

/*
// ===========================================================================
// IG729ABENC_Status
//
// Status structure defines the parameters that can be read
// during real-time operation of the alogrithm.
// All fields of isphenc1_status (except size, vadFlag, and bufInfo) 
// are not applicable for G.729 Encoder
*/
typedef struct IG729ABENC_Status {
    ISPHENC1_Status isphenc1_status;
} IG729ABENC_Status;

/*
// ===========================================================================
// IG729ABENC_Cmd
//
// The Cmd enumeration defines the control commands for the G729ABENC
// control method.
*/
//typedef enum IG729ABENC_Cmd {
//  IG729ABENC_GETSTATUS,
//  IG729ABENC_SETSTATUS
//} IG729ABENC_Cmd;

/*
// ===========================================================================
// IG729ABENC_Params
//
// This structure defines the creation parameters for all G729ABENC objects
// All fields of isphenc1_params (except size) are not applicable for G.729 Encoder
*/
typedef struct IG729ABENC_Params {
    ISPHENC1_Params  isphenc1_params;
} IG729ABENC_Params;

/*
// ===========================================================================
// IG729ABENC_PARAMS
//
// Default parameter values for G729ABENC instance objects
*/

extern ADT_API IG729ABENC_Params IG729ABENC_PARAMS;

/*
// ===========================================================================
//	IG729ABENC_DynamicParams
//
//     This structure defines the parameters that can be
//              modified after creation via control() calls.
// All fields of isphenc1_DynamicParams (except size, vadFlag) 
// are not applicable for G.729 Encoder
*/
typedef struct IG729ABENC_DynamicParams {
    ISPHENC1_DynamicParams  isphenc1_DynamicParams;
} IG729ABENC_DynamicParams;

/*
// ===========================================================================
//	IG729ABenC_InArgs
//
//     Defines the input arguments for all G729ABENC instance process function.
*/
typedef struct IG729ABENC_InArgs {
    ISPHENC1_InArgs isphenc1_inArgs;
	XDAS_Int16      VADEnable;
	XDAS_Int16      padding;
} IG729ABENC_InArgs;

/*
// ===========================================================================
//	IG729ABENC_OutArgs
//
//  Defines the run time output arguments for all G729ABENC instance objects.
*/
typedef struct IG729ABENC_OutArgs {
    ISPHENC1_OutArgs isphenc1_outArgs;
} IG729ABENC_OutArgs;

/*
// ===========================================================================
// IG729ABENC_Fxns
//
// This structure defines all of the operations on G729ABENC objects
*/
typedef struct IG729ABENC_Fxns {
    ISPHENC1_Fxns   isphenc1;    /* IG722ENC extends IAUDENC_Fxns */
} IG729ABENC_Fxns;

#endif	/* IG729ABENC_ */
