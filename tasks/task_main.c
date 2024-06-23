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

#include "audio.h"
#include "board.h"
#include "misc.h"
#include "radio.h"
#include "app.h"
#include "settings.h"
#include "version.h"

#include "app/app.h"
#include "app/dtmf.h"
#include "bsp/dp32g030/irq.h"
#include "bsp/dp32g030/gpio.h"
#include "bsp/dp32g030/syscon.h"

#include "driver/backlight.h"
#include "driver/bk4819.h"
#include "driver/st7565.h"
#include "driver/gpio.h"
#include "driver/system.h"
#include "driver/systick.h"

#include "bsp/dp32g030/uart.h"
#include "driver/uart.h"	
#include "app/uart.h"
#include "ARMCM0.h"

#include "ui/status.h"

#include "task_main.h"
#include "vfo.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

#include "task_messages.h"
#include "applications_task.h"

#include "frequencies.h"

StackType_t main_task_stack[configMINIMAL_STACK_SIZE + 100];
StaticTask_t main_task_buffer;

//TimerHandle_t radioStatusTimer;
//StaticTimer_t radioStatusTimerBuffer;

#define QUEUE_LENGTH    20
#define ITEM_SIZE       sizeof( MAIN_Messages_t )
static StaticQueue_t mainTasksQueue;
QueueHandle_t mainTasksMsgQueue;
uint8_t mainQueueStorageArea[ QUEUE_LENGTH * ITEM_SIZE ];

void main_push_message(MAIN_MSG_t msg);

/* --------------------------------------------------------------------------------------------------------- */

//
// this code needs a new home....
// and a big clean up...
//

uint32_t APP_SetFreqByStepAndLimits(VFO_Info_t *pInfo, int8_t direction, uint32_t lower, uint32_t upper)
{
	uint32_t Frequency = FREQUENCY_RoundToStep(pInfo->freq_config_RX.Frequency + (uint32_t)(direction * pInfo->StepFrequency), pInfo->StepFrequency);

	if (Frequency >= upper)
		Frequency =  lower;
	else if (Frequency < lower)
		Frequency = FREQUENCY_RoundToStep(upper - pInfo->StepFrequency, pInfo->StepFrequency);

	return Frequency;
}

uint32_t APP_SetFrequencyByStep(VFO_Info_t *pInfo, int8_t direction)
{
	return APP_SetFreqByStepAndLimits(pInfo, direction, frequencyBandTable[pInfo->Band].lower, frequencyBandTable[pInfo->Band].upper);
}


