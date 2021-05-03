#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "roomMonitoring.h"



void leaveRoom(){
	ESP_LOGI(TAG,"Command: Leave");
	gpio_set_level(triggerPinIn,1);
	vTaskDelay(100 / portTICK_RATE_MS);
	gpio_set_level(triggerPinIn,0);
	vTaskDelay(100 / portTICK_RATE_MS);
	gpio_set_level(triggerPinOut,1);
	vTaskDelay(100 / portTICK_RATE_MS);
	gpio_set_level(triggerPinOut,0);
	vTaskDelay(500 / portTICK_RATE_MS);
}

void enterRoom(){
	ESP_LOGI(TAG,"Command: Enter");
	gpio_set_level(triggerPinOut,1);
	vTaskDelay(100 / portTICK_RATE_MS);
	gpio_set_level(triggerPinOut,0);
	vTaskDelay(100 / portTICK_RATE_MS);
	gpio_set_level(triggerPinIn,1);
	vTaskDelay(100 / portTICK_RATE_MS);
	gpio_set_level(triggerPinIn,0);
	vTaskDelay(500 / portTICK_RATE_MS);
}

