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



void leaveRoom(){
	ESP_LOGI(TAG,"Command: Leave");
	gpio_set_level(triggerPinIn,1);
	delay(100);
	gpio_set_level(triggerPinIn,0);
	delay(100);
	gpio_set_level(triggerPinOut,1);
	delay(100);
	gpio_set_level(triggerPinOut,0);
	delay(500);
}

void enterRoom(){
	ESP_LOGI(TAG,"Command: Enter");
	gpio_set_level(triggerPinOut,1);
	delay(100);
	gpio_set_level(triggerPinOut,0);
	delay(100);
	gpio_set_level(triggerPinIn,1);
	delay(100);
	gpio_set_level(triggerPinIn,0);
	delay(500);
}

