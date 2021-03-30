#ifndef ROOMMONITORING_H
#define ROOMMONITORING_H

#include <stdint.h>

extern const char *TAG;
extern const uint8_t interruptPinOut;
extern const uint8_t interruptPinIn;
extern const uint8_t triggerPinIn;
extern const uint8_t triggerPinOut;
extern const uint8_t ledPin;

extern int count; 
extern int prediction; 
extern const int capacityOfClass;


#endif