void CheckRadioInterrupts(void)
{
	/*if (SCANNER_IsScanning())
		return;*/

	//LogUartf("BK4819_REG_0C %b \r\n", BK4819_ReadRegister(BK4819_REG_0C));

	while (BK4819_ReadRegister(BK4819_REG_0C) & 1u) { // BK chip interrupt request
		// clear interrupts
		BK4819_WriteRegister(BK4819_REG_02, 0);
		// fetch interrupt status bits

		union {
			struct {
				uint16_t __UNUSED : 1;
				uint16_t fskRxSync : 1;
				uint16_t sqlLost : 1;
				uint16_t sqlFound : 1;
				uint16_t voxLost : 1;
				uint16_t voxFound : 1;
				uint16_t ctcssLost : 1;
				uint16_t ctcssFound : 1;
				uint16_t cdcssLost : 1;
				uint16_t cdcssFound : 1;
				uint16_t cssTailFound : 1;
				uint16_t dtmf5ToneFound : 1;
				uint16_t fskFifoAlmostFull : 1;
				uint16_t fskRxFinied : 1;
				uint16_t fskFifoAlmostEmpty : 1;
				uint16_t fskTxFinied : 1;
			};
			uint16_t __raw;
		} interrupts;		

		interrupts.__raw = BK4819_ReadRegister(BK4819_REG_02);

		//LogUartf("interrupts %0.16b \r\n", interrupts);

		// 0 = no phase shift
		// 1 = 120deg phase shift
		// 2 = 180deg phase shift
		// 3 = 240deg phase shift
//		const uint8_t ctcss_shift = BK4819_GetCTCShift();
//		if (ctcss_shift > 0)
//			g_CTCSS_Lost = true;
/*
		if (interrupts.dtmf5ToneFound) {	
			const char c = DTMF_GetCharacter(BK4819_GetDTMF_5TONE_Code()); // save the RX'ed DTMF character
			if (c != 0xff) {
				if (gCurrentFunction != FUNCTION_TRANSMIT) {
					if (gSetting_live_DTMF_decoder) {
						size_t len = strlen(gDTMF_RX_live);
						if (len >= sizeof(gDTMF_RX_live) - 1) { // make room
							memmove(&gDTMF_RX_live[0], &gDTMF_RX_live[1], sizeof(gDTMF_RX_live) - 1);
							len--;
						}
						gDTMF_RX_live[len++]  = c;
						gDTMF_RX_live[len]    = 0;
						gDTMF_RX_live_timeout = DTMF_RX_live_timeout_500ms;  // time till we delete it
						gUpdateDisplay        = true;
					}

#ifdef ENABLE_DTMF_CALLING
					if (gRxVfo->DTMF_DECODING_ENABLE || gSetting_KILLED) {
						if (gDTMF_RX_index >= sizeof(gDTMF_RX) - 1) { // make room
							memmove(&gDTMF_RX[0], &gDTMF_RX[1], sizeof(gDTMF_RX) - 1);
							gDTMF_RX_index--;
						}
						gDTMF_RX[gDTMF_RX_index++] = c;
						gDTMF_RX[gDTMF_RX_index]   = 0;
						gDTMF_RX_timeout           = DTMF_RX_timeout_500ms;  // time till we delete it
						gDTMF_RX_pending           = true;
						
						SYSTEM_DelayMs(3);//fix DTMF not reply@Yurisu
						DTMF_HandleRequest();
					}
#endif
				}
			}
		}
*/		

		if (interrupts.cssTailFound) {
			//main_push_message(RADIO_CSS_TAIL_FOUND);
			/*if (gSettings.ste) {
				AUDIO_AudioPathOff();
			}*/
		}

		if (interrupts.cdcssLost) {			
			gCDCSSCodeType = BK4819_GetCDCSSCodeType();
			g_CDCSS_Lost = true;
			//main_push_message(RADIO_CDCSS_LOST);		
		}

		if (interrupts.cdcssFound) {
			//main_push_message(RADIO_CDCSS_FOUND);
			g_CDCSS_Lost = false;
		}

		if (interrupts.ctcssLost) {
			g_CTCSS_Lost = true;
			//main_push_message(RADIO_CTCSS_LOST);
		}

		if (interrupts.ctcssFound) {
			//main_push_message(RADIO_CTCSS_FOUND);
			g_CTCSS_Lost = false;
		}
/*
#ifdef ENABLE_VOX
		if (interrupts.voxLost) {
			g_VOX_Lost         = true;
			gVoxPauseCountdown = 10;

			if (gSettings.VOX_SWITCH) {
				if (gCurrentFunction == FUNCTION_POWER_SAVE && !gRxIdleMode) {
					gPowerSave_10ms            = power_save2_10ms;
					gPowerSaveCountdownExpired = 0;
				}

				if (gSettings.DUAL_WATCH != DUAL_WATCH_OFF && (gScheduleDualWatch || gDualWatchCountdown_10ms < dual_watch_count_after_vox_10ms)) {
					gDualWatchCountdown_10ms = dual_watch_count_after_vox_10ms;
					gScheduleDualWatch = false;

					// let the user see DW is not active
					gDualWatchActive = false;
					//gUpdateStatus    = true;
				}
			}
		}

		if (interrupts.voxFound) {
			g_VOX_Lost         = false;
			gVoxPauseCountdown = 0;
		}
#endif
*/
		if (interrupts.sqlLost) {
			main_push_message(RADIO_SQUELCH_LOST);
		}

		if (interrupts.sqlFound) {
			main_push_message(RADIO_SQUELCH_FOUND);
		}
/*
#ifdef ENABLE_AIRCOPY
		if (interrupts.fskFifoAlmostFull &&
			gScreenToDisplay == DISPLAY_AIRCOPY &&
			gAircopyState == AIRCOPY_TRANSFER &&
			gAirCopyIsSendMode == 0)
		{
			for (unsigned int i = 0; i < 4; i++) {
				g_FSK_Buffer[gFSKWriteIndex++] = BK4819_ReadRegister(BK4819_REG_5F);
			}

			AIRCOPY_StorePacket();
		}
#endif

#ifdef ENABLE_MESSENGER
		MSG_StorePacket(interrupts.__raw);
#endif
*/
	}
}

