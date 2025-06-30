/**
 * Jon Durrant.
 *
 * Blink LED on GPIO 02
 */

#include "pico/stdlib.h"
#include <stdio.h>
#include "tst_variables.h"

#define DELAY 500 // in microseconds

uint8_t rxData[TSTMAXSIZE];
size_t rxSize = 0;
uint8_t txData[TSTMAXSIZE];
size_t txSize = 0;

int main() {
	bool ledState = false;
    const uint LED_PIN =  PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);

    gpio_set_dir(LED_PIN, GPIO_OUT);

    stdio_init_all();
    sleep_ms(2000);

    // Register device
    tstInit(&TST_Device);

    uint32_t last = to_ms_since_boot (get_absolute_time());


    while (true) { // Loop forever

    	int l = stdio_get_until (rxData,  TSTMAXSIZE, make_timeout_time_ms (200) );
    	// Process the received data using the tst protocol
		if (l > 0) {
			rxSize = l;
			tstRx(TST_Device.name, TST_Interface.interface, rxData, rxSize);
		}


		TST_V.myVariable++;


    	uint32_t now = to_ms_since_boot (get_absolute_time());
    	if (now > (last + 200)){
    		gpio_put(LED_PIN, ledState);
    		TST_V.myLED = ledState;
    		ledState = ! ledState;
    		last = now;


    		tstMonitorSend(TST_Device.name, TST_Interface.interface, "TST Device alive");
    		if (tstTx(TST_Device.name, TST_Interface.interface, txData, &txSize) == TST_OK && txSize > 0) {
				for (int i=0; i < txSize; i++){
					stdio_putchar_raw (txData[i]);
				}
			}

    	}

    }

}
