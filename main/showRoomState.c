#include "esp_event.h"
#include "timeMgmt.h"

#include "ssd1306.h"
#include "counting.h"
#include "showRoomState.h"
#include <stdio.h>
#include "esp_log.h"


#include "roomMonitoring.h"

#include <driver/adc.h>
#include <math.h>
#include "esp_pm.h"


bool displayON=true;

void initDisplay(){
	printf("Hallo\n");
	ssd1306_128x64_i2c_init();
	ssd1306_setFixedFont(ssd1306xled_font6x8);
}


void toggleDisplay( void * pvParameters ){

	while (true){
		displayON= !displayON;
		if (displayON){
			ESP_LOGI(TAG,"Display ON");
			ssd1306_displayOn();
		} else {
			ESP_LOGI(TAG,"Display OFF");
			ssd1306_displayOff();
		}
		vTaskDelay(10000 / portTICK_RATE_MS);
	}
}

void showRoomState( void * pvParameters ){
	int oldCount= -1;
	int lastMinute = 0;
	
	for(;;){
		delay(1000);
		//esp_pm_dump_locks(stdout);

	
		
		//ESP_LOGI(TAG,"Task1 running on core %d",xPortGetCoreID());
		struct tm timeinfo = { 0 };
		time_t now = 0;
		time(&now);
		localtime_r(&now, &timeinfo);

		if (displayON){
			if (oldCount!=count || lastMinute!=timeinfo.tm_min){
				oldCount=count;
				lastMinute=timeinfo.tm_min;
				
				ssd1306_clearScreen();
				
				char buf[64];
				
				sprintf(buf, "GE  %.2d:%.2d",timeinfo.tm_hour,timeinfo.tm_min);
				
				ssd1306_printFixedN(0,  0, buf, STYLE_NORMAL,1);
				
				sprintf(buf, "%.2d",count);
				ssd1306_printFixedN(0,  30, buf, STYLE_NORMAL,2);
				
				sprintf(buf, "%.2d",prediction);
				ssd1306_printFixedN(80,  30, buf, STYLE_NORMAL,2);
		}
		}
	} 
}

double getCurrent() {
	
	int val=adc1_get_raw(ADC1_CHANNEL_7); //Pin 35
	double current = (val-2300)/185; //185 mV pro ampere
	printf("mV: %d  curr: %f\n", val, current);
	return current;
}

void showCurrent( void * pvParameters ){
	int oldCount= -1;
	int lastMinute = 0;
	
	for(;;){
		delay(1000);
		
		//ESP_LOGI(TAG,"Task1 running on core %d",xPortGetCoreID());
		struct tm timeinfo = { 0 };
		time_t now = 0;
		time(&now);
		localtime_r(&now, &timeinfo);
	
		
			ssd1306_clearScreen();
			
			char buf[64];
			
			sprintf(buf, "GE  %.2d:%.2d",timeinfo.tm_hour,timeinfo.tm_min);
			
			ssd1306_printFixedN(0,  0, buf, STYLE_NORMAL,1);
			
			double current = getCurrent();
			sprintf(buf, "%1.3f",current);
			
			ssd1306_printFixedN(0,  30, buf, STYLE_NORMAL,1);
	} 
}
