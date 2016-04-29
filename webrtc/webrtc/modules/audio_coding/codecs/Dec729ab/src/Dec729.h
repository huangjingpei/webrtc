/*******************************************************************************
* 浙江南望图像信息产业有限公司基础技术研究部 
* Copyright (c) 1997 - 2004
* All rights reserved.
* 
* $Id: Dec729.h,v 1.1 2004-08-10 07:39:39 gaokp Exp $
* 
* 摘   要：简要描述本文件的内容
*
* 函数列表：主要函数及其功能简要说明 
* 
* 修改记录：见文件尾
* 
*******************************************************************************/
#ifndef CODEC_DEC729_H
#define CODEC_DEC729_H
	
#ifdef __cplusplus
extern "C"
{
#endif

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
/*******************************************************************************
* $Log: not supported by cvs2svn $
 ******************************************************************************/
