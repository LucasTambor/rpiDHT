#include "DHT.h"
// #include <stdio.h>


static dht_t DHT_TYPE;
static int PIN_DHT;
int bits[250];
int bitidx = 0;




void dht_configure(dht_t dhtType, int pinDHT)
{
    PIN_DHT = pinDHT;
    DHT_TYPE = dhtType; 

    GPIOUnexport(PIN_DHT);
    GPIOExport(PIN_DHT);
    GPIODirection(PIN_DHT, IN);

	// Wait 1 sec to stabilize sensor
	usleep(1000*1000);
}

int read_dht_data(float* temperature, float* humidity)
{
    uint8_t laststate	= HIGH;
	uint16_t counter	= 0;
	uint16_t j			= 0;
	uint16_t i			= 0;

	int data[100];

    // GPIOUnexport(PIN_DHT);
    // GPIOExport(PIN_DHT);
	GPIODirection(PIN_DHT, OUT);

	GPIOWrite(PIN_DHT, 1);
	usleep( 500*1000 );

	/* pull pin down for 20 milliseconds */
	GPIOWrite(PIN_DHT, 0);
	usleep( 20*1000 );

	/* prepare to read the pin */
    // GPIOUnexport(PIN_DHT);
    // GPIOExport(PIN_DHT);
    GPIODirection(PIN_DHT, IN);

	data[0] = data[1] = data[2] = data[3] = data[4] = 0;
	while(GPIORead(PIN_DHT) == 1)
	{
		usleep(1);
	}

	/* detect change and read data */
	for ( i = 0; i < MAX_TIMINGS; i++ )
	{
		counter = 0;
		while ( GPIORead(PIN_DHT) == laststate )
		{
			counter++;
			usleep(1);
			if ( counter == MAX_COUNTER )
			{
				#ifdef DEBUG_DHT
				fprintf(stderr, "DHT - No change - counter equals %d\n", MAX_COUNTER);
				#endif
				break;
			}
		}
		

		laststate = GPIORead(PIN_DHT);

		if ( counter == MAX_COUNTER )
        {
			#ifdef DEBUG_DHT
			fprintf(stderr, "DHT - So break!\n");
			#endif
			break;
        }
		bits[bitidx++] = counter;

		// #ifdef DEBUG_DHT
		// fprintf(stderr, "DHT - change! - data equals to %d\n", counter);
		// #endif

		/* ignore first 3 transitions */
		if ( (i >= 3) && (i % 2 == 0) )
		{
			/* shove each bit into the storage bytes */
			data[j / 8] <<= 1;
			if ( counter > 200 )
				data[j / 8] |= 1;
			j++;
		}

	}
	#ifdef DEBUG_DHT
	// for (int i=3; i<bitidx; i+=2) {
	// 	printf("bit %d: %d\n", i-3, bits[i]);
	// 	printf("bit %d: %d (%d)\n", i-2, bits[i+1], bits[i+1] > 200);

	// }
	printf("Data (%d): 0x%x 0x%x 0x%x 0x%x 0x%x\n", j, data[0], data[1], data[2], data[3], data[4]);
	#endif

	/*
	 * check we read 40 bits (8bit x 5 ) + verify checksum in the last byte
	 * print it out if data is good
	 */
	if ( (j >= 39) &&
	     (data[4] == ( (data[0] + data[1] + data[2] + data[3]) & 0xFF) ) )
	{

		printf("Temp = %d *C, Hum = %d \n", data[2], data[0]);

		float h = (float)data[0];
		float c = (float)data[2];
		// float h = (float)((data[0] << 8) + data[1]) / 10;
		// if ( h > 100 )
		// {
		// 	h = data[0];	// for DHT11
		// }
		// float c = (float)(((data[2] & 0x7F) << 8) + data[3]) / 10;
		// if ( c > 125 )
		// {
		// 	c = data[2];	// for DHT11
		// }
		// if ( data[2] & 0x80 )
		// {
		// 	c = -c;
		// }
		// float f = c * 1.8f + 32;

        *humidity = h;
        *temperature = c;
        return 0;
	}else  {
        return 1;
	}
}
