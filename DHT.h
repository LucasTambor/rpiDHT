#ifndef DHT_H
#define DHT_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "gpioFileSys.h"
#include <unistd.h>

#define MAX_TIMINGS	85

typedef enum  
{
    DHT_11,
    DHT_22
} dht_t;

void dht_configure(dht_t dhtType, int pinDHT);

int read_dht_data(float* temperature, float* humidity);





#endif//DHT_H