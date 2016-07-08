/*
 * app.c
 *
 * Created: 2016/3/15 14:17:57
 *  Author: JHDEV2
 */ 
#include "app.h"
#include "../Log/log.h"

#include "Radio/xcmp.h"
#include "Radio/payload.h"

#include "fs/fs.h"
#include "rtc/rtc.h"
#include "radio/physical.h"

static __app_Thread_(app_cfg);

U32 bunchofrandomstatusflags;

U8 is_unmute = 0;
U8 Silent_flag = 0;
U8 Terminator_Flag = 0;
U8 AMBE_flag = 0;
volatile U8 VF_SN = 0;

/* Declare a variable that will be incremented by the hook function. */
unsigned long ulIdleCycleCount = 0UL;


/*until receive/transmit payload data*/
static void app_payload_rx_proc(void  * payload);
static void app_payload_tx_proc(void  * payload);

extern void payload_tx(void * payload);

//app func--list

void DeviceInitializationStatus_brdcst_func(xcmp_fragment_t  * xcmp)
{
	if (xcmp->u8[4] == 0x01)
	{
		bunchofrandomstatusflags |= 0x01;  //Need do nothing else.
	}
	else if(xcmp->u8[4] != 0x02)
	{
		bunchofrandomstatusflags  &= 0xFFFFFFFC; //Device Init no longer Complete.
		xcmp_DeviceInitializationStatus_request();
	}
}

void DeviceManagement_brdcst_func(xcmp_fragment_t * xcmp)
{
		U8 temp = 0;
		temp  = xcmp->u8[1] << 8;
		temp |= xcmp->u8[2];
		//if (temp == theXNL_Ctrlr.XNL_DeviceLogicalAddress)
		{
			if (xcmp->u8[0] == 0x01)
			{
				bunchofrandomstatusflags |= 0x00000002;
			}
			else
			{
				bunchofrandomstatusflags &= 0xFFFFFFFD;
			}
		}
}

void ToneControl_reply_func(xcmp_fragment_t * xcmp)
{
	if (xcmp->u8[0] == xcmp_Res_Success)
	{		
		log("Tone OK");
		//fl_write("/test.txt", FILE_END, (void *)"send tone ok\r\n", sizeof("send tone ok\r\n") - 1);
	}
	else
	{
		log("Tone error");
	}
}

void dcm_reply_func(xcmp_fragment_t * xcmp)
{
	if (xcmp->u8[0] == xcmp_Res_Success)
	{
		if(xcmp->u8[1] == DCM_ENTER)
		{
			log("\n\r Dcm-Enter OK \n\r");
			
		}
		else if (xcmp->u8[1] == DCM_EXIT)
		{
			log("\n\r Dcm-Exit OK \n\r");
		}
		else
		{
			log("\n\r Dcm-Revoke \n\r");
		}
		
		log("dcm OK-mo%X", xcmp->u8[3]);
	}
	else
	{
		log("dcm error");
	}
}


void dcm_brdcst_func(xcmp_fragment_t * xcmp)
{
	
	/*point to xcmp payload*/
	DeviceControlMode_brdcst_t *ptr = (DeviceControlMode_brdcst_t* )xcmp->u8;
	
	log("\n\r Dcm_brdcst \n\r");		
	log("\n\r Function: %x \n\r " ,  ptr->Function);
	log("\n\r ControlType: %x \n\r " ,  ptr->ControlType);
	log("\n\r ControlTypeSize: %x \n\r " ,  ptr->ControlTypeSize);
	
	
}

void mic_reply_func(xcmp_fragment_t * xcmp)
{
	/*point to xcmp payload*/
	MicControl_reply_t *ptr = (MicControl_reply_t* )xcmp->u8;
	
	log("\n\r Mic_reply \n\r");
	if (ptr->Result == 0x00)
	{
		
		if (ptr->Function == Mic_Disable)
		{
		
			log("\n\r Mic_close_ok \n\r " );
			log("\n\r Mic_type: %x \n\r " ,  ptr->Mic_Type);
			log("\n\r Signaling_type: %x \n\r " ,  ptr->Signaling_Type);
			log("\n\r Mic_state: %x \n\r " ,  ptr->Mic_State);
			log("\n\r Gain_offset: %x \n\r " ,  ptr->Gain_Offset);
			
		}
		else
		{
			log("\n\r Mic_function: %x \n\r ", ptr->Function );
		}
		
		
	}
	else 
	{
		
	
		log("\n\r Mic error \n\r");
		
	}
	
	
	
}

