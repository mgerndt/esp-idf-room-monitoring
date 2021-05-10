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

#include "counting.h"
#include "commands.h"
#include "showRoomState.h"
#include "roomMonitoring.h"

 

#define GPIO_INPUT_PIN_SEL ((1ULL<<GPIO_INPUT_IO_0))

const uint8_t ledPin = 13;        //Onboard LED
const uint8_t interruptPinOut =32;  //Outer barrier interrupt pin
const uint8_t interruptPinIn =25;   //Inner barrier interrupt pin
const uint8_t triggerPinIn = 33;     //Pin to trigger inner barrier
const uint8_t triggerPinOut = 23;    //Pin to trigger outer barrier

#define GPIO_INPUT_PIN_SEL ((1ULL<<32)||(1ULL<<25))

int count; 
int prediction; 
const int capacityOfClass=99;

const char *TAG = "RoomMonitoring";
TaskHandle_t showRoomStateTask,publishRoomCountTask;

esp_mqtt_client_handle_t mqttClient = NULL;


static void textDemo()
{
	ssd1306_clearScreen();
	ssd1306_printFixedN(0,  0, "Normal text", STYLE_NORMAL,1);
	ssd1306_printFixed(0, 16, "Bold text", STYLE_BOLD);
	ssd1306_printFixed(0, 24, "Italic text", STYLE_ITALIC);
	ssd1306_negativeMode();
	ssd1306_printFixed(0, 32, "Inverted bold", STYLE_BOLD);
	ssd1306_positiveMode();
	delay(3000);
	ssd1306_clearScreen();	
}


void app_main(void)
{
	//Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);
	
	//esp_log_level_set("*", ESP_LOG_ERROR);
	esp_log_level_set("*", ESP_LOG_VERBOSE);       
	//esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
	
	
	ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
	ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());
	
	
	ESP_LOGI(TAG, "Initializing wifi");
	wifi_init_sta();
	
	initialize_sntp();
	initDisplay();
	//ssd1306_128x64_i2c_init();
	//ssd1306_setFixedFont(ssd1306xled_font6x8);

	ssd1306_clearScreen();
	//textDemo();
	mqttInit();
	mqttIotInit();
	
	ESP_ERROR_CHECK(gpio_set_direction(triggerPinOut, GPIO_MODE_OUTPUT));
	ESP_ERROR_CHECK(gpio_set_direction(triggerPinIn, GPIO_MODE_OUTPUT));
	
	
	ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_IRAM));
	
	ESP_ERROR_CHECK(gpio_set_direction(interruptPinOut, GPIO_MODE_INPUT));
	ESP_ERROR_CHECK(gpio_pullup_en(interruptPinOut));
	ESP_ERROR_CHECK(gpio_set_intr_type(interruptPinOut, GPIO_INTR_POSEDGE));
	ESP_ERROR_CHECK(gpio_isr_handler_add(interruptPinOut, outISR, NULL));

	ESP_ERROR_CHECK(gpio_set_direction(interruptPinIn, GPIO_MODE_INPUT));
	ESP_ERROR_CHECK(gpio_pullup_en(interruptPinIn));
	ESP_ERROR_CHECK(gpio_set_intr_type(interruptPinIn, GPIO_INTR_POSEDGE));
	ESP_ERROR_CHECK(gpio_isr_handler_add(interruptPinIn, inISR, NULL));
	
	
	
	xTaskCreatePinnedToCore(
		showRoomState,   /* Task function. */
		"showRoomState",     /* name of task. */
		2000,       /* Stack size of task */
		NULL,        /* parameter of the task */
		1,           /* priority of the task */
		&showRoomStateTask,      /* Task handle to keep track of created task */
		0);          /* pin task to core 0 */  


	xTaskCreatePinnedToCore(
		publishRoomCount,   /* Task function. */
		"publishRoomCount",     /* name of task. */
		4000,       /* Stack size of task */
		NULL,        /* parameter of the task */
		1,           /* priority of the task */
		&publishRoomCountTask,      /* Task handle to keep track of created task */
		0);          /* pin task to core 0 */  
	
// 	while (true){
// 		delay(2000);
// 		enterRoom();
// 	}
 }
