/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_PPM PPM Input Functions
 * @brief Code to measure PPM input and seperate into channels
 * @{
 *
 * @file       pios_ppm.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      PPM Input functions (STM32 dependent)
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
#include "pios_ppm_priv.h"

#if defined(PIOS_INCLUDE_PPM)

/* Provide a RCVR driver */
static int32_t PIOS_PPM_Get(uint32_t rcvr_id, uint8_t channel);

const struct pios_rcvr_driver pios_ppm_rcvr_driver = {
	.read = PIOS_PPM_Get,
};

#define PIOS_PPM_IN_MIN_NUM_CHANNELS		4
#define PIOS_PPM_IN_MAX_NUM_CHANNELS		PIOS_PPM_NUM_INPUTS
#define PIOS_PPM_STABLE_CHANNEL_COUNT		25	// frames
#define PIOS_PPM_IN_MIN_SYNC_PULSE_US		3800	// microseconds
#define PIOS_PPM_IN_MIN_CHANNEL_PULSE_US	750	// microseconds
#define PIOS_PPM_IN_MAX_CHANNEL_PULSE_US	2250   // microseconds
#define PIOS_PPM_INPUT_INVALID			0

/* Local Variables */
static TIM_ICInitTypeDef TIM_ICInitStructure;
static uint8_t PulseIndex;
static uint32_t PreviousTime;
static uint32_t CurrentTime;
static uint32_t DeltaTime;
static uint32_t CaptureValue[PIOS_PPM_IN_MAX_NUM_CHANNELS];
static uint32_t CaptureValueNewFrame[PIOS_PPM_IN_MAX_NUM_CHANNELS];
static uint32_t LargeCounter;
static int8_t NumChannels;
static int8_t NumChannelsPrevFrame;
static uint8_t NumChannelCounter;

static uint8_t supv_timer = 0;
static bool Tracking;
static bool Fresh;

static void PIOS_PPM_Supervisor(uint32_t ppm_id);

void PIOS_PPM_Init(void)
{
	/* Flush counter variables */
	int32_t i;

	PulseIndex = 0;
	PreviousTime = 0;
	CurrentTime = 0;
	DeltaTime = 0;
	LargeCounter = 0;
	NumChannels = -1;
	NumChannelsPrevFrame = -1;
	NumChannelCounter = 0;
	Tracking = FALSE;
	Fresh = FALSE;

	for (i = 0; i < PIOS_PPM_IN_MAX_NUM_CHANNELS; i++) {
		CaptureValue[i] = 0;
		CaptureValueNewFrame[i] = 0;
	}

	NVIC_InitTypeDef NVIC_InitStructure = pios_ppm_cfg.irq.init;

	/* Enable appropriate clock to timer module */
	switch((int32_t) pios_ppm_cfg.timer) {
		case (int32_t)TIM1:
			NVIC_InitStructure.NVIC_IRQChannel = TIM1_CC_IRQn;
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
			break;
		case (int32_t)TIM2:
			NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
			break;
		case (int32_t)TIM3:
			NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
			break;
		case (int32_t)TIM4:
			NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
			break;
#ifdef STM32F10X_HD
		case (int32_t)TIM5:
			NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
			break;
		case (int32_t)TIM6:
			NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn;
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
			break;
		case (int32_t)TIM7:
			NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
			break;
		case (int32_t)TIM8:
			NVIC_InitStructure.NVIC_IRQChannel = TIM8_CC_IRQn;
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);
			break;
#endif
	}
	/* Enable timer interrupts */
	NVIC_Init(&NVIC_InitStructure);

	/* Configure input pins */
	GPIO_InitTypeDef GPIO_InitStructure = pios_ppm_cfg.gpio_init;
	GPIO_Init(pios_ppm_cfg.port, &GPIO_InitStructure);

	/* Configure timer for input capture */
	TIM_ICInitStructure = pios_ppm_cfg.tim_ic_init;
	TIM_ICInit(pios_ppm_cfg.timer, &TIM_ICInitStructure);

	/* Configure timer clocks */
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure = pios_ppm_cfg.tim_base_init;
	TIM_InternalClockConfig(pios_ppm_cfg.timer);
	TIM_TimeBaseInit(pios_ppm_cfg.timer, &TIM_TimeBaseStructure);

	/* Enable the Capture Compare Interrupt Request */
	TIM_ITConfig(pios_ppm_cfg.timer, pios_ppm_cfg.ccr | TIM_IT_Update, ENABLE);

	/* Enable timers */
	TIM_Cmd(pios_ppm_cfg.timer, ENABLE);

	/* Setup local variable which stays in this scope */
	/* Doing this here and using a local variable saves doing it in the ISR */
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter = 0x0;

	if (!PIOS_RTC_RegisterTickCallback(PIOS_PPM_Supervisor, 0)) {
		PIOS_DEBUG_Assert(0);
	}
}

/**
* Get the value of an input channel
* \param[in] Channel Number of the channel desired
* \output -1 Channel not available
* \output >0 Channel value
*/
static int32_t PIOS_PPM_Get(uint32_t rcvr_id, uint8_t channel)
{
	/* Return error if channel not available */
	if (channel >= PIOS_PPM_IN_MAX_NUM_CHANNELS) {
		return -1;
	}
	return CaptureValue[channel];
}

