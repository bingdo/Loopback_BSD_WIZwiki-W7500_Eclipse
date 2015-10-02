#ifndef _LOOPBACK_H_
#define _LOOPBACK_H_

#include <stdint.h>

/* Loopback test debug message printout enable */
#define	_LOOPBACK_DEBUG_
#define	TEST_TCPS // one of TEST_TCPS, TEST_TCPC

/* DATA_BUF_SIZE define for Loopback example */
#ifndef DATA_BUF_SIZE
	#define DATA_BUF_SIZE			2048
#endif

/************************/
/* Select LOOPBACK_MODE */
/************************/
#define LOOPBACK_MAIN_NOBLOCK	0
#define LOOPBACK_MAIN_SAMPLE	1
#define LOOPBACK_MODE   LOOPBACK_MAIN_SAMPLE // one of LOOPBACK_MAIN_NOBLOCK, LOOPBACK_MAIN_SAMPLE

#endif
