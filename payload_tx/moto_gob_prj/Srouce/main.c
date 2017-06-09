/**
Copyright (C), Jihua Information Tech. Co., Ltd.

File name: ssc.c
Author: wxj
Version: 1.0.0.01
Date: 22016/3/15 14:20:51

Description:
History:
*/


#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"

#include "intc/intc.h"

#include "timers/timer.h"

#include "app/app.h"

#include "Log/log.h"
#include "Radio/xcmp.h"

#include "fs/fs.h"
#include "rtc/rtc.h"
#include "flash/data_flash.h"

int main(void)
{
	//Force SSC_TX_DATA_ENABLE Disabled as soon as possible.
	AVR32_GPIO.port[1].ovrs  =  0x00000001;  //Value will be high.
	AVR32_GPIO.port[1].oders =  0x00000001;  //Output Driver will be Enabled.
	AVR32_GPIO.port[1].gpers =  0x00000001;  //Enable as GPIO.
	
	Disable_global_interrupt();
	local_start_pll0();
		
	INTC_init_interrupts();
	
	log_init();		
	log("----start debug----");	
		
	//rtc_init();
	
	//fs_init();//65795机器无法通过文件系统初始化,究起原因，貌似是Radio对OB板的输出功率无法满足SD卡的正常工作。
	
	data_flash_init();

	tc_init();	
	
	app_init();
	
	xcmp_init();
		
	while ((AVR32_GPIO.port[1].pvr & 0x00000002) == 0); //Wait for FS High.
	while ((AVR32_GPIO.port[1].pvr & 0x00000002) != 0); //Wait for FS Low.
	local_start_timer();
	
	Enable_global_interrupt();
	
	vTaskStartScheduler();		
	return 0;
}