/**
* Handle TIMx global interrupt request
* Some work and testing still needed, need to detect start of frame and decode pulses
*
*/
void PIOS_PPM_irq_handler(void)
{
	/* Timer Overflow Interrupt
	 * The time between timer overflows must be greater than the PPM
	 * frame period. If a full frame has not decoded in the between timer
	 * overflows then capture values should be cleared.
	 */

	if (TIM_GetITStatus(pios_ppm_cfg.timer, TIM_IT_Update) == SET) {
		/* Clear TIMx overflow interrupt pending bit */
		TIM_ClearITPendingBit(pios_ppm_cfg.timer, TIM_IT_Update);

		/* If sharing a timer with a servo output the ARR register will
		   be set according to the PWM period. When timer reaches the
		   ARR value a timer overflow interrupt will fire. We use the
		   interrupt accumulate a 32-bit timer. */
		LargeCounter = LargeCounter + pios_ppm_cfg.timer->ARR;
	}

	/* Signal edge interrupt */
	if (TIM_GetITStatus(pios_ppm_cfg.timer, pios_ppm_cfg.ccr) == SET) {
		PreviousTime = CurrentTime;

		switch((int32_t) pios_ppm_cfg.ccr) {
			case (int32_t)TIM_IT_CC1:
				CurrentTime = TIM_GetCapture1(pios_ppm_cfg.timer);
				break;
			case (int32_t)TIM_IT_CC2:
				CurrentTime = TIM_GetCapture2(pios_ppm_cfg.timer);
				break;
			case (int32_t)TIM_IT_CC3:
				CurrentTime = TIM_GetCapture3(pios_ppm_cfg.timer);
				break;
			case (int32_t)TIM_IT_CC4:
				CurrentTime = TIM_GetCapture4(pios_ppm_cfg.timer);
				break;
		}

		/* Clear TIMx Capture compare interrupt pending bit */
		TIM_ClearITPendingBit(pios_ppm_cfg.timer, pios_ppm_cfg.ccr);

		/* Convert to 32-bit timer result */
		CurrentTime = CurrentTime + LargeCounter;

		/* Capture computation */		
		DeltaTime = CurrentTime - PreviousTime;

		PreviousTime = CurrentTime;

		/* Sync pulse detection */
		if (DeltaTime > PIOS_PPM_IN_MIN_SYNC_PULSE_US) {
			if (PulseIndex == NumChannelsPrevFrame
			 && PulseIndex >= PIOS_PPM_IN_MIN_NUM_CHANNELS
			 && PulseIndex <= PIOS_PPM_IN_MAX_NUM_CHANNELS)
			{
				/* If we see n simultaneous frames of the same
				 number of channels we save it as our frame size */
				if (NumChannelCounter < PIOS_PPM_STABLE_CHANNEL_COUNT)
					NumChannelCounter++;
				else
					NumChannels = PulseIndex;
			} else {
				NumChannelCounter = 0;
			}

			/* Check if the last frame was well formed */
			if (PulseIndex == NumChannels && Tracking) {
				/* The last frame was well formed */
				for (uint32_t i = 0; i < NumChannels; i++) {
					CaptureValue[i] = CaptureValueNewFrame[i];
				}
				for (uint32_t i = NumChannels;
				     i < PIOS_PPM_IN_MAX_NUM_CHANNELS; i++) {
					CaptureValue[i] = PIOS_PPM_INPUT_INVALID;
				}
			}

			Fresh = TRUE;
			Tracking = TRUE;
			NumChannelsPrevFrame = PulseIndex;
			PulseIndex = 0;

			/* We rely on the supervisor to set CaptureValue to invalid
			 if no valid frame is found otherwise we ride over it */

		} else if (Tracking) {
			/* Valid pulse duration 0.75 to 2.5 ms*/
			if (DeltaTime > PIOS_PPM_IN_MIN_CHANNEL_PULSE_US
			    && DeltaTime < PIOS_PPM_IN_MAX_CHANNEL_PULSE_US
			    && PulseIndex < PIOS_PPM_IN_MAX_NUM_CHANNELS) {

				CaptureValueNewFrame[PulseIndex] = DeltaTime;
				PulseIndex++;
			} else {
				/* Not a valid pulse duration */
				Tracking = FALSE;
				for (uint32_t i = 0; i < PIOS_PPM_IN_MAX_NUM_CHANNELS ; i++) {
					CaptureValueNewFrame[i] = PIOS_PPM_INPUT_INVALID;
				}
			}
		}
	}
}

static void PIOS_PPM_Supervisor(uint32_t ppm_id) {
	/* 
	 * RTC runs at 625Hz so divide down the base rate so
	 * that this loop runs at 25Hz.
	 */
	if(++supv_timer < 25) {
		return;
	}
	supv_timer = 0;

	if (!Fresh) {
		Tracking = FALSE;

		for (int32_t i = 0; i < PIOS_PPM_IN_MAX_NUM_CHANNELS ; i++) {
			CaptureValue[i] = PIOS_PPM_INPUT_INVALID;
			CaptureValueNewFrame[i] = PIOS_PPM_INPUT_INVALID;
		}
	}

	Fresh = FALSE;
}

#endif

/**
  * @}
  * @}
  */
