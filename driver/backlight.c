/* Copyright 2023 Dual Tachyon
 * https://github.com/DualTachyon
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
 */

#include "backlight.h"
#include "bsp/dp32g030/gpio.h"
#include "bsp/dp32g030/pwmplus.h"
#include "bsp/dp32g030/portcon.h"
#include "driver/gpio.h"
#include "settings.h"

// this is decremented once every 500ms
//uint16_t gBacklightCountdown_500ms = 0;
bool backlightOn;

void BACKLIGHT_InitHardware()
{
	// 48MHz / 94 / 1024 ~ 500Hz
	const uint32_t PWM_FREQUENCY_HZ =  1000;
	PWM_PLUS0_CLKSRC |= ((48000000 / 1024 / PWM_FREQUENCY_HZ) << 16);
	PWM_PLUS0_PERIOD = 1023;

	PORTCON_PORTB_SEL0 &= ~(0
		// Back light
		| PORTCON_PORTB_SEL0_B6_MASK
		);
	PORTCON_PORTB_SEL0 |= 0
		// Back light PWM
		| PORTCON_PORTB_SEL0_B6_BITS_PWMP0_CH0
		;

	PWM_PLUS0_GEN = 	
		PWMPLUS_GEN_CH0_OE_BITS_ENABLE |
		PWMPLUS_GEN_CH0_OUTINV_BITS_ENABLE |
		0;

	PWM_PLUS0_CFG =  	
		PWMPLUS_CFG_CNT_REP_BITS_ENABLE |
		PWMPLUS_CFG_COUNTER_EN_BITS_ENABLE |
		0;
}

void BACKLIGHT_TurnOn(void) {
	if (gSettings.backlight == 0) {
		BACKLIGHT_TurnOff();
		return;
	}

	backlightOn = true;
	BACKLIGHT_SetBrightness(gSettings.backlight);
}

uint32_t BACKLIGHT_getTime(void) {
	switch (gSettings.backlightTime) {
		default:
		case 1:	// 5 sec
			return 5 * 1000;
		case 2:	// 10 sec
			return 10 * 1000;
		case 3:	// 20 sec
			return 20 * 1000;
		case 4:	// 1 min
			return 60 * 1000;
		case 5:	// 2 min
			return 120 * 1000;
		case 6:	// 4 min
			return 240 * 1000;
		case 7:	// always on
			return 0;
	}
}

void BACKLIGHT_TurnOff() {
	if ( BACKLIGHT_getTime() == 0 ) return;
	BACKLIGHT_SetBrightness(0);
	backlightOn = false;
}

bool BACKLIGHT_IsOn() {
	return backlightOn;
}

static uint8_t currentBrightness;

void BACKLIGHT_SetBrightness(uint8_t brigtness) {
	currentBrightness = brigtness;
	PWM_PLUS0_CH0_COMP = (uint32_t)(1 << brigtness) - 1;
	//PWM_PLUS0_SWLOAD = 1;
}

uint8_t BACKLIGHT_GetBrightness(void) {
	return currentBrightness;
}