void RADIO_EndTransmission(/*bool inmediately*/)
{
	RADIO_SendEndOfTransmission();

	/*if (gMonitor) {
		 //turn the monitor back on
		gFlagReconfigureVfos = true;
	}

	if (inmediately || gSettings.repeaterSte == 0) {
		FUNCTION_Select(FUNCTION_FOREGROUND);
	} else {
		gRTTECountdown = gSettings.repeaterSte * 10;
	}*/
}

void RADIO_StartListening(/*FUNCTION_Type_t function*/)
{
	const unsigned int vfo = gSettings.activeVFO;
/*
#ifdef ENABLE_DTMF_CALLING
	if (gSetting_KILLED)
		return;
#endif

#ifdef ENABLE_FMRADIO
	if (gFmRadioMode)
		BK1080_Init0();
#endif
*/
	// clear the other vfo's rssi level (to hide the antenna symbol)
	gVFO_RSSI_bar_level[!vfo] = 0;

	AUDIO_AudioPathOn();
	//gEnableSpeaker = true;

	//if (gSetting_backlight_on_tx_rx & BACKLIGHT_ON_TR_RX) {
		BACKLIGHT_TurnOn();
	//}

	/*if (gScanStateDir != SCAN_OFF)
		CHFRSCANNER_Found();

	if (gScanStateDir == SCAN_OFF &&
	    gSettings.DUAL_WATCH != DUAL_WATCH_OFF)
	{	// not scanning, dual watch is enabled

		gDualWatchCountdown_10ms = dual_watch_count_after_2_10ms;
		gScheduleDualWatch       = false;

		// when crossband is active only the main VFO should be used for TX
		if(gSettings.CROSS_BAND_RX_TX == CROSS_BAND_OFF)
			gRxVfoIsActive = true;

		// let the user see DW is not active
		gDualWatchActive = false;
		//gUpdateStatus    = true;
	}*/

	BK4819_WriteRegister(BK4819_REG_48, (uint16_t)(
		(11u << 12)                |     // ??? .. 0 to 15, doesn't seem to make any difference
		( 1u << 10)                |     // AF Rx Gain-1
		(/*gSettings.VOLUME_GAIN*/56 << 4) |     // AF Rx Gain-2
		(/*gSettings.DAC_GAIN*/8    << 0)
		
	));     // AF DAC Gain (after Gain-1 and Gain-2)

		RADIO_SetModulation(gRxVfo->Modulation);  // no need, set it now

	//FUNCTION_Select(function);

/*
#ifdef ENABLE_FMRADIO
	if (function == FUNCTION_MONITOR || gFmRadioMode)
#else
	if (function == FUNCTION_MONITOR)
#endif
	{	// squelch is disabled
		if (gScreenToDisplay != DISPLAY_MENU)     // 1of11 .. don't close the menu
			GUI_SelectNextDisplay(DISPLAY_MAIN);
	}
	//else
		//gUpdateDisplay = true;

	//gUpdateStatus = true;
*/
}

/* --------------------------------------------------------------------------------------------------------- */

void init_radio(void) {

	BK4819_Init();

	BOARD_ADC_GetBatteryInfo(&gBatteryCurrentVoltage, &gBatteryCurrent);

	SETTINGS_InitEEPROM();

	ST7565_SetContrast(gSettings.contrast);

	//SETTINGS_WriteBuildOptions();
	SETTINGS_LoadCalibration();

	RADIO_ConfigureChannel(0, VFO_CONFIGURE_RELOAD);
	RADIO_ConfigureChannel(1, VFO_CONFIGURE_RELOAD);

	RADIO_SelectVfos();

	RADIO_SetupRegisters(true);

	for (unsigned int i = 0; i < ARRAY_SIZE(gBatteryVoltages); i++)
		BOARD_ADC_GetBatteryInfo(&gBatteryVoltages[i], &gBatteryCurrent);

	BATTERY_GetReadings(false);

	GPIO_ClearBit(&GPIOA->DATA, GPIOA_PIN_VOICE_0);

	//gUpdateStatus = true;

}

/*static void HandlePowerSave()
{
	if (!gRxIdleMode) {
		CheckForIncoming();
	}
}*/


