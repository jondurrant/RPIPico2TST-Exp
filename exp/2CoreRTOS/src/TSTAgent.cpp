/*
 * TSTAgent.cpp
 *
 *  Created on: 28 Jun 2025
 *      Author: jondurrant
 */

#include "TSTAgent.h"
#include "pico/stdlib.h"

#define DEBUG_LINE 15


TSTAgent::TSTAgent(uart_inst_t * uart) {
	pUart = uart;

}

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

	size_t read = 0;
	tstInit(&TST_Device);

	for (;;){

		read = readData( rxData,  TSTMAXSIZE);
		if (read > 0) {
			//debugPrintBuffer( "Read",   rxData,  read);
			uint8_t err = tstRx(TST_Device.name, TST_Interface.interface, rxData, read);
			if (err != TST_OK){
				char errTxt[10];
				sprintf(errTxt,"Error: %u", err);
				tstMonitorSend(TST_Device.name, TST_Interface.interface, errTxt);
			}
		}

		//tstMonitorSend(TST_Device.name, TST_Interface.interface, "TST Device alive");
		if (tstTx(TST_Device.name, TST_Interface.interface, txData, &txSize) == TST_OK && txSize > 0) {
			writeData( txData, txSize);
		}

		vTaskDelay(pdMS_TO_TICKS(10));

	}
}


size_t TSTAgent::readData(uint8_t *buf, size_t max){
	size_t res = 0;
	if (pUart == NULL){
		while (res < max) {
			int c = getchar_timeout_us(0);
			if (c == PICO_ERROR_TIMEOUT || c < 0) break;
			buf[res++] = (uint8_t)c;
		}
	} else {
		for (int i = 0; i < 5; i++ ){
			while (uart_is_readable ( pUart)){
				buf[res] = uart_getc ( pUart);
				res++;
				if (res >= max){
					break;
				}

			}
			vTaskDelay(pdMS_TO_TICKS(10));
		}
	}
	return res;
}

void TSTAgent::writeData(uint8_t *buf, size_t length){
	if (pUart == NULL){
		 fwrite(buf, 1, length, stdout);
		 fflush(stdout);
	} else {
		uart_write_blocking (pUart,  buf,  length);
	}
}



/***
 * Get the static depth required in words
 * @return - words
 */
configSTACK_DEPTH_TYPE TSTAgent::getMaxStackSize(){
	return 256;
}


void TSTAgent::debugPrintBuffer(const char *title, const void * pBuffer, size_t bytes){
	size_t count =0;
	size_t lineEnd=0;
	const uint8_t *pBuf = (uint8_t *)pBuffer;

	printf("DEBUG: %s of size %d\n", title, bytes);

	while (count < bytes){
		lineEnd = count + DEBUG_LINE;
		if (lineEnd > bytes){
			lineEnd = bytes;
		}

		//Print HEX DUMP
		for (size_t i=count; i < lineEnd; i++){
			if (pBuf[i] <= 0x0F){
				printf("0%X ", pBuf[i]);
			} else {
				printf("%X ", pBuf[i]);
			}
		}

		//Pad for short lines
		size_t pad = (DEBUG_LINE - (lineEnd - count)) * 3;
		for (size_t i=0; i < pad; i++){
			printf(" ");
		}

		//Print Plain Text
		for (size_t i=count; i < lineEnd; i++){
			if ((pBuf[i] >= 0x20) && (pBuf[i] <= 0x7e)){
				printf("%c", pBuf[i]);
			} else {
				printf(".");
			}
		}

		printf("\n");

		count = lineEnd;

	}
}
