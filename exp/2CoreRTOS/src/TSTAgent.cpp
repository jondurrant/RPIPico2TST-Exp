/*
 * TSTAgent.cpp
 *
 *  Created on: 28 Jun 2025
 *      Author: jondurrant
 */

#include "TSTAgent.h"
#include "Counter.h"




TSTAgent::TSTAgent() {
	// TODO Auto-generated constructor stub

}

TSTAgent::~TSTAgent() {
	// TODO Auto-generated destructor stub
}


void TSTAgent::run(){
		UBaseType_t uxCoreAffinityMask;
	    uxCoreAffinityMask = ( ( 1 << 0 ) );
	    vTaskCoreAffinitySet( xHandle, uxCoreAffinityMask );

	    char buf[25];

	uint32_t c0, c1;
	char s[20] = "TST Agent started\n";
	printf("TST Agent started\n");
	//stdio_usb.out_chars(s, 20);
	//stdio_usb.out_flush();
	int read = 0;
	uint32_t last = to_ms_since_boot (get_absolute_time());
	tstInit(&TST_Device);

	for (;;){

		read = stdio_usb.in_chars((char*)rxData, TSTMAXSIZE);
		if (read > 0) {
			//printf("Read %u\n", read);
			rxSize = read;
			tstRx(TST_Device.name, TST_Interface.interface, rxData, rxSize);
		}

		uint32_t now = to_ms_since_boot (get_absolute_time());
		if (now > (last + 200)){


			Counter::getInstance()->getCores(c0, c1);
			TST_V.core0Count = c0;
			TST_V.core1Count = c1;

			tstVariablesSet(
					TST_Device.name,
					TST_Interface.interface,
					&TST_V.core0Count,
					sizeof(TST_V.core0Count),
					&TST_V.core0Count);

			tstVariablesSet(
					TST_Device.name,
					TST_Interface.interface,
					&TST_V.core1Count,
					sizeof(TST_V.core1Count),
					&TST_V.core1Count);

			tstMonitorSend(TST_Device.name, TST_Interface.interface, "TST Device alive");
			if (tstTx(TST_Device.name, TST_Interface.interface, txData, &txSize) == TST_OK && txSize > 0) {

				stdio_usb.out_chars((char*)txData, txSize);
				stdio_usb.out_chars(buf, txSize);
				stdio_usb.out_flush();
				vTaskDelay(1);
				printf("Send(%u) data [%lu, %lu]\n", txSize, c0, c1);
			}

			last = now;
		}

		vTaskDelay(10);

	}
}

/***
 * Get the static depth required in words
 * @return - words
 */
configSTACK_DEPTH_TYPE TSTAgent::getMaxStackSize(){
	return 256;
}
