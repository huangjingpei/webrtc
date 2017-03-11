/*
//============================================================================
//
//    FILE NAME : IG729ABDEC.h
//
//    ALGORITHM : G729ABDEC
//
//    PURPOSE   : IG729ABDEC Interface Header
//
// 	  Copyright (c) 2011 by Adaptive Digital Technologies, Inc.
//
//============================================================================
*/

#ifndef IG729ABDEC_
#define IG729ABDEC_

#include <ti/xdais/std.h>
#include <ti/xdais/xdas.h>
#include <ti/xdais/ialg.h>
#include <ti/xdais/dm/isphdec1.h>
#include <common/include/adt_typedef_user.h>
/*
// ===========================================================================
// IG729ABDEC_Handle
//
// This handle is used to reference all G729ABDEC instance objects
*/
typedef struct IG729ABDEC_Obj *IG729ABDEC_Handle;

/*
// ===========================================================================
// IG729ABDEC_Obj
//
// This structure must be the first field of all G729ABDEC instance objects
*/
typedef struct IG729ABDEC_Obj {
    struct IG729ABDEC_Fxns *fxns;
} IG729ABDEC_Obj;

/*
// ===========================================================================
// IG729ABDEC_Status
//
// Status structure defines the parameters that can be changed or read
// during real-time operation of the alogrithm.
// All fields of isphdec1_status (except size and bufInfo) 
// are not applicable for G.729 Decoder
*/
typedef struct IG729ABDEC_Status {
    ISPHDEC1_Status isphdec1_status;
} IG729ABDEC_Status;

/*
// ===========================================================================
// IG729ABDEC_Cmd
//
// The Cmd enumeration defines the control commands for the G729ABDEC
// control method.
*/
///typedef enum IG729ABDEC_Cmd {
//  IG729ABDEC_GETSTATUS,
//  IG729ABDEC_SETSTATUS
//} IG729ABDEC_Cmd;

/*
// ===========================================================================
// IG729ABDEC_Params
//
// This structure defines the creation parameters for all G729ABDEC objects
// All fields of isphdec1_params (except size) are not applicable for G.729 Decoder
*/
typedef struct IG729ABDEC_Params {
    ISPHDEC1_Params  isphdec1_params;
} IG729ABDEC_Params;

/*
// ===========================================================================
// IG729ABDEC_PARAMS
//
// Default parameter values for G729ABDEC instance objects
*/
extern ADT_API IG729ABDEC_Params IG729ABDEC_PARAMS;

/*
// ===========================================================================
//	IG729ABDEC_DynamicParams
//
//     This structure defines the parameters that can be
//              modified after creation via control() calls.
// All fields of isphdec1_DynamicParams (except size) 
// are not applicable for G.729 Decoder
*/
typedef struct IG729ABDEC_DynamicParams {
    ISPHDEC1_DynamicParams  isphdec1_DynamicParams;
} IG729ABDEC_DynamicParams;

/*
// ===========================================================================
//	IG729ABenC_InArgs
//
//     Defines the input arguments for all G729ABDEC instance process function.
*/
typedef struct IG729ABDEC_InArgs {
    ISPHDEC1_InArgs isphdec1_inArgs;
    XDAS_Int16      frameErase;
    XDAS_Int16 		padding;
} IG729ABDEC_InArgs;

/*
// ===========================================================================
//	IG729ABDEC_OutArgs
//
//  Defines the run time output arguments for all G729ABDEC instance objects.
*/
typedef struct IG729ABDEC_OutArgs {
    ISPHDEC1_OutArgs isphdec1_outArgs;
} IG729ABDEC_OutArgs;


/*
// ===========================================================================
// IG729ABDEC_Fxns
//
// This structure defines all of the operations on G729ABDEC objects
*/
typedef struct IG729ABDEC_Fxns {
    ISPHDEC1_Fxns   isphdec;    /* IG729ABDEC extends IAUDDEC_Fxns */
} IG729ABDEC_Fxns;
#endif	/* IG729ABDEC_ */