/*
static void HandleIncoming(void)
{
	APP_StartListening(gMonitor ? FUNCTION_MONITOR : FUNCTION_RECEIVE);
}

static void (*HandleFunction_fn_table[])(void) = {
	[FUNCTION_FOREGROUND] = &CheckForIncoming,
	[FUNCTION_TRANSMIT] = &FUNCTION_NOP,
	[FUNCTION_MONITOR] = &FUNCTION_NOP,
	[FUNCTION_INCOMING] = &HandleIncoming,
	[FUNCTION_RECEIVE] = &HandleReceive,
	[FUNCTION_POWER_SAVE] = &HandlePowerSave,
	[FUNCTION_BAND_SCOPE] = &FUNCTION_NOP,
};*/

void APP_Function(FUNCTION_Type_t function) {	
	(void)function;
	//HandleFunction_fn_table[function]();

}


void COMMON_SwitchVFOMode() {

    //if (gSettings.VFO_OPEN) {
        if (IS_MR_CHANNEL(gTxVfo->CHANNEL_SAVE))
        {	// swap to frequency mode
            gScreenChannel[gSettings.activeVFO] = gFreqChannel[gSettings.activeVFO];
            //gRequestSaveVFO            = true;
            //gVfoConfigureMode          = VFO_CONFIGURE_RELOAD;
            return;
        }

        uint8_t Channel = RADIO_FindNextChannel(gMrChannel[gSettings.activeVFO], 1, false, 0);
        if (Channel != 0xFF)
        {	// swap to channel mode
            gScreenChannel[gSettings.activeVFO] = Channel;
            //gRequestSaveVFO     = true;
            //gVfoConfigureMode   = VFO_CONFIGURE_RELOAD;
            return;
        }
    //}
}

void COMMON_SwitchVFOs()
{
/*#ifdef ENABLE_SCAN_RANGES    
    gScanRangeStart = 0;
#endif*/
    //gSettings.activeVFO ^= 1;
	gSettings.activeVFO = !gSettings.activeVFO;

    /*if (gSettings.CROSS_BAND_RX_TX != CROSS_BAND_OFF)
        gSettings.CROSS_BAND_RX_TX = gSettings.activeVFO + 1;
    if (gSettings.DUAL_WATCH != DUAL_WATCH_OFF)
        gSettings.DUAL_WATCH = gSettings.activeVFO + 1;*/

    //gRequestSaveSettings  = 1;
    //gFlagReconfigureVfos  = true;
    //gScheduleDualWatch = true;

    //gRequestDisplayScreen = DISPLAY_MAIN;
}

/* --------------------------------------------------------------------------------------------------------- */
// Hardware interrupt handlers


void HandlerGPIOB(void) {
	/*if (GPIOB_ENUM_EQUALS(INTSTAUS, 14, ASSERTED)) {
		GPIOB->INTCLR |= GPIO_INTCLR_14_BITS_CLEAR_EDGE;
		CheckRadioInterrupts();		
		BK4819_WriteRegister(BK4819_REG_02, 0);
	}*/
}



void HandlerUART1(void) {
	if (UART_IsCommandAvailable()) {
		UART_HandleCommand();
	}
	UART1->IF |= UART_IF_RXTO_BITS_NOT_SET | UART_IF_RXFIFO_OVF_BITS_NOT_SET;
}


/* --------------------------------------------------------------------------------------------------------- */

void DTMF_Reply(void)
{
	uint16_t    Delay;

	if ( gCurrentVfo->DTMF_PTT_ID_TX_MODE == PTT_ID_OFF    ||
		 gCurrentVfo->DTMF_PTT_ID_TX_MODE == PTT_ID_TX_DOWN) {
		return;
	}

	Delay = 200;//(gSettings.DTMF_PRELOAD_TIME < 200) ? 200 : gSettings.DTMF_PRELOAD_TIME;

	/*if (false)
	{	// the user will also hear the transmitted tones
		AUDIO_AudioPathOn();
	}*/

	SYSTEM_DelayMs(Delay);

	BK4819_EnterDTMF_TX(/*false*/false);

	/*BK4819_PlayDTMFString(
		gSettings.DTMF_UP_CODE,
		1,
		gSettings.DTMF_FIRST_CODE_PERSIST_TIME,
		gSettings.DTMF_HASH_CODE_PERSIST_TIME,
		gSettings.DTMF_CODE_PERSIST_TIME,
		gSettings.DTMF_CODE_INTERVAL_TIME);*/

	AUDIO_AudioPathOff();

	BK4819_ExitDTMF_TX(false);
}


