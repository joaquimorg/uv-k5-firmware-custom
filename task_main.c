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

#include <stdint.h>
#include <string.h>
#include <stdio.h>     // NULL

#include "debugging.h"

#ifdef ENABLE_MESSENGER
	#include "app/messenger.h"
#endif

#ifdef ENABLE_AM_FIX
	#include "am_fix.h"
#endif

#include "audio.h"
#include "board.h"
#include "misc.h"
#include "radio.h"
#include "app.h"
#include "settings.h"
#include "version.h"

#include "app/app.h"
#include "app/dtmf.h"
#include "bsp/dp32g030/gpio.h"
#include "bsp/dp32g030/syscon.h"

#include "driver/backlight.h"
#include "driver/bk4819.h"
#include "driver/st7565.h"
#include "driver/gpio.h"
#include "driver/system.h"
#include "driver/systick.h"
#ifdef ENABLE_UART
	#include "driver/uart.h"
	#include "app/uart.h"
	#include "ARMCM0.h"
#endif

#include "helper/battery.h"
#include "helper/boot.h"

#include "ui/status.h"
#include "ui/ui.h"

#include "ui/lock.h"
#include "ui/welcome.h"
#include "ui/menu.h"

#include "task_main.h"
#include "vfo.h"
#include "common.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

#include "task_messages.h"
#include "applications_task.h"

StackType_t main_task_stack[configMINIMAL_STACK_SIZE + 100];
StaticTask_t main_task_buffer;

TimerHandle_t hwStatusTimer;
StaticTimer_t hwStatusTimerBuffer;

TimerHandle_t hwStatusTimer500;
StaticTimer_t hwStatusTimerBuffer500;

#define QUEUE_LENGTH    20
#define ITEM_SIZE       sizeof( MAIN_Messages_t )
static StaticQueue_t mainTasksQueue;
QueueHandle_t mainTasksMsgQueue;
uint8_t mainQueueStorageArea[ QUEUE_LENGTH * ITEM_SIZE ];

extern void SystickHandlerA(void);

void main_push_message(MAIN_MSG_t msg);

/* --------------------------------------------------------------------------------------------------------- */

void init_radio(void) {

	BK4819_Init();

	BOARD_ADC_GetBatteryInfo(&gBatteryCurrentVoltage, &gBatteryCurrent);

	SETTINGS_InitEEPROM();

	ST7565_SetContrast(gEeprom.LCD_CONTRAST);

	//SETTINGS_WriteBuildOptions();
	SETTINGS_LoadCalibration();

	RADIO_ConfigureChannel(0, VFO_CONFIGURE_RELOAD);
	RADIO_ConfigureChannel(1, VFO_CONFIGURE_RELOAD);

	RADIO_SelectVfos();

	RADIO_SetupRegisters(true);

	for (unsigned int i = 0; i < ARRAY_SIZE(gBatteryVoltages); i++)
		BOARD_ADC_GetBatteryInfo(&gBatteryVoltages[i], &gBatteryCurrent);

	BATTERY_GetReadings(false);

#ifdef ENABLE_MESSENGER
	MSG_Init();
#endif

#ifdef ENABLE_AM_FIX
	AM_fix_init();
#endif


	/*// count the number of menu items
	gMenuListCount = 0;
	while (MenuList[gMenuListCount].name[0] != '\0') {
		if(!gF_LOCK && MenuList[gMenuListCount].menu_id == FIRST_HIDDEN_MENU_ITEM)
			break;

		gMenuListCount++;
	}*/

	GPIO_ClearBit(&GPIOA->DATA, GPIOA_PIN_VOICE_0);

	gUpdateStatus = true;

}


/* --------------------------------------------------------------------------------------------------------- */


void HandlerGPIOB1(void) {
	LogUartf("HandlerGPIOB IRQ %b \r\n", GPIO_CheckBit(&GPIOB->DATA, GPIOB_PIN_SWD_CLK));
}

void hw_timer_callback(TimerHandle_t xTimer) {

#ifdef ENABLE_UART
	//taskENTER_CRITICAL();
	if (UART_IsCommandAvailable()) {
		UART_HandleCommand();
	}
	//taskEXIT_CRITICAL();
#endif

	if (GPIO_CheckBit(&GPIOB->DATA, GPIOB_PIN_SWD_CLK)) {
		CheckRadioInterrupts();
	}
	//SystickHandlerA();

    xTimerStart(xTimer, 0);
}

//bool flippp = false;

void hw_timer_callback_500(TimerHandle_t xTimer) {

	//BK4819_ToggleGpioOut(2, flippp);
	//LogUartf("GPIOB->DATA %b \r\n", GPIO_CheckBit(&GPIOB->DATA, GPIOB_PIN_SWD_CLK));
	//flippp = !flippp;

	//LogUartf("500ms \r\n");

	//APP_TimeSlice500ms();
    xTimerStart(xTimer, 0);
}


