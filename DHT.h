#ifndef DHT_H
#define DHT_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "gpioFileSys.h"
#include <unistd.h>

#define IN		0
#define OUT 	1

#define LOW 	0
#define HIGH 	1

#define MAX_TIMINGS	100
#define MAX_COUNTER 10000

#define DEBUG_DHT

typedef enum  
{
    DHT_11,
    DHT_22
} dht_t;

void dht_configure(dht_t dhtType, int pinDHT);

int read_dht_data(float* temperature, float* humidity);





#endif//DHT_H