void mic_brdcst_func(xcmp_fragment_t * xcmp)
{
	/*point to xcmp payload*/
	MicControl_brdcast_t *ptr = (MicControl_brdcast_t* )xcmp->u8;
	//log("\n\r Mic_brdcst \n\r");		
	//log("\n\r Mic_type: %x \n\r " ,  ptr->Mic_Type);
	//log("\n\r Signal_type: %x \n\r " ,  ptr->Signaling_Type);
	//log("\n\r Mic_state: %x \n\r " ,  ptr->Mic_State);
	//log("\n\r Gain_offset: %x \n\r " ,  ptr->Gain_Offset);
			
	
}

void spk_reply_func(xcmp_fragment_t * xcmp)
{
	if (xcmp->u8[0] == xcmp_Res_Success)
	{
		
		if(xcmp->u8[4])
		{
			is_unmute = 1;
			
			//Silent_flag = 1;
		}
		log("spk OK -st%2x", xcmp->u8[4] );
		
	}
	else
	{
		log("spk error");
	}
}

void spk_brdcst_func(xcmp_fragment_t * xcmp)
{
	if (xcmp->u8[3] == xcmp_Res_Success)//0x0000:mute
	{
		is_unmute =0;
		Silent_flag = 0;
		log("spk_s_close ");
		
		
	}
	else
	{
		Silent_flag = 1;
		//is_unmute = 1;
		log("spk_s_open ");
	}
	
	
	
	
}


void Volume_reply_func(xcmp_fragment_t * xcmp)
{
	
	/*point to xcmp payload*/
	VolumeControl_reply_t *ptr = (VolumeControl_reply_t* )xcmp->u8;
	
		if (ptr->Result == xcmp_Res_Success)
		{
			if (ptr->Function == Enable_IntelligentAudio)
			{
				log("\n\r Enable_IA OK \n\r");
				log("\n\r Attenuator_Number: %x \n\r",  ((ptr->Attenuator_Number[0]<<8) | (ptr->Attenuator_Number[1])) );
	
			}
			else
			{
				
				log("\n\r VolumeControl: %x \n\r", ptr->Function);
				
			}
			
			
		}
		
		else
		{
			log("\n\r Enable_IA error \n\r");
		}
	
	
	
}

void Volume_brdcst_func(xcmp_fragment_t * xcmp)
{
	/*point to xcmp payload*/
	VolumeControl_brdcst_t *ptr = (VolumeControl_brdcst_t* )xcmp->u8;
	
	log("\n\r Attenuator_Number: %x \n\r",  ((ptr->Attenuator_Number[0]<<8) | (ptr->Attenuator_Number[1])) );
	
	log("\n\r Audio_Parameter: %x \n\r", ptr->Audio_Parameter);
	
	
}


void AudioRoutingControl_reply_func(xcmp_fragment_t * xcmp)
{
	if (xcmp->u8[0] == xcmp_Res_Success)
	{
		log("AudioRouting OK");
		//is_unmute = 1;
	}
	else
	{
		log("AudioRouting error");
		log("\n\r AudioRouting result: %x \n\r", xcmp->u8[0]);
		
	}
}


void AudioRoutingControl_brdcst_func(xcmp_fragment_t * xcmp)
{
	
	U16 num_routings = 0;
	U8 j = 0 ;
	
	num_routings = ((xcmp->u8[0]<< 8) | (xcmp->u8[1]) );
	log("\n\r num_routings: %d \n\r", num_routings);
	
	for(j = 0; j< num_routings ; j++ )
	{
		
		
		log("\n\r Audio-Input: %x \n\r", xcmp->u8[2+j*2]);
		log("\n\r Audio-Output: %x \n\r", xcmp->u8[3+j*2]);
		
		
	}
	
	log("\n\r Audio-Function: %x \n\r", xcmp->u8[3+j*2-1]);
	
	
	
}