void BK4819_ToggleAFBit(bool on) {
  uint16_t reg = BK4819_ReadRegister(BK4819_REG_47);
  reg &= (uint16_t)~(1 << 8);
  if (on)
    reg |= 1 << 8;
  BK4819_WriteRegister(BK4819_REG_47, reg);
}

void BK4819_ToggleAFDAC(bool on) {
  uint16_t Reg = BK4819_ReadRegister(BK4819_REG_30);
  Reg &= (uint16_t)~BK4819_REG_30_ENABLE_AF_DAC;
  if (on)
    Reg |= BK4819_REG_30_ENABLE_AF_DAC;
  BK4819_WriteRegister(BK4819_REG_30, Reg);
}

void BK4819_SetMode(bool on) {
  if (on) {
    BK4819_ToggleAFDAC(true);
    BK4819_ToggleAFBit(true);

    SYSTEM_DelayMs(10);
	
	gCurrentCodeType = (gRxVfo->Modulation != MODULATION_FM) ? CODE_TYPE_OFF : gRxVfo->pRX->CodeType;
	if (gCurrentCodeType != CODE_TYPE_OFF) {
		AUDIO_AudioPathOff();
	} else {
		AUDIO_AudioPathOn();
	}

  } else {
	AUDIO_AudioPathOff();
    SYSTEM_DelayMs(10);

    BK4819_ToggleAFDAC(false);
    BK4819_ToggleAFBit(false);
  }
}

void RADIO_SetRX(bool on) {
  if (gIsReceiving == on) {
    return;
  }
  gIsReceiving = on;  
  BK4819_ToggleGpioOut(BK4819_GPIO6_PIN2_GREEN, on);
  BK4819_SetMode(on);
}


void RADIO_SetTransmit() {

	RADIO_SetRX(false);
	//RADIO_SetVfoState(VFO_STATE_NORMAL);
	RADIO_PrepareTX();

	// if DTMF is enabled when TX'ing, it changes the TX audio filtering !! .. 1of11
	BK4819_DisableDTMF();
	RADIO_SetTxParameters();

	// turn the RED LED on
	BK4819_ToggleGpioOut(BK4819_GPIO5_PIN1_RED, true);	

	if (gCurrentVfo->DTMF_PTT_ID_TX_MODE == PTT_ID_APOLLO) {
		BK4819_PlaySingleTone(2525, 250, 0, false);
	} /*else {
		DTMF_Reply();
	}*/

}

void RADIO_Handler(void) {

	/*if (UART_IsCommandAvailable()) {
		UART_HandleCommand();
	}*/

	CheckRadioInterrupts();
	if (gIsReceiving) {
		if (g_CTCSS_Lost && gCurrentCodeType == CODE_TYPE_CONTINUOUS_TONE && BK4819_GetCTCType() == 1) {		
			g_CTCSS_Lost = false;
			AUDIO_AudioPathOn();
			//LogUartf("CTCSS_Lost \r\n");
		}

		if (g_CDCSS_Lost && gCDCSSCodeType == CDCSS_POSITIVE_CODE
	    	&& (gCurrentCodeType == CODE_TYPE_DIGITAL || gCurrentCodeType == CODE_TYPE_REVERSE_DIGITAL) ) {
			g_CDCSS_Lost = false;
			AUDIO_AudioPathOn();
			//LogUartf("CDCSS_Lost \r\n");
		}

	}
}


