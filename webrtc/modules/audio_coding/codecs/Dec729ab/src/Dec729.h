/*******************************************************************************
* �㽭����ͼ����Ϣ��ҵ���޹�˾���������о��� 
* Copyright (c) 1997 - 2004
* All rights reserved.
* 
* $Id: Dec729.h,v 1.1 2004-08-10 07:39:39 gaokp Exp $
* 
* ժ   Ҫ����Ҫ�������ļ�������
*
* �����б���Ҫ�������书�ܼ�Ҫ˵�� 
* 
* �޸ļ�¼�����ļ�β
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
