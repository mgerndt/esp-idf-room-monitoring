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
#include "esp32/rom/rtc.h"

#include "ssd1306.h"

#include "counting.h"
#include "commands.h"
#include "showRoomState.h"
#include "roomMonitoring.h"
#include "log_buffer.h"
#include "smtp.h"
#include <driver/adc.h>
#include "esp_pm.h"

#include "esp_sleep.h"
#include "esp_wifi.h"


 

#define GPIO_INPUT_PIN_SEL ((1ULL<<GPIO_INPUT_IO_0))

const uint8_t ledPin = 19;        //Onboard LED
const uint8_t interruptPinOut =32;  //Outer barrier interrupt pin, gelb, right outer 4 from top
const uint8_t interruptPinIn =33;   //Inner barrier interrupt pin, weiss, right outer 3 from top
const uint8_t triggerPinIn = 5;     //Pin to trigger inner barrier, left outer 5 from top
const uint8_t triggerPinOut = 4;    //Pin to trigger outer barrier, left inner 6 from top

#define GPIO_INPUT_PIN_SEL ((1ULL<<32)||(1ULL<<25))


RTC_NOINIT_ATTR int count; 
int prediction; 
const int capacityOfClass=30;

const char *TAG = "RoomMonitoring";
TaskHandle_t showRoomStateTask,publishRoomCountTask;

esp_mqtt_client_handle_t mqttClient = NULL;


static void textDemo()
{
	ssd1306_clearScreen();
	ssd1306_printFixedN(0,  0, "Welcome", STYLE_NORMAL,1);
	// ssd1306_printFixed(0, 16, "Bold text", STYLE_BOLD);
	// ssd1306_printFixed(0, 24, "Italic text", STYLE_ITALIC);
	// ssd1306_negativeMode();
	// ssd1306_printFixed(0, 32, "Inverted bold", STYLE_BOLD);
	// ssd1306_positiveMode();
	delay(3000);
	ssd1306_clearScreen();	
}


void configPM(void * pvParameters){
	int count=0;
	esp_pm_config_esp32_t arr[] = {
            {  80,  80, false},
            { 160, 160, false},
            { 240, 240, false},
            {  80,  80, true},
            { 160, 160, true},
            { 240, 240, true},
    };
    // esp_pm_config_esp32_t pm_config = {
    //     .max_freq_mhz = 80,
    //     .min_freq_mhz = 80,         //DFS, enable in menucofig in Power Management
    //     .light_sleep_enable = false   //automatic light sleep, enable via menuconfig in FreeRTOS
    // };
	while (true){
		//esp_wifi_stop();
		ESP_ERROR_CHECK(esp_pm_configure(&arr[count]));
		//esp_wifi_start();
		count=count+1;
		count=count%6;
		//ESP_LOGI("power","power settings (%d,%d,%d)", arr[count].min_freq_mhz, arr[count].max_freq_mhz, arr[count].light_sleep_enable);
		vTaskDelay(20000 / portTICK_RATE_MS);
	}
    //ESP_ERROR_CHECK(esp_pm_configure(&pm_config));

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
	
	esp_log_level_set("*", ESP_LOG_ERROR);
	esp_log_level_set("*", ESP_LOG_INFO);       
	//esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
	//esp_log_level_set("wifi", ESP_LOG_VERBOSE);

	ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_IRAM));
	
#ifdef SEMINAR_ROOM
	ESP_ERROR_CHECK(gpio_set_direction(interruptPinOut, GPIO_MODE_INPUT));
	ESP_ERROR_CHECK(gpio_pulldown_en(interruptPinOut));
	ESP_ERROR_CHECK(gpio_set_intr_type(interruptPinOut, GPIO_INTR_POSEDGE));
	ESP_ERROR_CHECK(gpio_isr_handler_add(interruptPinOut, outISR, NULL));

	ESP_ERROR_CHECK(gpio_set_direction(interruptPinIn, GPIO_MODE_INPUT));
	ESP_ERROR_CHECK(gpio_pulldown_en(interruptPinIn));
	ESP_ERROR_CHECK(gpio_set_intr_type(interruptPinIn, GPIO_INTR_POSEDGE));
	ESP_ERROR_CHECK(gpio_isr_handler_add(interruptPinIn, inISR, NULL));
#else
	ESP_ERROR_CHECK(gpio_set_direction(interruptPinOut, GPIO_MODE_INPUT));
	ESP_ERROR_CHECK(gpio_pulldown_en(interruptPinOut));
	ESP_ERROR_CHECK(gpio_set_intr_type(interruptPinOut, GPIO_INTR_POSEDGE));
	ESP_ERROR_CHECK(gpio_isr_handler_add(interruptPinOut, outISR, NULL));

	ESP_ERROR_CHECK(gpio_set_direction(interruptPinIn, GPIO_MODE_INPUT));
	ESP_ERROR_CHECK(gpio_pulldown_en(interruptPinIn));
	ESP_ERROR_CHECK(gpio_set_intr_type(interruptPinIn, GPIO_INTR_POSEDGE));
	ESP_ERROR_CHECK(gpio_isr_handler_add(interruptPinIn, inISR, NULL));

	ESP_ERROR_CHECK(gpio_set_direction(triggerPinIn, GPIO_MODE_OUTPUT));
	ESP_ERROR_CHECK(gpio_set_direction(triggerPinOut, GPIO_MODE_OUTPUT));

	// while (true){
	// 	gpio_set_level(triggerPinOut,HIGH);
	// 	ESP_LOGI(TAG, "%d 1",triggerPinOut);
	// 	vTaskDelay(3000 / portTICK_RATE_MS);
	// 	gpio_set_level(triggerPinOut,LOW);
	// 	ESP_LOGI(TAG, "%d 0",triggerPinOut);
	// 	vTaskDelay(3000 / portTICK_RATE_MS);
	// 	gpio_set_level(triggerPinIn,1);
	// 	ESP_LOGI(TAG, "%d 1",triggerPinIn);
	// 	vTaskDelay(3000 / portTICK_RATE_MS);
	// 	gpio_set_level(triggerPinIn,0);
	// 	ESP_LOGI(TAG, "%d 0",triggerPinIn);
	// 	vTaskDelay(3000 / portTICK_RATE_MS);
	// }
