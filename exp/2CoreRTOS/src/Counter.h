/*
 * Counter.h
 *
 *  Created on: 17 Jan 2024
 *      Author: jondurrant
 */

#ifndef SRC_COUNTER_H_
#define SRC_COUNTER_H_

#include "pico/stdlib.h"
#include "pico/multicore.h"

#define MAX_ID 4
#define MAX_CORES 2

class Counter {
public:
	virtual ~Counter();

	static Counter * getInstance(uart_inst_t *uart = NULL);

	void setUart(uart_inst_t *uart);

	void start();
	void inc(uint8_t id=0);
	void incCore(uint8_t id=0, uint8_t  core=0);
	void report();

	void getCores(uint32_t &core0, uint32_t &core1);

	void print(const char *s);

protected:
	Counter();
private:

	static Counter *pSingleton;

	uint32_t xStartTime;
	uint32_t xStopTime = 0;
	uint32_t xCounts[MAX_ID];
	uint32_t xCoreCounts[MAX_CORES];

	uart_inst_t * pUart = NULL;

};


#endif /* SRC_COUNTER_H_ */
