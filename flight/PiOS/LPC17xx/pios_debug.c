/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @defgroup   PIOS_DEBUG Debugging Functions
 * @brief Debugging functionality
 * @{
 *
 * @file       pios_debug.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Debugging Functions
 * @see        The GNU Public License (GPL) Version 3
 * 
 *****************************************************************************/
/* 
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 3 of the License, or 
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License 
 * for more details.
 * 
 * You should have received a copy of the GNU General Public License along 
 * with this program; if not, write to the Free Software Foundation, Inc., 
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* Project Includes */
#include "pios.h"

// Global variables
const char *PIOS_DEBUG_AssertMsg = "ASSERT FAILED";

#include <pios_servo_priv.h>
extern const struct pios_servo_channel pios_servo_channels[];
#define PIOS_SERVO_GPIO_PORT_1TO4 pios_servo_channels[0].port
#define PIOS_SERVO_GPIO_PORT_5TO8 pios_servo_channels[4].port
#define PIOS_SERVO_GPIO_PIN_1  pios_servo_channels[0].pin
#define PIOS_SERVO_GPIO_PIN_2  pios_servo_channels[1].pin
#define PIOS_SERVO_GPIO_PIN_3  pios_servo_channels[2].pin
#define PIOS_SERVO_GPIO_PIN_4  pios_servo_channels[3].pin
#define PIOS_SERVO_GPIO_PIN_5  pios_servo_channels[4].pin
#define PIOS_SERVO_GPIO_PIN_6  pios_servo_channels[5].pin
#define PIOS_SERVO_GPIO_PIN_7  pios_servo_channels[6].pin
#define PIOS_SERVO_GPIO_PIN_8  pios_servo_channels[7].pin
/* Private Function Prototypes */

/**
* Initialise Debug-features
*/
void PIOS_DEBUG_Init(void)
{
#ifdef PIOS_ENABLE_DEBUG_PINS
	// Initialise Servo pins as standard output pins
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = PIOS_SERVO_GPIO_PIN_1 | PIOS_SERVO_GPIO_PIN_2 | PIOS_SERVO_GPIO_PIN_3 | PIOS_SERVO_GPIO_PIN_4;
	GPIO_Init(PIOS_SERVO_GPIO_PORT_1TO4, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = PIOS_SERVO_GPIO_PIN_5 | PIOS_SERVO_GPIO_PIN_6 | PIOS_SERVO_GPIO_PIN_7 | PIOS_SERVO_GPIO_PIN_8;
	GPIO_Init(PIOS_SERVO_GPIO_PORT_5TO8, &GPIO_InitStructure);

	// Drive all pins low
	PIOS_SERVO_GPIO_PORT_1TO4->BRR = PIOS_SERVO_GPIO_PIN_1 | PIOS_SERVO_GPIO_PIN_2 | PIOS_SERVO_GPIO_PIN_3 | PIOS_SERVO_GPIO_PIN_4;
	PIOS_SERVO_GPIO_PORT_5TO8->BRR = PIOS_SERVO_GPIO_PIN_5 | PIOS_SERVO_GPIO_PIN_6 | PIOS_SERVO_GPIO_PIN_7 | PIOS_SERVO_GPIO_PIN_8;
#endif // PIOS_ENABLE_DEBUG_PINS
}

/**
* Set debug-pin high
* \param pin 0 for S1 output
*/
void PIOS_DEBUG_PinHigh(uint8_t Pin)
{
#ifdef PIOS_ENABLE_DEBUG_PINS
	if (Pin < 4) {
		PIOS_SERVO_GPIO_PORT_1TO4->BSRR = (PIOS_SERVO_GPIO_PIN_1 << Pin);
	} else if (Pin <= 7) {
		PIOS_SERVO_GPIO_PORT_5TO8->BSRR = (PIOS_SERVO_GPIO_PIN_5 << (Pin - 4));
	}
#endif // PIOS_ENABLE_DEBUG_PINS
}

/**
* Set debug-pin low
* \param pin 0 for S1 output
*/
void PIOS_DEBUG_PinLow(uint8_t Pin)
{
#ifdef PIOS_ENABLE_DEBUG_PINS
	if (Pin < 4) {
		PIOS_SERVO_GPIO_PORT_1TO4->BRR = (PIOS_SERVO_GPIO_PIN_1 << Pin);
	} else if (Pin <= 7) {
		PIOS_SERVO_GPIO_PORT_5TO8->BRR = (PIOS_SERVO_GPIO_PIN_5 << (Pin - 4));
	}
#endif // PIOS_ENABLE_DEBUG_PINS
}


void PIOS_DEBUG_PinValue8Bit(uint8_t value)
{
#ifdef PIOS_ENABLE_DEBUG_PINS
	uint32_t bsrr_l = ( ((~value)&0x0F)<<(16+6)   ) | ((value & 0x0F)<<6);
	uint32_t bsrr_h = ( ((~value)&0xF0)<<(16+6-4) ) | ((value & 0xF0)<<(6-4));
	PIOS_IRQ_Disable();
	PIOS_SERVO_GPIO_PORT_1TO4->BSRR = bsrr_l;
	PIOS_SERVO_GPIO_PORT_5TO8->BSRR = bsrr_h;
	PIOS_IRQ_Enable();
#endif // PIOS_ENABLE_DEBUG_PINS
}

void PIOS_DEBUG_PinValue4BitL(uint8_t value)
{
#ifdef PIOS_ENABLE_DEBUG_PINS
	uint32_t bsrr_l = ((~(value & 0x0F)<<(16+6))) | ((value & 0x0F)<<6);
	PIOS_SERVO_GPIO_PORT_1TO4->BSRR = bsrr_l;
#endif // PIOS_ENABLE_DEBUG_PINS
}


/**
 * Report a serious error and halt
 */
void PIOS_DEBUG_Panic(const char *msg)
{
#ifdef PIOS_COM_DEBUG
	register int *lr asm("lr");	// Link-register holds the PC of the caller
	PIOS_COM_SendFormattedStringNonBlocking(PIOS_COM_DEBUG, "\r%s @0x%x\r", msg, lr);
#endif

	// Stay put
	while (1) ;
}

/**
  * @}
  * @}
  */
