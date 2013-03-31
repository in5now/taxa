/*
 * readers.h
 *
 *  Created on: 02.02.2013
 *      Author: admin
 */

#ifndef M_READERS_H_
#define M_READERS_H_

#include "TCPIP Stack/TCPIP.h"

//#define MIN_WIEGAND_PULSE_LEN	TICK_SECOND/2ul

#define BP_SIZE	5

#define WIEGAND_34_LEN				34
#define WIEGAND_34_HALF_DATA_LEN	16
#define WIEGAND_34_LDATA_START		1
#define WIEGAND_34_HDATA_START		(WIEGAND_34_LEN/2)

typedef enum {
	READER_VOID = 0,
	READER_INPROGRESS,
	READER_READY
} reader_status_e;

//typedef union {
//	QWORD all;
//	struct {
//		QWORD odd:		1;
//		QWORD ldata: 	16;
//		QWORD hdata: 	16;
//		QWORD even:		1;
//	} field;
//} w34_code_t;

void readers_init(void);
void readers_isr(void);
BYTE readers_get_uid(DWORD *uid);
reader_status_e readers_getstatus(void);
void readers_module(void);


#endif /* M_READERS_H_ */