#endif	


	
	
	if (rtc_get_reset_reason(0) == POWERON_RESET){
		count=0;
		bufferInit();
		ESP_LOGI(TAG, "Init: count %d",count);
				
	}
	ESP_LOGI(TAG, "count %d",count);
	//configPM();

	//esp_log_set_vprintf(&bufferAppend);
	
	
	ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
	ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());
	
	
	ESP_LOGI(TAG, "Initializing wifi");
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	//Creates a station network interface object
		
	wifi_init_sta();
	
	initialize_sntp();

	//i2cdev_init();
	//ssd1306_128x64_i2c_init();
	//ssd1306_setFixedFont(ssd1306xled_font6x8);
	initDisplay();

	
	ssd1306_clearScreen();
	textDemo();
	//mqttInit();
	mqttIotInit();
	

	

	
	

	
	ESP_ERROR_CHECK(gpio_set_direction(ledPin, GPIO_MODE_OUTPUT));
	gpio_set_level(ledPin,0);
	

	// esp_sleep_enable_gpio_wakeup();
	// ESP_ERROR_CHECK(esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON));

	//gpio_wakeup_enable(32, GPIO_INTR_HIGH_LEVEL);
	// ESP_ERROR_CHECK(gpio_pulldown_en(interruptPinIn));
	// gpio_wakeup_enable(interruptPinIn, GPIO_INTR_HIGH_LEVEL);



	
	xTaskCreatePinnedToCore(
		showRoomState,   /* Task function. */
		"showRoomState",     /* name of task. */
		2000,       /* Stack size of task */
		NULL,        /* parameter of the task */
		1,           /* priority of the task */
		&showRoomStateTask,      /* Task handle to keep track of created task */
		0);          /* pin task to core 0 */  

	
    // int sleep_sec = 3;
    // ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(1000000LL * sleep_sec));
    // ESP_LOGI(TAG,"Starting light sleep");  
	// 	vTaskDelay(1000 / portTICK_RATE_MS);
    // esp_light_sleep_start();
    // ESP_LOGI(TAG,"Woke up from light sleep");  
	// 	vTaskDelay(1000 / portTICK_RATE_MS);



    // ESP_LOGI(TAG,"Starting deep sleep");  
	// 	vTaskDelay(1000 / portTICK_RATE_MS);
    // esp_deep_sleep(1000000LL * sleep_sec);


		
	xTaskCreatePinnedToCore(
		publishRoomCount,   /* Task function. */
		"publishRoomCount",     /* name of task. */
		8000,       /* Stack size of task */
		NULL,        /* parameter of the task */
		1,           /* priority of the task */
		&publishRoomCountTask,      /* Task handle to keep track of created task */
		0);          /* pin task to core 0 */  
	
	
	// xTaskCreatePinnedToCore(
	// 	monitorMQTT,   /* Task function. */
	// 	"monitorMQTT",     /* name of task. */
	// 	4000,       /* Stack size of task */
	// 	NULL,        /* parameter of the task */
	// 	1,           /* priority of the task */
	// 	NULL,      /* Task handle to keep track of created task */
	// 	0);          /* pin task to core 0 */  
	
	// xTaskCreatePinnedToCore(
	// 	traffic,   /* Task function. */
	// 	"StudentSimulation",     /* name of task. */
	// 	2000,       /* Stack size of task */
	// 	NULL,        /* parameter of the task */
	// 	1,           /* priority of the task */
	// 	NULL,      /* Task handle to keep track of created task */
	// 	0);          /* pin task to core 0 */  
	
	// xTaskCreatePinnedToCore(
	// 	toggleDisplay,   /* Task function. */
	// 	"Display",     /* name of task. */
	// 	2000,       /* Stack size of task */
	// 	NULL,        /* parameter of the task */
	// 	1,           /* priority of the task */
	// 	NULL,      /* Task handle to keep track of created task */
	// 	0);          /* pin task to core 0 */  
	

	// xTaskCreatePinnedToCore(
	// 	configPM,   /* Task function. */
	// 	"PowerSettings",     /* name of task. */
	// 	2000,       /* Stack size of task */
	// 	NULL,        /* parameter of the task */
	// 	1,           /* priority of the task */
	// 	NULL,      /* Task handle to keep track of created task */
	// 	0);          /* pin task to core 0 */  
	

}
