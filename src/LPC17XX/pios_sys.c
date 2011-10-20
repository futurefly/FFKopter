/*
 * pios_sys.c
 *
 *  Created on: 20.10.2011
 *      Author: gandhi
 */

#include "pios.h"

#if defined(PIOS_INCLUDE_SYS)

/* Private Function Prototypes */
void NVIC_Configuration(void);
void SysTick_Handler(void);

/* Local Macros */
#define MEM8(addr) (*((volatile uint8_t *)(addr)))
#define MEM16(addr) (*((volatile uint16_t *)(addr)))
#define MEM32(addr) (*((volatile uint32_t *)(addr)))

/* ========= Initialises all system peripherals ==================== */
void PIOS_SYS_Init(void)
{
	/* Setup LPC system (RCC, clock, PLL and Flash conf.) - CMSIS Function */
	SystemInit();

	/* Enable GPIOA, GPIOB, GPIOC, GPIOE and AFIO clocks */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE |
			RCC_APB2Periph_AFIO, ENABLE);

	/* Activate pull-ups on all pins by default */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = 0xffff;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_Init(GPIOD, &GPIO_Initstructure);
#if (PIOS_USB_ENABLED)
	GPIO_InitStructure.GPIO_Pin = 0xffff & ~GPIO_Pin_11 & ~GPIO_Pin_12; /* Exclude USB pins */
#endif
	GPIO_Init(GPIOA, &GPIO_InitStructure);

#if (PIOS_USB_ENABLED)
	/* Ensure that pull-up is active on detect pin */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = PIOS_USB_DETECT_GPIO_PIN;
	GPIO_Init(PIOS_USB_DETECT_GPIO_PORT, &GPIO_InitStructure);
#endif


}
