#include "DHT.h"


static dht_t DHT_TYPE;
static int PIN_DHT;

int data[5] = { 0, 0, 0, 0, 0 };


void dht_configure(dht_t dhtType, int pinDHT)
{
    PIN_DHT = pinDHT;
    DHT_TYPE = dhtType; 

    GPIOUnexport(PIN_DHT);
    GPIOExport(PIN_DHT);
    GPIODirection(PIN_DHT, OUT);
}

int read_dht_data(float* temperature, float* humidity)
{
    uint8_t laststate	= HIGH;
	uint8_t counter		= 0;
	uint8_t j			= 0, i;

	data[0] = data[1] = data[2] = data[3] = data[4] = 0;

	/* pull pin down for 18 milliseconds */
	GPIODirection(PIN_DHT, OUT);
	GPIOWrite(PIN_DHT, 1);

    // TODO
	usleep( 18*1000 );

	/* prepare to read the pin */
	GPIODirection(PIN_DHT, IN);

	/* detect change and read data */
	for ( i = 0; i < MAX_TIMINGS; i++ )
	{
		counter = 0;
		while ( GPIORead(PIN_DHT) == laststate )
		{
			counter++;
			usleep( 1 );
			if ( counter == 255 )
			{
				break;
			}
		}
		laststate = GPIORead(PIN_DHT);

		if ( counter == 255 )
        {
			break;
        }

		/* ignore first 3 transitions */
		if ( (i >= 4) && (i % 2 == 0) )
		{
			/* shove each bit into the storage bytes */
			data[j / 8] <<= 1;
			if ( counter > 16 )
				data[j / 8] |= 1;
			j++;
		}

        
	}

    fprintf(stderr, "i: %d \n", i);
    fprintf(stderr, "j: %d\n", j);

	/*
	 * check we read 40 bits (8bit x 5 ) + verify checksum in the last byte
	 * print it out if data is good
	 */
	if ( (j >= 40) &&
	     (data[4] == ( (data[0] + data[1] + data[2] + data[3]) & 0xFF) ) )
	{
		float h = (float)((data[0] << 8) + data[1]) / 10;
		if ( h > 100 )
		{
			h = data[0];	// for DHT11
		}
		float c = (float)(((data[2] & 0x7F) << 8) + data[3]) / 10;
		if ( c > 125 )
		{
			c = data[2];	// for DHT11
		}
		if ( data[2] & 0x80 )
		{
			c = -c;
		}
		float f = c * 1.8f + 32;

        *humidity = h;
        *temperature = c;
        fprintf(stderr, "Humidity = %.1f %% Temperature = %.1f *C (%.1f *F)\n", h, c, f );
        return 0;
	}else  {
        return 1;
	}
}