void TransmitControl_reply_func(xcmp_fragment_t * xcmp)
{
	/*point to xcmp payload*/
	TransmitControl_reply_t *ptr = (TransmitControl_reply_t* )xcmp->u8;
	
	if (ptr->Result == xcmp_Res_Success)
	{
		
		log("\n\r  TransmitControl OK \n\r ");
		log("\n\r Function: %x \n\r", ptr->Function);
		log("\n\r Mode of Operation: %x \n\r", ptr->Mode_Of_Operation);
		log("\n\r State: %x \n\r", ptr->State);
		
		if (ptr->Function == KEY_UP)
		{
			//is_unmute = 1;
		}
		else if (ptr->Function ==DE_KEY)
		{
			is_unmute = 0;
		}
		else
		{
				//Silent_flag = 1;;
		}
		
		
		//Silent_flag = 1;
	}
	else
	{
		log("TransmitControl error");
	}
	

}


void TransmitControl_brdcst_func(xcmp_fragment_t * xcmp)
{
	/*point to xcmp payload*/
	//is_unmute = 1;
	
	TransmitControl_brdcast_t *ptr = (TransmitControl_brdcast_t* )xcmp->u8;
	//log("\n\r  TransmitControl broadcast \n\r ");
	//log("\n\r  Mode_Of_Operation: %x \n\r ", ptr->Mode_Of_Operation );
	//log("\n\r  State: %x \n\r ", ptr->State );
	//log("\n\r  State_change_reason: %x \n\r ", ptr->State_change_reason );
	//
	
	
}


void DataSession_reply_func(xcmp_fragment_t * xcmp)
{
	if (xcmp->u8[0] == xcmp_Res_Success)
	{
		log("\n\r DATArep OK \n\r");
		//log("\n\r Func: 0x %X \n\r", xcmp->u8[1]);
		//log("\n\r ID: 0x %X \n\r", xcmp->u8[2]);
		
	}
	else
	{
		log("\n\r Result:  %X \n\r", xcmp->u8[0]);
		log("\n\r DATArep error \n\r");
		log("\n\r Func:  %X \n\r", xcmp->u8[1]);
		log("\n\r ID:  %X \n\r", xcmp->u8[2]);
	}
	
}

void DataSession_brdcst_func(xcmp_fragment_t * xcmp)
{
	U8 Session_number = 0;
	U16 data_length = 0;
	U8 i = 0;
	/*point to xcmp payload*/
	DataSession_brdcst_t *ptr = (DataSession_brdcst_t* )xcmp->u8;

	if (ptr->State == CSBK_DATA_RX_Suc)
	{
		
		log("\n\r CSBK_RX OK \n\r");
		Session_number = ptr->DataPayload.Session_ID_Number;//xcmp->u8[1];
		
		data_length = (ptr->DataPayload.DataPayload_Length[0]<<8) | (ptr->DataPayload.DataPayload_Length[1]);//( xcmp->u8[2]<<8) | (xcmp->u8[3]);

		log("\n\r Session_ID: %x \n\r",Session_number );
		log("\n\r paylaod_length: %d \n\r",data_length );
		for(i=0; i<data_length; i++)
		{
			
			//log("\n\r payload[%d]: %X \n\r", i, xcmp->u8[4+i]);
			log("\n\r payload[%d]: %X \n\r", i, ptr->DataPayload.DataPayload[i]);
			
		}
		
	}
	else
	{
		//log("\n\r State: 0x %X \n\r", xcmp->u8[0]);
		log("\n\r State: 0x %X \n\r", ptr->State);
		
	}
	
}

void ButtonConfig_reply_func(xcmp_fragment_t * xcmp)
{
	/*point to xcmp payload*/
	ButtonConfig_reply_t *ptr = (ButtonConfig_reply_t* )(xcmp->u8);
	if (ptr->Result == xcmp_Res_Success)
	{
		log("\n\r Button_Config OK \n\r");
		
		log("\n\r Function: %X \n\r" , ptr->Function );
		
	}
	
	else
	{
		log("\n\r Button_Request error \n\r");
	}
	
}


