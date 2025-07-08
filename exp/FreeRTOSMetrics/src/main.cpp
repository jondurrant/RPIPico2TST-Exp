/**
 *TST Test on RPI Pico2
 * Jon Durrant - 2024
 */

#include "pico/stdlib.h"
#include <stdio.h>
#include <cstdio>
#include <cstdlib>
#include <FreeRTOS.h>
#include "TSTAgent.h"
#include "TSTMetrics.h"
#include "BlinkAgent.h"
#include "hardware/uart.h"


#define DELAY_SHORT 200 // in microseconds
#define DELAY_LONG  1000 // in microseconds

#define TASK_PRIORITY      ( tskIDLE_PRIORITY + 1UL )


#define UART_ID uart0
#define UART_TX_PIN 16
#define UART_RX_PIN 17






void main_task(void* params){


  //TSTAgent tst(UART_ID);
 TSTAgent tst;
  tst.start("TST", TASK_PRIORITY);

  TSTMetrics metrics;
  metrics.start("Metrics", TASK_PRIORITY);

  BlinkAgent blink;
  blink.start("Blink", TASK_PRIORITY);


  for (;;){
	  vTaskDelay(3000);
  }
}




int main() {


	//Initialise IO as we are using printf for debug
	stdio_init_all();

	/* Uart alternative comms
	uart_init (UART_ID, 115200);
	gpio_set_function(UART_TX_PIN, UART_FUNCSEL_NUM(UART_ID, UART_TX_PIN));
	gpio_set_function(UART_RX_PIN, UART_FUNCSEL_NUM(UART_ID, UART_RX_PIN));
	*/

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
