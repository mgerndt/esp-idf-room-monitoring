#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "roomMonitoring.h"
#include "commands.h"
#include "esp_random.h"


void traffic( void * pvParameters ){

	
	while (true){
    uint32_t randomValue = esp_random();

    if (randomValue<UINT32_MAX/3){
      leaveRoom();
    } else {
	    enterRoom();
    }
    vTaskDelay(2000 / portTICK_RATE_MS);
  }
}


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
	
	//expected outcome -1
}

void enterRoom(){
	ESP_LOGI(TAG,"Command: Enter");
	gpio_set_level(triggerPinOut,1);
	vTaskDelay(500 / portTICK_RATE_MS);
	gpio_set_level(triggerPinOut,0);
	vTaskDelay(500 / portTICK_RATE_MS);
	gpio_set_level(triggerPinIn,1);
	vTaskDelay(500 / portTICK_RATE_MS);
	gpio_set_level(triggerPinIn,0);
	vTaskDelay(500 / portTICK_RATE_MS);
	//expected outcome +1
}

void ping(){
	ESP_LOGI(TAG,"Command: ping");
	gpio_set_level(ledPin,1);
	vTaskDelay(1000 / portTICK_RATE_MS);
	gpio_set_level(ledPin,0);
	vTaskDelay(100 / portTICK_RATE_MS);
	//expected outcome no change
}


/*
Here a corner case from Group 4: The student almost enters the room. When the student decides to turn around the student has already unblocked the outer barrier but not the inner one. At the end there should be no change in the room count.
*/

void breaksOuterAndInnerButReturnsG4()
{
    ESP_LOGI(TAG, "Command: breakOuterAndInnerButReturnsG4");
    gpio_set_level(triggerPinOut, 1);
    vTaskDelay(200 / portTICK_RATE_MS);
    gpio_set_level(triggerPinIn, 1);
    vTaskDelay(200 / portTICK_RATE_MS);
    gpio_set_level(triggerPinOut, 0);
    vTaskDelay(200 / portTICK_RATE_MS);

    gpio_set_level(triggerPinOut, 1);
    vTaskDelay(200 / portTICK_RATE_MS);
    gpio_set_level(triggerPinIn, 0);
    vTaskDelay(200 / portTICK_RATE_MS);
    gpio_set_level(triggerPinOut, 0);
    vTaskDelay(200 / portTICK_RATE_MS);
	//expected outcome no change
	}

/*
Here is a corner case that details someone entering, shortly after someone also enters from the other side, then lets the first person pass through. 
	The other returns.
*/

void enterCongestionG10()
{
    ESP_LOGI(TAG,"Command: Enter Congestion");
    gpio_set_level(triggerPinOut,1);
    vTaskDelay(100 / portTICK_RATE_MS);
    gpio_set_level(triggerPinIn,1);
    vTaskDelay(1000 / portTICK_RATE_MS);
    gpio_set_level(triggerPinIn,0);
    vTaskDelay(100 / portTICK_RATE_MS);
    gpio_set_level(triggerPinOut,0);
    vTaskDelay(100 / portTICK_RATE_MS);
    gpio_set_level(triggerPinIn,1);
    vTaskDelay(100 / portTICK_RATE_MS);
    gpio_set_level(triggerPinIn,0);
    vTaskDelay(500 / portTICK_RATE_MS);
	//expected outcome +1
	}

/*
In this corner case, a person enters the room (breaking the first and then the second light barrier) turns around (while the second light barrier is still broken) and leaves the rooms (breaking the first light barrier again quickly after).
The difference to the halfwayEnter is, that in this case the first barrier is broken again when leaving. This is the case when a person turns around a bit later than in halfwayEnter, freeing the first light barrier.
*/

