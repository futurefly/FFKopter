/*
 * pios_lpc17xx.h
 *
 *  Created on: 20.10.2011
 *      Author: gandhi
 */

#ifndef PIOS_LPC17XX_H_
#define PIOS_LPC17XX_H_

struct lpc17xx_irq {
	uint32_t flags;
	NVIC_InitTypeDef init;
};

struct lpc17xx_dma_chan {
	DMA_Channel_TypeDef *channel;
	DMA_InitTypeDef init;
};

struct lpc17xx_dma {
	uint32_t ahb_clk;
	struct lpc17xx_irq irq;
	struct lpc17xx_dma_chan rx;
	struct lpc17xx_dma_chan tx;
};

struct lpc17xx_gpio {
	GPIO_TypeDef *gpio;
	GPIO_InitTypeDef init;
};

#endif /* PIOS_LPC17XX_H_ */