void main_task(void* arg) {
	(void)arg;

	BOARD_Init();

#ifdef ENABLE_UART
	UART_Init();
	UART_Send(UART_Version, strlen(UART_Version));
#endif

	init_radio();

	mainTasksMsgQueue = xQueueCreateStatic(QUEUE_LENGTH, ITEM_SIZE, mainQueueStorageArea, &mainTasksQueue);

	main_push_message(MAIN_MSG_INIT);

	hwStatusTimer = xTimerCreateStatic("hwStatus", pdMS_TO_TICKS(20), pdFALSE, NULL, hw_timer_callback, &hwStatusTimerBuffer);
	//hwStatusTimer500 = xTimerCreateStatic("hwStatus500", pdMS_TO_TICKS(500), pdFALSE, NULL, hw_timer_callback_500, &hwStatusTimerBuffer500);

	BACKLIGHT_TurnOn();

	applications_task_init();

	//xTimerStart(hwStatusTimer500, 0);
	xTimerStart(hwStatusTimer, 0);
	
	LogUartf("Main Task Ready... \r\n");

	while (true) {
		MAIN_Messages_t msg;
    	if (xQueueReceive(mainTasksMsgQueue, &msg, 10)) {
			switch(msg.message) {
				case MAIN_MSG_INIT:
					//LogUartf("MSG INIT \r\n");
					break;
				case MAIN_MSG_IDLE:
					break;

				case MAIN_MSG_BKLIGHT_ON:
					BACKLIGHT_TurnOn();
					break;

				case MAIN_MSG_BKLIGHT_OFF:
					BACKLIGHT_TurnOff();
					break;

				case MAIN_MSG_PLAY_BEEP:
					if ( msg.payload != 0 ) {
						AUDIO_PlayBeep(msg.payload);
					}
					break;

				/* -------------------------------------------------------- */

				case SET_VFO_STATE_NORMAL:
					RADIO_SetVfoState(VFO_STATE_NORMAL);
					break;
				case RADIO_TX:
					//FUNCTION_Select(FUNCTION_TRANSMIT);
					RADIO_PrepareTX();
					break;

				case RADIO_RX:
					APP_EndTransmission(true);
					break;

				case RADIO_SQUELCH_LOST:
					gCurrentFunction = FUNCTION_INCOMING;
					//APP_Function(gCurrentFunction);
					if (gCurrentCodeType == CODE_TYPE_OFF) {
						APP_StartListening(gMonitor ? FUNCTION_MONITOR : FUNCTION_RECEIVE);
					}
					app_push_message(APP_MSG_RX);
					//LogUartf("SQUELCH_LOST\r\n");
                    break;

				case RADIO_SQUELCH_FOUND:
					gCurrentFunction = FUNCTION_RECEIVE;
					APP_Function(gCurrentFunction);
					app_push_message(APP_MSG_IDLE);
					//LogUartf("SQUELCH_FOUND\r\n");
                    break;

				case RADIO_CTCSS_LOST:
					//LogUartf("CTCSS_LOST (%b) CT = (%i)\r\n", g_CTCSS_Lost, gCurrentCodeType);
					APP_StartListening(gMonitor ? FUNCTION_MONITOR : FUNCTION_RECEIVE);
					break;

				case RADIO_CTCSS_FOUND:
					//LogUartf("CTCSS_FOUND (%b) CT = (%i)\r\n", g_CTCSS_Lost, gCurrentCodeType);
					break;

				case RADIO_CDCSS_LOST:
					//LogUartf("CDCSS_LOST (%b) CD = (%i)\r\n", g_CDCSS_Lost, gCurrentCodeType);
					APP_StartListening(gMonitor ? FUNCTION_MONITOR : FUNCTION_RECEIVE);
					break;

				case RADIO_CDCSS_FOUND:
					//LogUartf("CDCSS_FOUND (%b) CD = (%i)\r\n", g_CDCSS_Lost, gCurrentCodeType);
					break;

				case RADIO_VFO_UP:
					VFO_Up_Down(1);
					break;

				case RADIO_VFO_DOWN:
					VFO_Up_Down(-1);
					break;

				case RADIO_VFO_SWITCH:
					COMMON_SwitchVFOs();
					main_push_message(RADIO_SAVE_VFO);
					main_push_message(RADIO_RECONFIGURE_VFO);
					break;

				case RADIO_VFO_SWITCH_MODE:
                	COMMON_SwitchVFOMode();
					main_push_message(RADIO_SAVE_VFO);
					main_push_message(RADIO_VFO_CONFIGURE_CHANNEL);
					main_push_message(RADIO_RECONFIGURE_VFO);
					break;

				case RADIO_SAVE_VFO:
					SETTINGS_SaveVfoIndices();
                    break;

				case RADIO_VFO_CONFIGURE_RELOAD:
                    RADIO_ConfigureChannel(0, VFO_CONFIGURE_RELOAD);
	                RADIO_ConfigureChannel(1, VFO_CONFIGURE_RELOAD);
                    break;

				case RADIO_RECONFIGURE_VFO:
                    RADIO_SelectVfos();
                    RADIO_SetupRegisters(true);
                    break;

                case RADIO_VFO_CONFIGURE:
					RADIO_ConfigureChannel(gEeprom.TX_VFO, VFO_CONFIGURE);
                    break;

				case RADIO_VFO_CONFIGURE_CHANNEL:
					RADIO_ConfigureChannel(gEeprom.TX_VFO, VFO_CONFIGURE_RELOAD);
                    break;

				case RADIO_SAVE_CHANNEL:
					SETTINGS_SaveChannel(gTxVfo->CHANNEL_SAVE, gEeprom.TX_VFO, gTxVfo, 1);
					break;

				case RADIO_SAVE_SETTINGS:
					SETTINGS_SaveSettings();
                    break;

				case RADIO_SET_CHANNEL:
					if ( msg.payload != 0 ) {
						if (!RADIO_CheckValidChannel((uint16_t)msg.payload, false, 0)) {
				            main_push_message_value(MAIN_MSG_PLAY_BEEP, BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL);				            
			            } else {
							gEeprom.MrChannel[gEeprom.TX_VFO]     = (uint8_t)msg.payload;
							gEeprom.ScreenChannel[gEeprom.TX_VFO] = (uint8_t)msg.payload;
							main_push_message(RADIO_SAVE_VFO);
							main_push_message(RADIO_VFO_CONFIGURE_RELOAD);
							main_push_message(RADIO_RECONFIGURE_VFO);
						}
					}
					break;

				case RADIO_SET_FREQ:
					if ( msg.payload != 0 ) {
						uint32_t frequency = msg.payload;
						// clamp the frequency entered to some valid value
                        if (frequency < frequencyBandTable[0].lower) {
                            frequency = frequencyBandTable[0].lower;
                        }
                        else if (frequency >= BX4819_band1.upper && frequency < BX4819_band2.lower) {
                            const uint32_t center = (BX4819_band1.upper + BX4819_band2.lower) / 2;
                            frequency = (frequency < center) ? BX4819_band1.upper : BX4819_band2.lower;
                        }
                        else if (frequency > frequencyBandTable[BAND_N_ELEM - 1].upper) {
                            frequency = frequencyBandTable[BAND_N_ELEM - 1].upper;
                        }

                        const FREQUENCY_Band_t band = FREQUENCY_GetBand(frequency);

                        if (gTxVfo->Band != band) {
                            gTxVfo->Band               				= band;
                            gEeprom.ScreenChannel[gEeprom.TX_VFO] 	= band + FREQ_CHANNEL_FIRST;
                            gEeprom.FreqChannel[gEeprom.TX_VFO]   	= band + FREQ_CHANNEL_FIRST;

                            main_push_message(RADIO_SAVE_VFO);
                            main_push_message(RADIO_VFO_CONFIGURE_CHANNEL);
                        }

                        frequency = FREQUENCY_RoundToStep(frequency, gTxVfo->StepFrequency);

                        if (frequency >= BX4819_band1.upper && frequency < BX4819_band2.lower)
                        {	// clamp the frequency to the limit
                            const uint32_t center = (BX4819_band1.upper + BX4819_band2.lower) / 2;
                            frequency = (frequency < center) ? BX4819_band1.upper - gTxVfo->StepFrequency : BX4819_band2.lower;
                        }
                        gTxVfo->freq_config_RX.Frequency = frequency;

                        main_push_message(RADIO_SAVE_CHANNEL);
					}
					break;

			}
		}

	}
}


void main_push_message_value(MAIN_MSG_t msg, uint32_t value) {
	MAIN_Messages_t mainMSG = { msg, value };
    BaseType_t xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendToBackFromISR(mainTasksMsgQueue, (void *)&mainMSG, &xHigherPriorityTaskWoken);
}

void main_push_message(MAIN_MSG_t msg) {
	main_push_message_value(msg, 0);
}

void main_task_init(void) {

    xTaskCreateStatic(
		main_task,
		"MAIN",
		ARRAY_SIZE(main_task_stack),
		NULL,
		1 + tskIDLE_PRIORITY,
		main_task_stack,
		&main_task_buffer
	);

}
