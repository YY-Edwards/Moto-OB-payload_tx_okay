/*
 * ambe.h
 *
 * Created: 2016/6/21  14:42:36
 *  Author: Administrator
 */ 


#ifndef AMBE_H_
#define AMBE_H_

#include "compiler.h"

//assemble 2 items into 1 command and send it to the OB.
volatile U16 SendAMBEBurst_EncryptionPackage[12*10];	//Vocoder Bits Stream Parameter + 20ms AMBE bits of Voice Burst A(B,C,D,E,F)
volatile U16 VBSP_data[2];				//Vocoder Bits Stream Parameter,RX:keep Radio Vocoder Bits Stream Parameter unchanged.
volatile U16 AMBEBurst_rawdata[4];		//Radio shall send 20ms AMBE bits of Voice Burst A(B,C,D,E,F)49bits


 typedef enum {
	  
	  VOICE_WATING,
	  VOICEHEADER,
	  UNSUREDATA,
	  VOICEBURST_A,
	  VOICEBURST_B,
	  VOICEBURST_C,
	  VOICEBURST_D,
	  VOICEBURST_E,
	  VOICEBURST_F,
	  VOICETERMINATOR
	  
 }RxAMBEBurstType; //enum are 32 bits.



/*Calculate the Burst flag of receiving relative Vocoder Bits Streams */

volatile RxAMBEBurstType CalculateBurst(U8 vf_sn);


#endif /* AMBE_H_ */