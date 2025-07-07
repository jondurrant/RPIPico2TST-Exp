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
#include "hardware/uart.h"

extern"C"{
#include "pico/stdio/driver.h"
#include "pico/stdio.h"
#include "pico/stdio_usb.h"
#include "pico/stdio_uart.h"
}


#define DELAY_SHORT 200 // in microseconds
#define DELAY_LONG  1000 // in microseconds

#define TASK_PRIORITY      ( tskIDLE_PRIORITY + 1UL )

#define UART_ID uart1
#define UART_TX_PIN 4
#define UART_RX_PIN 5


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

  uart_init (UART_ID, 115200);
  gpio_set_function(UART_TX_PIN, UART_FUNCSEL_NUM(UART_ID, UART_TX_PIN));
  gpio_set_function(UART_RX_PIN, UART_FUNCSEL_NUM(UART_ID, UART_RX_PIN));


  TSTAgent tst(UART_ID);

  printf("Main task started\n");

  alarm_id_t alarm = add_alarm_in_ms(
  			60 * 1000,
  			alarmCB, NULL, false);

	Counter::getInstance()->start();
	tst.start("TST", TASK_PRIORITY+3);
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
	stdio_filter_driver(&stdio_uart);
	sleep_ms(2000);
	printf("Start\n");

	TaskHandle_t task;

   xTaskCreate(main_task, "MainThread", 2048, NULL, TASK_PRIORITY, &task);

  /* Start the tasks and timer running. */
	vTaskStartScheduler();

	for (;;){

	}
}