void Phyuserinput_brdcst_func(xcmp_fragment_t * xcmp)
{
	U8 PUI_Source =0;
	U8 PUI_Type =0;
	U16 PUI_ID =0;
	U8 PUI_State =0;
	U8 PUI_State_Min_Value =0;
	U8 PUI_State_Max_Value =0;
	
	PUI_Source = xcmp->u8[0];
	PUI_Type = xcmp ->u8[1];
	PUI_ID = ((xcmp->u8[2]<<8) | xcmp->u8[3]);
	PUI_State = xcmp->u8[4];
	PUI_State_Min_Value = xcmp->u8[5];
	PUI_State_Max_Value = xcmp->u8[6];
	
	log("\n\r PhysicalUserInput_broadcast  \n\r"  );
	
	log("\n\r PUI_Source: %X \n\r" , PUI_Source);
	log("\n\r PUI_Type: %X \n\r" , PUI_Type);
	log("\n\r PUI_ID: %X \n\r" , PUI_ID);
	log("\n\r PUI_State: %X \n\r" , PUI_State);
	log("\n\r PUI_State_Min_Value: %X \n\r" , PUI_State_Min_Value);
	log("\n\r PUI_State_Max_Value: %X \n\r" , PUI_State_Max_Value);
	
	
	
	
	
}


void ButtonConfig_brdcst_func(xcmp_fragment_t * xcmp)
{
	U8 Num_Button =0;
	U8 i = 0 ;
	/*point to xcmp payload*/
	ButtonConfig_brdcst_t  *ptr = (ButtonConfig_brdcst_t* )xcmp->u8;
	
	Num_Button = ptr->NumOfButtons;
	
	log("\n\r ButtonConfig_broadcast  \n\r"  );
	
	log("\n\r Function: %X \n\r" , ptr->Function );
	
	log("\n\r NumOfButtons: %d \n\r" , Num_Button );
	
	log("\n\r ButtonInfoStructSize: %x \n\r" , ptr->ButtoInfoStructSize );
	
	for (i; i<Num_Button; i++)
	{
		log("\n\r ButtonInfo[%d].Bt_Identifier: %x \n\r" , i, 
				(ptr->ButtonInfo[i].ButtonIdentifier[0]<<8) | (ptr->ButtonInfo[i].ButtonIdentifier[1]) );
				
		log("\n\r ButtonInfo[%d].S_PressFeature: %x \n\r" , i,
				 (ptr->ButtonInfo[i].ShortPressFeature[0]<<8 )| (ptr->ButtonInfo[i].ShortPressFeature[1]) );
				 
		log("\n\r ButtonInfo[%d].Reserved1: %x \n\r" , i, 
				(ptr->ButtonInfo[i].Reserved1[0]<<8) |  (ptr->ButtonInfo[i].Reserved1[1]));
		
		log("\n\r ButtonInfo[%d].L_PressFeature: %x \n\r" , i,
				 (ptr->ButtonInfo[i].LongPressFeature[0]<<8) | (ptr->ButtonInfo[i].LongPressFeature[1]));
				 
		
		log("\n\r ButtonInfo[%d].Reserved2: %x \n\r" , i, 
				(ptr->ButtonInfo[i].Reserved2[0]<<8) | (ptr->ButtonInfo[i].Reserved2[1]));
		
	}
	

	
}


void SingleDetection_brdcst_func(xcmp_fragment_t * xcmp)
{
	if (xcmp->u8[0] == 0x11)
	{
		log("\n\r DMR_CSBK OK \n\r");
		
	}
	//if(xcmp->u8[1] == 0x11)
	else
	{
		log("SIGBRCST error");
		log("\n\r Signal_type: %X \n\r", xcmp->u8[0] );
	}
	

	//;
}



void EnOB_reply_func(xcmp_fragment_t * xcmp)
{
		/*point to xcmp payload*/
	//En_OB_Control_reply_t *ptr = (En_OB_Control_reply_t* )xcmp->u8;
	//log("\n\r Xcmp_opcode: %x \n\r", xcmp->xcmp_opcode);
	
	if (xcmp->u8[0]== xcmp_Res_Success)
	{
		if (xcmp->u8[1] == EN_OB_Enter)
		{
		
			log("\n\r En_OB_Enter OK \n\r");
			
		}
		else if (xcmp->u8[1] == EN_OB_Exit )
		{
			log("\n\r En_OB_Exit OK \n\r");
		}
		else
		{
			
			log("\n\r En_OB_Control: %x \n\r", xcmp->u8[1]);
		}
		
	}
	
	else
	{
		log("\n\r En_OB_Control error \n\r");
		log("\n\r En_OB_result: %x \n\r", xcmp->u8[0]);
		
	}
	
	
}

