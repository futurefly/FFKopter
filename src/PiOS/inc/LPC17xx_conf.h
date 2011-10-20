/*
 * LPC17xx_conf.h
 *
 *  Created on: 19.10.2011
 *      Author: gandhi
 */

#ifndef LPC17XX_CONF_H_
#define LPC17XX_CONF_H_

/* Includes ---------------------------------------------------------*/


/* ------------------------------------------------------------------*/

#ifdef USE_FULL_ASSERT

#define assert_param(expr) ((expr) ? (void)0 : assert_failed((uint8_t *)__FILE__, __LINE__))

extern void assert_failed(uint8_t * file, uint32_t line);
#else
#define assert_param(expr) ((void)0)
#endif

#endif /* LPC17XX_CONF_H_ */
