/*
 * TSTMetrics.cpp
 *
 *  Created on: 8 Jul 2025
 *      Author: jondurrant
 */

#include "TSTMetrics.h"

TSTMetrics::TSTMetrics() {
	// TODO Auto-generated constructor stub

}

TSTMetrics::~TSTMetrics() {
	// TODO Auto-generated destructor stub
}

void TSTMetrics::run(){
	for (;;){
		vTaskList(stats_buffer);
		char msg[TSTMAXSIZE];

		for (int i=0; i < 20; i++){

			// Update TST_V with system/monitoring variables
			TST_V.heap_free = xPortGetFreeHeapSize();
			TST_V.heap_min_ever = xPortGetMinimumEverFreeHeapSize();
			TST_V.task_count = uxTaskGetNumberOfTasks();

			vTaskDelay(pdMS_TO_TICKS(100));
		}

		snprintf(msg, sizeof(msg), "Task List:\n%s", stats_buffer);
		tstMonitorSend(TST_Device.name, TST_Interface.interface, msg);

		snprintf(msg, sizeof(msg), "Heap free: %u bytes", TST_V.heap_free);
		tstMonitorSend(TST_Device.name, TST_Interface.interface, msg);

		snprintf(msg, sizeof(msg), "Heap min ever: %u bytes", TST_V.heap_min_ever);
		tstMonitorSend(TST_Device.name, TST_Interface.interface, msg);

		snprintf(msg, sizeof(msg), "Task count: %u", TST_V.task_count);
		tstMonitorSend(TST_Device.name, TST_Interface.interface, msg);


	}

}

configSTACK_DEPTH_TYPE TSTMetrics::getMaxStackSize(){
	return 1024;
}
