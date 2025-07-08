/*
 * TSTMetrics.cpp
 *
 *  Created on: 8 Jul 2025
 *      Author: jondurrant
 */

#include "TSTMetrics.h"
#include "Counter.h"

TSTMetrics::TSTMetrics() {
	// TODO Auto-generated constructor stub

}

TSTMetrics::~TSTMetrics() {
	// TODO Auto-generated destructor stub
}

void TSTMetrics::run(){
	for (;;){

		// Update TST_V with system/monitoring variables
		uint32_t c0, c1;
		Counter::getInstance()->getCores(c0, c1);
		TST_V.core0Count = c0;
		TST_V.core1Count = c1;

		vTaskDelay(pdMS_TO_TICKS(100));
	}

}

configSTACK_DEPTH_TYPE TSTMetrics::getMaxStackSize(){
	return 1024;
}