void personTurnedG9() {
    ESP_LOGI(TAG,"Command: Person entered the room and turned around");

    // person entering
    gpio_set_level(triggerPinOut, 1);
    vTaskDelay(100 / portTICK_RATE_MS);
    gpio_set_level(triggerPinOut, 0);
    vTaskDelay(100 / portTICK_RATE_MS);

    // person turning around
    gpio_set_level(triggerPinIn, 1);
    vTaskDelay(300 / portTICK_RATE_MS); // turning takes time
    gpio_set_level(triggerPinIn, 0);
    vTaskDelay(100 / portTICK_RATE_MS);

    // person left the room again
    gpio_set_level(triggerPinOut, 1);
    vTaskDelay(100 / portTICK_RATE_MS);
    gpio_set_level(triggerPinOut, 0);
    vTaskDelay(1000 / portTICK_RATE_MS);
		//expected outcome no change
		
}


// Someone almost enters, steps back, but then enters
void unsureEnter() {
    ESP_LOGI(TAG,"Command: Unsure Enter");
    gpio_set_level(triggerPinOut, 1);
    vTaskDelay(100 / portTICK_RATE_MS);
    gpio_set_level(triggerPinIn, 1);
    vTaskDelay(100 / portTICK_RATE_MS);
    gpio_set_level(triggerPinOut, 0);
    vTaskDelay(100 / portTICK_RATE_MS);
    gpio_set_level(triggerPinOut, 1);
    vTaskDelay(100 / portTICK_RATE_MS);
    gpio_set_level(triggerPinIn, 0);
    vTaskDelay(100 / portTICK_RATE_MS);
    gpio_set_level(triggerPinIn, 1);
    vTaskDelay(100 / portTICK_RATE_MS);
    gpio_set_level(triggerPinOut, 0);
    vTaskDelay(100 / portTICK_RATE_MS);
    gpio_set_level(triggerPinIn, 0);
    vTaskDelay(100 / portTICK_RATE_MS);
		//expected outcome n+1e
		
	}

//Someone is trying to manipulate the count by waving their arm through the barrier towards the inside
//Sequence is not possible if a person enters
void manipulationEnter(){ 
    ESP_LOGI(TAG,"Command: Manipulation Enter ");
    gpio_set_level(triggerPinOut,1);
    vTaskDelay(15 / portTICK_RATE_MS);
    gpio_set_level(triggerPinOut,0);
    vTaskDelay(15 / portTICK_RATE_MS);
    gpio_set_level(triggerPinIn, 1);
    vTaskDelay(15 / portTICK_RATE_MS);
    gpio_set_level(triggerPinIn, 0);
    vTaskDelay(500 / portTICK_RATE_MS);
		//expected outcome no change
		
}

//Someone is standing in the inside barrier, making counting impossible
void obstructionInside(){ 
    ESP_LOGI(TAG,"Command: Obstruction Inside");
    gpio_set_level(triggerPinIn, 1);
    vTaskDelay(6000 / portTICK_RATE_MS);
    gpio_set_level(triggerPinOut, 1);
    vTaskDelay(3000 / portTICK_RATE_MS);
  // Resolve obstruction
    gpio_set_level(triggerPinIn, 0);
    gpio_set_level(triggerPinOut, 0);
    vTaskDelay(500 / portTICK_RATE_MS);
		//expected outcome -1 may be error message?
		
	}

/*
	Someone peeks into the room and another exits afterwards.
	*/
void peakIntoandLeave(){
	ESP_LOGI(TAG,"Command: Peek in plus exit");
	gpio_set_level(triggerPinOut,1);
	vTaskDelay(100 / portTICK_RATE_MS);
	gpio_set_level(triggerPinOut,0);
	vTaskDelay(100 / portTICK_RATE_MS);

	gpio_set_level(triggerPinIn,1);
	vTaskDelay(100 / portTICK_RATE_MS);
	gpio_set_level(triggerPinIn,0);
	vTaskDelay(100 / portTICK_RATE_MS);
	gpio_set_level(triggerPinOut,1);
	vTaskDelay(100 / portTICK_RATE_MS);
	gpio_set_level(triggerPinOut,0);
	vTaskDelay(500 / portTICK_RATE_MS);
	//expected outcome -1
	
}

