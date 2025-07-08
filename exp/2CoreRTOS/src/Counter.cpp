/*
 * Counter.cpp
 *
 *  Created on: 17 Jan 2024
 *      Author: jondurrant
 */

#include "Counter.h"
#include <cstdio>

Counter *Counter::pSingleton = NULL;

Counter::Counter() {
	// TODO Auto-generated constructor stub

}

Counter::~Counter() {
	// TODO Auto-generated destructor stub
}


Counter * Counter::getInstance( uart_inst_t *uart){
	if (Counter::pSingleton == NULL){
		Counter::pSingleton = new Counter;
	}
	if (uart != NULL){
		Counter::pSingleton->setUart(uart);
	}
	return Counter::pSingleton;
}

void Counter::setUart(uart_inst_t *uart){
	pUart = uart;
}

void Counter::start(){
	print( "Start\n\r");
	xStartTime =  to_ms_since_boot(get_absolute_time());
	for (int i = 0; i < MAX_ID; i++){
		xCounts[i] = 0;
	}
	for (int i = 0; i < MAX_CORES; i++){
		xCoreCounts[i] = 0;
	}
}

void Counter::inc(uint8_t id){
	if (id < MAX_ID){
		if (xStopTime == 0){
			xCounts[id]++;
			xCoreCounts[get_core_num()]++;
		}
	}

}

void Counter::incCore(uint8_t id, uint8_t  core){
	if (id < MAX_ID){
		if (xStopTime == 0){
			xCounts[id]++;
		}
	}
	if (core < MAX_CORES){
		if (xStopTime == 0){
			xCoreCounts[core]++;
		}
	}
}

void Counter::report(){
	char line[80];
	 xStopTime =  to_ms_since_boot(get_absolute_time());

	 uint32_t sampleTime = xStopTime - xStartTime;

	 sprintf(line, "Sampled over %d sec and %d ms\n\r", sampleTime/1000, sampleTime%1000);
	 print(line);
	 print("#\t+Id\t+Core\n\r");
	 uint32_t total = 0;
	 for (int i = 0; i < MAX_ID; i++){
		 sprintf(line,"%d:\t%u\r", i, xCounts[i]);
		 print(line);
		 if (i < MAX_CORES){
			 sprintf(line,"\t%u\n\r", xCoreCounts[i]);
			 print(line);
		 } else {
			 print("\n\r");
		 }
		total += xCounts[i] ;
	 }

	 double perSec = (double)total /  ((double) sampleTime / 1000.0);

	 sprintf(line,"Total: %u \t%f per sec\n\r", total, perSec);
	 print(line);

}


void Counter::getCores(uint32_t &core0, uint32_t &core1){
	core0 = xCoreCounts[0];
	core1 = xCoreCounts[1];
}

void Counter::print(const char *s){
	if (pUart != NULL){
		uart_puts (pUart,  s);
	} else {
		printf(s);
	}
}
