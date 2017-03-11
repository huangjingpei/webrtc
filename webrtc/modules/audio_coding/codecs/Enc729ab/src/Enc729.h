/*******************************************************************************
* 浙江南望图像信息产业有限公司基础技术研究部 
* Copyright (c) 1997 - 2004
* All rights reserved.
* 
* $Id: Enc729.h,v 1.1 2004-08-10 07:38:41 gaokp Exp $
* 
* 摘   要：简要描述本文件的内容
*
* 函数列表：主要函数及其功能简要说明 
* 
* 修改记录：见文件尾
* 
*******************************************************************************/
#ifndef CODEC_ENC729_H
#define CODEC_ENC729_H
	
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
     unsigned char *Vout                  /* output:  output parameters         */      
);

#ifdef __cplusplus
}
#endif
	
#endif	
/*******************************************************************************
* $Log: not supported by cvs2svn $
 ******************************************************************************/
