/*
 * PCM_encryption.h
 *
 * Created: 2016/7/22 星期五 下午 13:18:10
 *  Author: Administrator
 */ 


#ifndef PCM_ENCRYPTION_H_
#define PCM_ENCRYPTION_H_
#include "compiler.h"

//volatile U16 PCM_Header[3];								
//volatile U16 PCM_rawdata[4];
volatile U16 PCM_frame_Payload[4]={0xABCD, 0x5A5A, 0x0000, 0x0000};

#define  Public_PCMkey 0xB2F5
//const U16 Public_PCMkey[] ={0x1AC3, 0x1840 , 0xA380 , 0x1949};	




#endif /* PCM_ENCRYPTION_H_ */