void EnOB_brdcst_func(xcmp_fragment_t * xcmp)
{
	
	
	log("\n\r En_OB Broadcast \n\r");
}



void FD_request_func(xcmp_fragment_t * xcmp)
{
	
	log("\n\r Forward Data Request \n\r");
	
	
}

void FD_reply_func(xcmp_fragment_t * xcmp)
{
	
	log("\n\r Forward Data Reply \n\r");
	
	
}

void FD_brdcst_func(xcmp_fragment_t * xcmp)
{
	
	
	log("\n\r Forward Data Broadcast \n\r");
	
}






static const volatile app_exec_t the_app_list[MAX_APP_FUNC]=
{
    /*XCMP_REQUEST,XCMP_REPLY,XCMP_BOARDCAST-*/
    {NULL, NULL, DeviceInitializationStatus_brdcst_func},// 0x400 -- Device Initialization Status
    {NULL, NULL, NULL},// 0x401 -- Display Text
    {NULL, NULL, NULL},// 0x402 -- Indicator Update
    {NULL, NULL, NULL},// 0x403 --
    {NULL, NULL, NULL},// 0x404 --
    {NULL, NULL, Phyuserinput_brdcst_func},// 0x405 -- Physical User Input Broadcast
    {NULL, Volume_reply_func, Volume_brdcst_func},// 0x406 -- Volume Control
    {NULL, spk_reply_func, spk_brdcst_func},// 0x407 -- Speaker Control
    {NULL, NULL, NULL},// 0x408 -- Transmit Power Level
    {NULL, ToneControl_reply_func, NULL},// 0x409 -- Tone Control
    {NULL, NULL, NULL},// 0x40A -- Shut Down
    {NULL, NULL, NULL},// 0x40B --
    {NULL, NULL, NULL},// 0x40C -- Monitor Control
    {NULL, NULL, NULL},// 0x40D -- Channel Zone Selection
    {NULL, mic_reply_func, mic_brdcst_func},// 0x40E -- Microphone Control
    {NULL, NULL, NULL},// 0x40F -- Scan Control
    {NULL, NULL, NULL},// 0x410 -- Battery Level
    {NULL, NULL, NULL},// 0x411 -- Brightness
    {NULL, ButtonConfig_reply_func, ButtonConfig_brdcst_func},// 0x412 -- Button Configuration
    {NULL, NULL, NULL},// 0x413 -- Emergency Control
    {NULL, AudioRoutingControl_reply_func, AudioRoutingControl_brdcst_func},// 0x414 -- Audio Routing Control
    {NULL, TransmitControl_reply_func, TransmitControl_brdcst_func},// 0x415 -- Transmit Control
    {NULL, NULL, NULL},// 0x416 --
    {NULL, NULL, NULL},// 0x417 --
    {NULL, NULL, NULL},// 0x418 --
    {NULL, NULL, NULL},// 0x419 --
    {NULL, NULL, NULL},// 0x41A --
    {NULL, NULL, SingleDetection_brdcst_func},// 0x41B -- Signal Detection Broadcast
    {NULL, NULL, NULL},// 0x41C -- Remote Radio Control
    {NULL, DataSession_reply_func, DataSession_brdcst_func},// 0x41D -- Data Session
    {NULL, NULL, NULL},// 0x41E -- Call Control
    {NULL, NULL, NULL},// 0x41F -- Menu or List Navigation
    {NULL, NULL, NULL},// 0x420 -- Menu Control
    {NULL, dcm_reply_func, dcm_brdcst_func},// 0x421 -- Device Control Mode
    {NULL, NULL, NULL},// 0x422 -- Display Mode Control
    {NULL, NULL, NULL},// 0x423 --
    {NULL, NULL, NULL},// 0x424 --
    {NULL, NULL, NULL},// 0x425 --
    {NULL, NULL, NULL},// 0x426 --
    {NULL, NULL, NULL},// 0x427 --
    {NULL, NULL, DeviceManagement_brdcst_func},// 0x428 -- Device Management
    {NULL, NULL, NULL},// 0x429 --
    {NULL, NULL, NULL},// 0x42A --
    {NULL, NULL, NULL},// 0x42B --
    {NULL, NULL, NULL},// 0x42C --
    {NULL, NULL, NULL},// 0x42D -- Sub-audible Deviation
    {NULL, NULL, NULL},// 0x42E -- Radio Alarm Control
    {NULL, NULL, NULL},// 0x42F --
    {NULL, NULL, NULL},// 0x430 --
    {NULL, NULL, NULL},// 0x431 --
    {NULL, NULL, NULL},// 0x432 --
    {NULL, NULL, NULL},// 0x433 --
    {NULL, NULL, NULL},// 0x434 --
    {NULL, NULL, NULL},// 0x435 --
    {NULL, NULL, NULL},// 0x436 --
    {NULL, NULL, NULL},// 0x437 --
    {NULL, NULL, NULL},// 0x438 --
    {NULL, NULL, NULL},// 0x439 -- Pin Control
    {NULL, NULL, NULL},// 0x43A --
    {NULL, NULL, NULL},// 0x43B --
    {NULL, NULL, NULL},// 0x43C --
    {NULL, NULL, NULL},// 0x43D --
    {NULL, NULL, NULL},// 0x43E -- Bluetooth Status
    {NULL, NULL, NULL},// 0x43F --
    {NULL, NULL, NULL},// 0x440 --
    {NULL, NULL, NULL},// 0x441 --
    {NULL, NULL, NULL},// 0x442 --
    {NULL, NULL, NULL},// 0x443 --
    {NULL, NULL, NULL},// 0x444 --
    {NULL, NULL, NULL},// 0x445 --
    {NULL, NULL, NULL},// 0x446 --
    {NULL, NULL, NULL},// 0x447 --
    {NULL, NULL, NULL},// 0x448 -- Voice Announcement Control
    {NULL, NULL, NULL},// 0x449 -- Keypad Lock Control
    {NULL, NULL, NULL},// 0x44A --
    {NULL, NULL, NULL},// 0x44B -- Radio Wide Parameter Control
    {NULL, NULL, NULL},// 0x44C --
    {NULL, NULL, NULL},// 0x44D --
    {NULL, NULL, NULL},// 0x44E -- Screen Saver
    {NULL, NULL, NULL},// 0x44F --
	{NULL, NULL, NULL},// 0x450 --
	{NULL, NULL, NULL},// 0x451 --
	{NULL, NULL, NULL},// 0x452 --
	{NULL, NULL, NULL},// 0x453 --
	{NULL, NULL, NULL},// 0x454 --
	{NULL, NULL, NULL},// 0x455 --
	{NULL, NULL, NULL},// 0x456 --
	{NULL, NULL, NULL},// 0x457 --
	{FD_request_func, FD_reply_func ,FD_brdcst_func},// 0x458 -- Forward Data
	{NULL, NULL, NULL},// 0x459 --
	{NULL, NULL, NULL},// 0x45A --
	{NULL, NULL, NULL},// 0x45B --
	{NULL, NULL, NULL},// 0x45C --
	{NULL, NULL, NULL},// 0x45D --
	{NULL, NULL, NULL},// 0x45E --
	{NULL, NULL, NULL},// 0x45F --
	{NULL, NULL, NULL},// 0x460 --
	{NULL, NULL, NULL},// 0x461 --
	{NULL, NULL, NULL},// 0x462 --
	{NULL, NULL, NULL},// 0x463 --
	{NULL, NULL, NULL},// 0x464 --
	{NULL, EnOB_reply_func, EnOB_brdcst_func},// 0x465 --Enhanced Option Board Mode
	{NULL, NULL, NULL},// 0x466 --
    {NULL, NULL, NULL},// 0x467 --
	{NULL, NULL, NULL},// 0x468 --
	{NULL, NULL, NULL},// 0x469 --	
														
		
};

