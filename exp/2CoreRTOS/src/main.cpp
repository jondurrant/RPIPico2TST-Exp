/**
 * Calculate the value of PI using 4 Worker threads across 2 cores
 * Jon Durrant - 2024
 */

#include "pico/stdlib.h"
#include <stdio.h>
#include <cstdio>
#include <cstdlib>
#include <FreeRTOS.h>
#include "Counter.h"
#include "Worker.h"
#include "TSTAgent.h"
#include "TSTMetrics.h"
#include "hardware/uart.h"



#define TASK_PRIORITY      ( tskIDLE_PRIORITY + 1UL )

#define UART_ID uart0
#define UART_TX_PIN 16
#define UART_RX_PIN 17


Worker worker1(0);
Worker worker2(1);
Worker worker3(2);
Worker worker4(3);


int64_t alarmCB (alarm_id_t id, void *user_data){
	Counter::getInstance()->report();
	worker1.stop();
	worker2.stop();
	worker3.stop();
	worker4.stop();
	return 0;
}





void main_task(void* params){

  TSTAgent tst;
  TSTMetrics metrics;

  alarm_id_t alarm = add_alarm_in_ms(
  			60 * 1000,
  			alarmCB, NULL, false);

	Counter::getInstance(UART_ID)->start();
	tst.start("TST", TASK_PRIORITY);
	metrics.start("TXT Metrics",  TASK_PRIORITY);
	worker1.start("Worker 1", TASK_PRIORITY );
	worker2.start("Worker 2", TASK_PRIORITY);
	worker3.start("Worker 3", TASK_PRIORITY );
	worker4.start("Worker 4", TASK_PRIORITY);

  for (;;){
	  vTaskDelay(3000);
  }
}




int main() {


	//Initialise IO as we are using printf for debug
	stdio_init_all();

	uart_init (UART_ID, 115200);
	gpio_set_function(UART_TX_PIN, UART_FUNCSEL_NUM(UART_ID, UART_TX_PIN));
	gpio_set_function(UART_RX_PIN, UART_FUNCSEL_NUM(UART_ID, UART_RX_PIN));


	stdio_usb_init();
	// Wait for USB CDC to be connected (optional, but helps for debugging)
	while (!stdio_usb_connected()) {
		sleep_ms(10);
	}

	TaskHandle_t task;

   xTaskCreate(main_task, "MainThread", 2048, NULL, TASK_PRIORITY, &task);

  /* Start the tasks and timer running. */
	vTaskStartScheduler();

	for (;;){

	}
}
