#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include "commands.h"
#include "roomMonitoring.h"

const static int CONNECTED_BIT = BIT0;
extern esp_mqtt_client_handle_t mqttClient,mqttIotClient;
static EventGroupHandle_t mqtt_event_group;
esp_mqtt_client_handle_t mqttIotClient;

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    static int msg_id = 0;
	
    // your_context_t *context = event->context;
    switch (event->event_id) {
    case MQTT_EVENT_CONNECTED:
				ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
				xEventGroupSetBits(mqtt_event_group, CONNECTED_BIT);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        printf("ID=%d, total_len=%d, data_len=%d, current_data_offset=%d\n", event->msg_id, event->total_data_len, event->data_len, event->current_data_offset);
				if (strncmp(event->data,"enter",event->data_len)==0){
					enterRoom();
				} else
				if (strncmp(event->data,"leave",event->data_len)==0){
					leaveRoom();
				} else {
					ESP_LOGI(TAG, "Unknown command: %.*s",event->data_len, event->data);
				}
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        break;
	case MQTT_EVENT_BEFORE_CONNECT:
        ESP_LOGI(TAG, "MQTT_EVENT_BEFORE_CONNECT");
        break;
	default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
    return ESP_OK;
}



void mqttIotInit(void){
	mqtt_event_group = xEventGroupCreate();
	const esp_mqtt_client_config_t mqtt_cfg = {
		.event_handle = mqtt_event_handler,
		.host = "131.159.35.132",
		.username = "JWT",
		.password = "eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9.eyJpYXQiOjE2MTcxMjQ4ODMsImlzcyI6ImlvdHBsYXRmb3JtIiwic3ViIjoiMl8xMiJ9.gdMLXd5bQeAVwV5V2txhxbKc4-_opwu17ygiA_PIQRlwHtiW6JiwUc0f7N7L7Pr6kVfJvk4cJrhLl9yI53dJ4DMtZbdKOedPpClVxo1AWJok4eZQO4_TrJzqgVUpOgiL2R5DNEgFXA04xq1sb2kcw321PInaW_YImwM0VkIpBKHyD8m45T1BTKNWHlSee3qGsdRGj35XuKt_ikM4Jwo2vJLZdha_NJjIXwJDeFNLThF1G_bjIk7co5JKyJ01iEHDJ4QOVzmZrqpb650I_n4evIoGJCk9NIavAzC89ibUeqsc9tUt3myVX_FAM09ADGUKtbApIYazu5dKOQBnGBuF8kQyueRZRiEa5zxu47fIr48m5_daX_J7k6ocqDvxbZ9TYuSQwewrhpyBocjJ314K8NXWR1-g69VVGhJeSIoYTawi8mgAtZ2qyXr76XSVyBT3zgVch49bIL7FuW3NPhKqA84MzDW7_TQaPmDEKLbuPGDJuq85Ur1Z9dejxSFEOG43wZNDzTJunWDXAqV3FqkI6YvkQa92tx10E6anIZ25ob51ufTFb2hSufD_TdXb-Lk3Nhrddl83GFG4x2MKv3Ghfe20fLvGqxEG0WqlkDwgjd332FbGJ5rfzBu6N60qWzPkV9Z9n0W1uHBmIe6RVcwXp_emvefzrwGrLhd9xKNDJEQ",
		.port = 1883
	};
	
	ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
	mqttIotClient = esp_mqtt_client_init(&mqtt_cfg);
	esp_mqtt_client_start(mqttIotClient);
	xEventGroupWaitBits(mqtt_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
	ESP_LOGI(TAG, "After esp_mqtt_client_init()");
}

void publishRoomCount(void * pvParameters){
	time_t now = 0;
	char msg[256];
	
		
	for(;;){
		time(&now);
		sprintf(msg, "{\"username\":\"gerndt\",\"number\":%d,\"device_id\":\"12\",\"timestamp\":%lu000}", count,now);
		ESP_LOGI(TAG, "Topic %s: %s\n", "2_12", msg);
		// if (mqttIoTClient){
		// 	esp_mqtt_client_reconnect(mqttIotClient);
		// }

		//2_12_27 in Kibana
		int msg_id = esp_mqtt_client_publish(mqttIotClient,"2_12", msg, strlen(msg), 0, 0);
		if (msg_id==-1){
			ESP_LOGE(TAG, "msg_id returned by publish is -1!\n");
		} 
	
		vTaskDelay(20000 / portTICK_RATE_MS);
		//ESP_LOGI(TAG,"Task1 running on core %d",xPortGetCoreID());
	}
}

void mqttInit(void){
	static int msg_id = 0;
	
	mqtt_event_group = xEventGroupCreate();
	const esp_mqtt_client_config_t mqtt_cfg = {
		.event_handle = mqtt_event_handler,
		.host = "test.mosquitto.org",
		.port = 1883
	};
	
	
	mqttClient = esp_mqtt_client_init(&mqtt_cfg);
	esp_mqtt_client_start(mqttClient);
	xEventGroupWaitBits(mqtt_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
	msg_id = esp_mqtt_client_subscribe(mqttClient, "i10_iot",1);
  ESP_LOGI(TAG, "sent subscribe for i10_iot successful, msg_id=%d", msg_id);
}



