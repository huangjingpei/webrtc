/*******************************************************************************
* �㽭����ͼ����Ϣ��ҵ���޹�˾���������о��� 
* Copyright (c) 1997 - 2004
* All rights reserved.
* 
* $Id: Enc729.h,v 1.1 2004-08-10 07:38:41 gaokp Exp $
* 
* ժ   Ҫ����Ҫ�������ļ�������
*
* �����б���Ҫ�������书�ܼ�Ҫ˵�� 
* 
* �޸ļ�¼�����ļ�β
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
