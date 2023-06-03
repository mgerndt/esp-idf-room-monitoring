#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "log_buffer.h"





RTC_NOINIT_ATTR static char logBuffer[LOG_BUFFER_SIZE];



void bufferInit(){
	for (int i=0; i<LOG_BUFFER_SIZE; i++){
		logBuffer[i]='\000';
	}
}

void printBuffer(){
	printf("logBuffer:<%s>\n",logBuffer);
}

char *getBuffer(){
	return &logBuffer;
}

int bufferAppend(const char *format, va_list args)
{
	//va_list args;
	int done;
	char str[LOG_BUFFER_HELPER_SIZE];
	//va_start (args, format);
	//done = snprintf(str, LOG_BUFFER_SIZE, format, args);
	done=vsnprintf(str, LOG_BUFFER_HELPER_SIZE, format, args);
	printf(str);
	//va_end (args);
	
	//printBuffer();
	
	if (strlen(str)>LOG_BUFFER_SIZE) str[LOG_BUFFER_SIZE-1]='\000';
	//printBuffer();
	
	int logLength=strlen(logBuffer);
	int shift=logLength+strlen(str)-LOG_BUFFER_SIZE+1;
	if (shift>0){
		for (int i=0; i<logLength-shift;i++){
			logBuffer[i]=logBuffer[i+shift];
		}
		logBuffer[logLength-shift]='\000';
		//printBuffer();
	}
	strcat(logBuffer, str);
	//printBuffer();
	return done;
}

int bufferAppendHelp(const char * format, ...){
	va_list args;
	int done;
	
	va_start (args, format);
	done=bufferAppend(format, args);
	va_end (args);
	return done;
}