void main_task(void* arg) {
	(void)arg;

	BOARD_Init();

	UART_Init();
	LogUartf("\r\n\r\n");
	UART_Send(UART_Version, strlen(UART_Version));
	LogUartf("\r\nUV-K5 Starting... \r\n\r\n");


	init_radio();

	mainTasksMsgQueue = xQueueCreateStatic(QUEUE_LENGTH, ITEM_SIZE, mainQueueStorageArea, &mainTasksQueue);

	main_push_message(MAIN_MSG_INIT);

	//radioStatusTimer = xTimerCreateStatic("radioStatus", pdMS_TO_TICKS(50), pdFALSE, NULL, radio_timer_callback, &radioStatusTimerBuffer);

	BACKLIGHT_TurnOn();

	//LogUartf("\r\n BL : %i \r\n", gSettings.backlight);

	applications_task_init();

	//xTimerStart(radioStatusTimer500, 0);
	//xTimerStart(radioStatusTimer, 0);
	
	//LogUartf("Main Task Ready... \r\n");

	//NVIC_EnableIRQ((IRQn_Type)DP32_GPIOB_IRQn);

	NVIC_EnableIRQ((IRQn_Type)DP32_UART1_IRQn);

	FUNCTION_Init();
	//FUNCTION_Select(FUNCTION_RECEIVE);

	MAIN_Messages_t msg;

	for (;;) {
		
    	if (xQueueReceive(mainTasksMsgQueue, &msg, pdMS_TO_TICKS(1))) {
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
					//LogUartf("RADIO_TX\r\n");					
					RADIO_SetTransmit();
					break;

				case RADIO_RX:
					//LogUartf("RADIO_RX\r\n");
					RADIO_SetVfoState(VFO_STATE_NORMAL);
					RADIO_SendEndOfTransmission();
					RADIO_SetRX(false);
					break;

				case RADIO_SQUELCH_LOST:	
					//LogUartf("RADIO_SQUELCH_LOST\r\n");				
					RADIO_SetRX(true);
					app_push_message(APP_MSG_RX);
                    break;

				case RADIO_SQUELCH_FOUND:
					RADIO_SetRX(false);
                    break;

				case RADIO_CSS_TAIL_FOUND:
					break;

				case RADIO_CTCSS_LOST:
					LogUartf("CTCSS_LOST CT = (%i) CTCType = (%i) (%b)\r\n", gCurrentCodeType, BK4819_GetCTCType(), g_CDCSS_Lost);
					//if (BK4819_GetCTCType() == 1 && gCurrentCodeType == CODE_TYPE_CONTINUOUS_TONE) {
						/*BK4819_ToggleGpioOut(BK4819_GPIO6_PIN2_GREEN, true);						
						FUNCTION_Select(FUNCTION_INCOMING);
						RADIO_StartListening();
						app_push_message(APP_MSG_RX);*/
						//HandleReceive();
					//}
					break;

				case RADIO_CTCSS_FOUND:
					break;

				case RADIO_CDCSS_LOST:
					//gCDCSSCodeType = BK4819_GetCDCSSCodeType();
					LogUartf("CDCSS_LOST CD = (%i)  CDCSSCodeType = (%i)\r\n", gCurrentCodeType, gCDCSSCodeType);
					//if (gCDCSSCodeType == 1 && (gCurrentCodeType == CODE_TYPE_DIGITAL || gCurrentCodeType == CODE_TYPE_REVERSE_DIGITAL)) {
						/*BK4819_ToggleGpioOut(BK4819_GPIO6_PIN2_GREEN, true);
						FUNCTION_Select(FUNCTION_INCOMING);
						RADIO_StartListening();						
						app_push_message(APP_MSG_RX);*/
						//HandleReceive();
					//}
					break;

				case RADIO_CDCSS_FOUND:
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
					RADIO_ConfigureChannel(gSettings.activeVFO, VFO_CONFIGURE);
                    break;

				case RADIO_VFO_CONFIGURE_CHANNEL:
					RADIO_ConfigureChannel(gSettings.activeVFO, VFO_CONFIGURE_RELOAD);
                    break;

				case RADIO_SAVE_CHANNEL:
					SETTINGS_SaveChannel(gTxVfo->CHANNEL_SAVE, gSettings.activeVFO, gTxVfo, 1);
					RADIO_SetupRegisters(true);
					FUNCTION_Init();
					break;

				case RADIO_SAVE_SETTINGS:
					//SETTINGS_SaveSettings();
                    break;

				case RADIO_SET_CHANNEL:
					if ( msg.payload != 0 ) {
						if (!RADIO_CheckValidChannel((uint16_t)msg.payload, false, 0)) {
				            main_push_message_value(MAIN_MSG_PLAY_BEEP, BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL);				            
			            } else {
							gMrChannel[gSettings.activeVFO]     = (uint8_t)msg.payload;
							gScreenChannel[gSettings.activeVFO] = (uint8_t)msg.payload;
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
                            gScreenChannel[gSettings.activeVFO] 	= (uint8_t)(band + FREQ_CHANNEL_FIRST);
                            gFreqChannel[gSettings.activeVFO]   	= (uint8_t)(band + FREQ_CHANNEL_FIRST);

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

		RADIO_Handler();
		//vTaskDelay(pdMS_TO_TICKS(1));
	}
}


void main_push_message_value(MAIN_MSG_t msg, uint32_t value) {
	MAIN_Messages_t mainMSG = { msg, value };
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(mainTasksMsgQueue, (void *)&mainMSG, &xHigherPriorityTaskWoken);
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
