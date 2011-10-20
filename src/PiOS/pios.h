/*
 * pios.h
 *
 *  Created on: 19.10.2011
 *      Author: gandhi
 */

#ifndef PIOS_H_
#define PIOS_H_

/* PIOS feature */
#include "pios_config.h"

#if defined(PIOS_INCLUDE_FREERTOS)
/* FreeRTOS Includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#endif

/* C Lib Includes */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

/* LPCXPresso Lib */
#if defined (_USE_CMSIS)
#include "LPC17xx.h"
#endif
#include <LPC17xx_conf.h>

#include "pios_initcall.h"




#define NELEMENTS(x) (sizeof(x)/sizeof(*(x)))

#endif /* PIOS_H_ */
