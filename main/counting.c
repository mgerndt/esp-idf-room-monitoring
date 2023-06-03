#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#include "wifiInit.h"
#include "timeMgmt.h"
#include "mqtt.h"
#include "mqtt_client.h"

#include "ssd1306.h"

#include "roomMonitoring.h"


uint64_t timestampOut=0,timestampIn=0;
bool inFlag=false, outFlag=false;
const int debounceDelay=30;


/**
   This interrupt handler is called whenever the outer photoelectric barrier is broken.
*/
void IRAM_ATTR outISR(void* arg){
	//ets_printf("Interrupt OUT %ld %ld.\n",millis(), timestampOut);
	ets_printf("Interrupt OUT\n");
	if (millis()<timestampOut+debounceDelay) return;
	timestampOut=millis();
	//ets_printf("Interrupt OUT.\n");
	outFlag=true;
	
	if (inFlag){
		if (count > 0) {
		  count -= 1;
		}
		inFlag=false;
		outFlag=false;
	}
}

/**
   This interrupt handler is called whenever the outer photoelectric barrier is broken.
*/
void IRAM_ATTR inISR(void* arg){
	ets_printf("Interrupt IN.\n");
	
	if (millis()<timestampIn+debounceDelay) return;
	timestampIn=millis();
	//ets_printf("Interrupt IN.\n");
	inFlag=true;
	
	if (outFlag){
		if (count<capacityOfClass) {
		  count++;
		} else {
		  count=0;
		}
		inFlag=false;
		outFlag=false;
	}
} 
