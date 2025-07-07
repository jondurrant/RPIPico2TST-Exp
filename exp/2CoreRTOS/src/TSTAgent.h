/*
 * TSTAgent.h
 *
 *  Created on: 28 Jun 2025
 *      Author: jondurrant
 */

#ifndef EXP_2CORERTOS_SRC_TSTAGENT_H_
#define EXP_2CORERTOS_SRC_TSTAGENT_H_

#include "Agent.h"
#include "pico/stdlib.h"
#include "pico/stdlib.h"
#include <stdio.h>
extern "C"{
#include "tst_variables.h"
}

#include "pico/stdio/driver.h"
#include "pico/stdio.h"
#include "pico/stdio_usb.h"
#include "hardware/uart.h"

class TSTAgent  : public Agent {
public:
	TSTAgent();
	TSTAgent(uart_inst_t * uart);
	virtual ~TSTAgent();

protected:
	/***
	 * Task main run loop
	 */
	virtual void run();

	/***
	 * Get the static depth required in words
	 * @return - words
	 */
	virtual configSTACK_DEPTH_TYPE getMaxStackSize();



private:

	int readData(uint8_t *buf, size_t max);
	void writeData(uint8_t *buf, size_t length);

	uint8_t rxData[TSTMAXSIZE];
	size_t rxSize = 0;
	uint8_t txData[TSTMAXSIZE];
	size_t txSize = 0;

	uart_inst_t * pUart = NULL;
};

#endif /* EXP_2CORERTOS_SRC_TSTAGENT_H_ */