void app_init(void)
{	
	payload_init( app_payload_rx_proc , app_payload_tx_proc );	
	xcmp_register_app_list(the_app_list);
			
	static portBASE_TYPE res = 0;
	 res = xTaskCreate(
	app_cfg
	,  (const signed portCHAR *)"XNL_TX"
	,  384
	,  NULL
	,  1
	,  NULL );
	
	
}

extern  char AudioData[];
extern U32 tc_tick;

static __app_Thread_(app_cfg)
{
	static int coun=0;
	static  U32 isAudioRouting = 0;
	static  portTickType xLastWakeTime;
	const portTickType xFrequency = 4000;//2s,定时问题已经修正。2s x  2000hz = 4000
	U8 Burst_ID = 0;
	U8 i = 0 ;
	
	static U8 * AMBE_payload_ptr = NULL;
	
	static U8 is_first = FALSE;
	
	 xLastWakeTime = xTaskGetTickCount();
	 AMBE_payload_ptr = get_payload_idle_isr();
		
	for(;;)
	{
		
		if((NULL== AMBE_payload_ptr))
		{
			
			AMBE_payload_ptr = get_payload_idle_isr();
			
			if((NULL== AMBE_payload_ptr))
			{
				break;
			}
		}
	 //AMBE_payload_ptr = get_payload_idle_isr();//获取新的空地址
		
		
		//if((++coun) % 200 ==0)
		if (0x00000003 == (bunchofrandomstatusflags & 0x00000003))//确认连接成功了，再发送请求
		{	
			//if((++coun) % 3 ==0)		
			{
				
				// xcmp_audio_route_speaker();
				//xcmp_IdleTestTone();
				
				if (Terminator_Flag == 1)
				{
					//xcmp_transmit_dekeycontrol();
					
				}
				
				if(isAudioRouting == 0)
				{
					//xcmp_data_session();
					//xcmp_audio_route_mic();
					//xcmp_button_config();
					//xcmp_audio_route_speaker();
					xcmp_enter_device_control_mode();//调换3个命令的顺序，则不会导致掉线。。。奇葩
					//xcmp_unmute_speaker();
					
					//is_unmute = 1;
					//xcmp_function_mic();
					
					isAudioRouting = 1;
				}
				else if(isAudioRouting == 1)
				{
					//xcmp_function_mic();
					//xcmp_data_session();
				   // xcmp_transmit_control();
					//xcmp_volume_control();
					xcmp_enter_enhanced_OB_mode();
					//xcmp_button_config();
					//xcmp_audio_route_speaker();
					//xcmp_unmute_speaker();
					//log("\n\r time: %d \n\r", tc_tick);
					
					isAudioRouting = 2;
					//isAudioRouting++;
				}
				else if(isAudioRouting == 2)
				{
					
					//xcmp_volume_control();
					//xcmp_data_session();
					xcmp_audio_route_AMBE();
					//xcmp_audio_route_speaker();
					//xcmp_unmute_speaker();
					//xcmp_enter_device_control_mode();
					//xcmp_mute_speaker();	
					//log("\n\r time: %d \n\r", tc_tick); 
					isAudioRouting = 3;
					
				}
				else if(isAudioRouting == 3)
				{
					//xcmp_unmute_speaker();
					//xcmp_enter_device_control_mode();
					//xcmp_exit_enhanced_OB_mode();
					//xcmp_mute_speaker();
					//xcmp_enhanced_OB_mode();
					isAudioRouting = 4;
					
				}
				else
				{
					isAudioRouting++;
				}
				//
					//switch(VF_SN)
					//{
						//case 0x01:
						//case 0x02:
						//case 0x03:
						//
							//Burst_ID = 0x0A;
							//log("\n\r Burst_ID: %x-%d \n\r", Burst_ID, VF_SN);
						//
							//break;
						//
						//case 0x04:
						//case 0x05:
						//case 0x06:
						//
							//Burst_ID = 0x0B;
							//log("\n\r Burst_ID: %x-%d \n\r", Burst_ID, VF_SN);
						//
							//break;
						//
						//case 0x07:
						//case 0x08:
						//case 0x09:
						//
							//Burst_ID = 0x0C;
							//log("\n\r Burst_ID: %x-%d \n\r", Burst_ID, VF_SN);
						//
							//break;
						//
						//case 0x0A:
						//case 0x0B:
						//case 0x0C:
						//
							//Burst_ID = 0x0D;
							//log("\n\r Burst_ID: %x-%d \n\r", Burst_ID, VF_SN);
						//
							//break;
						//
						//case 0x0D:
						//case 0x0E:
						//case 0x0F:
						//
							//Burst_ID = 0x0E;
							//log("\n\r Burst_ID: %x-%d \n\r", Burst_ID, VF_SN);
						//
							//break;
						//
						//case 0x10:
						//case 0x11:
						//case 0x12:
						//
							//Burst_ID = 0x0F;
							//log("\n\r Burst_ID: %x-%d \n\r", Burst_ID, VF_SN);
						//
							//break;
						//
						//default:
						//
							//Burst_ID = 0x00;
							////log("\n\r Burst_ID: %x-%d \n\r", Burst_ID, VF_SN);
							//break;
						//
						//
					//}
				//
				//log("\n\r ulIdleCycleCount: %d \n\r", ulIdleCycleCount);
				log("\n\r un: %d \n\r", is_unmute);
				//log("\n\r S_flag: %d \n\r", Silent_flag);
				//log("\n\r Tend_flag: %d \n\r", Terminator_Flag);
			
				log("\n\r AMBE_flag: %d \n\r", AMBE_flag);
				//log("\n\r VF_SN: %x \n\r",  VF_SN);
				//log("\n\r time: %d \n\r", tc_tick);
				
				if(isAudioRouting  == 6)
				{
					//xcmp_audio_route_speaker();
					//xcmp_unmute_speaker();
					
					//xcmp_audio_route_speaker();
					//xcmp_enter_device_control_mode();//调换3个命令的顺序，则不会导致掉线。。。奇葩
					//xcmp_unmute_speaker();
					//xcmp_enter_device_control_mode();
					xcmp_exit_device_control_mode();
					//log("\n\r time: %d \n\r", tc_tick); 
			
					
				}
				
				if (isAudioRouting > 6)
				{
					 if (is_first == FALSE)
					 {
						 
						 payload_tx(AMBE_payload_ptr);//读数据到此地址
						 is_first = TRUE;
						 
					 }
					 if (i>512)
					 {
						 i = 0;		 
					 }
					 log("\n\r AMBE_payload_ptr[%d] = %x", i, AMBE_payload_ptr[i]);
					 i++;
					 
					 
					 //payload_tx(AMBE_payload_ptr);//读数据到此地址
					 //AMBE_payload_ptr = get_payload_idle_isr();//获取新的空地址
				}
				
				//if(is_unmute == 2)
				//{
					//
					//
					//xcmp_mute_speaker();					
					//isAudioRouting = 4;
					//is_unmute = 1;
				//}
				//
				//if(isAudioRouting == 3)
				{
					//xcmp_audio_route_speaker();
					//xcmp_exit_device_control_mode();
					//isAudioRouting = 5;
				}
				//
				//if(isAudioRouting == 5)
				//{
					//xcmp_audio_route_revert();
					//isAudioRouting = 6;
				//}
			}
			
			
			//fs_saveVoiceInfo("/test.txt", (void *)"send tone \r\n", sizeof("send tone \r\n"));
			
			//log("time:%d-%d-%d %d:%d:%d",t.year, t.month, t.day, t.hour, t.minute, t.second);
			
			//log("testtime:%d", now->second);
		}
		//vTaskDelay(300*2 / portTICK_RATE_MS);//延迟300ms
		//log("\n\r ulIdleCycleCount: %d \n\r", ulIdleCycleCount);
		
		vTaskDelayUntil( &xLastWakeTime, 2000*2 / portTICK_RATE_MS  );//精确的以2000ms为周期执行。
	}
}


static void app_payload_rx_proc(void  * payload)
{
	log("\n\r w: \n\r");
	if (AMBE_flag)
	{
		fl_write("AMBEvo.bit", FILE_END, payload, MAX_PAYLOAD_BUFF_SIZE * 2);
	}
	else
	{
		fl_write("PCMvo.pcm", FILE_END, payload, MAX_PAYLOAD_BUFF_SIZE * 2);
	}
	
	//payload_fragment_t * ptr = (payload_fragment_t *)payload;
	set_payload_idle(payload);

}


static void app_payload_tx_proc(void  * payload)
{
  log("R");
  
  //if (AMBE_flag)
  {
	  fl_read("AMBEvo.bit", FILE_BEGIN, payload, MAX_PAYLOAD_BUFF_SIZE * 2);
  }
  //else
  //{
	  //fl_read("PCMvo.pcm", FILE_BEGIN, payload, MAX_PAYLOAD_BUFF_SIZE * 2);
  //}
  
  
  set_payload_idle(payload);


}

void vApplicationIdleHook( void )
{
	/* This hook function does nothing but increment a counter. */
	ulIdleCycleCount++;
	
}


