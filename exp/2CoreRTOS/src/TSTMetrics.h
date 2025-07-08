/*
 * TSTMetrics.h
 *
 *  Created on: 8 Jul 2025
 *      Author: jondurrant
 */

#ifndef EXP_FREERTOSMETRICS_SRC_TSTMETRICS_H_
#define EXP_FREERTOSMETRICS_SRC_TSTMETRICS_H_

#include "Agent.h"
#include "pico/stdlib.h"
#include "pico/stdlib.h"
#include <stdio.h>
extern "C"{
#include "tst_variables.h"
}

class TSTMetrics : public Agent{
public:
	TSTMetrics();
	virtual ~TSTMetrics();

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
	char stats_buffer[1024];
};

#endif /* EXP_FREERTOSMETRICS_SRC_TSTMETRICS_H_ */
