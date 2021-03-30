#include "esp_event.h"
#include "timeMgmt.h"
#include "ssd1306.h"
#include "counting.h"
#include "showRoomState.h"
#include <stdio.h>
#include "ssd1306.h"
#include "esp_log.h"
#include "roomMonitoring.h"

void initDisplay(){
	ssd1306_128x64_i2c_init();
	ssd1306_setFixedFont(ssd1306xled_font6x8);
	
}

void showRoomState( void * pvParameters ){
	for(;;){
		delay(1000);
		//ESP_LOGI(TAG,"Task1 running on core %d",xPortGetCoreID());
		
		ssd1306_clearScreen();
	
		char buf[64];
		struct tm timeinfo = { 0 };
		time_t now = 0;
		time(&now);
		
		localtime_r(&now, &timeinfo);
		sprintf(buf, "G10  %.2d:%.2d",timeinfo.tm_hour,timeinfo.tm_min);
	
		ssd1306_printFixedN(0,  0, buf, STYLE_NORMAL,1);
		
		sprintf(buf, "%.2d",count);
		ssd1306_printFixedN(0,  30, buf, STYLE_NORMAL,2);
		
		sprintf(buf, "%.2d",prediction);
		ssd1306_printFixedN(80,  30, buf, STYLE_NORMAL,2);
	} 
}

