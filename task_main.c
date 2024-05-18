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

#include "frequencies.h"

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

//
// this code needs a new home....
// and a big clean up...
//

uint32_t APP_SetFreqByStepAndLimits(VFO_Info_t *pInfo, int8_t direction, uint32_t lower, uint32_t upper)
{
	uint32_t Frequency = FREQUENCY_RoundToStep(pInfo->freq_config_RX.Frequency + (direction * pInfo->StepFrequency), pInfo->StepFrequency);

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

		if (interrupts.cssTailFound)
			g_CxCSS_TAIL_Found = true;

		if (interrupts.cdcssLost) {
			g_CDCSS_Lost = true;
			gCDCSSCodeType = BK4819_GetCDCSSCodeType();
			main_push_message(RADIO_CDCSS_LOST);
		}

		if (interrupts.cdcssFound) {
			g_CDCSS_Lost = false;
			main_push_message(RADIO_CDCSS_FOUND);
		}

		if (interrupts.ctcssLost) {
			g_CTCSS_Lost = true;
			main_push_message(RADIO_CTCSS_LOST);
		}

		if (interrupts.ctcssFound) {
			g_CTCSS_Lost = false;
			main_push_message(RADIO_CTCSS_FOUND);
		}
/*
#ifdef ENABLE_VOX
		if (interrupts.voxLost) {
			g_VOX_Lost         = true;
			gVoxPauseCountdown = 10;

			if (gEeprom.VOX_SWITCH) {
				if (gCurrentFunction == FUNCTION_POWER_SAVE && !gRxIdleMode) {
					gPowerSave_10ms            = power_save2_10ms;
					gPowerSaveCountdownExpired = 0;
				}

				if (gEeprom.DUAL_WATCH != DUAL_WATCH_OFF && (gScheduleDualWatch || gDualWatchCountdown_10ms < dual_watch_count_after_vox_10ms)) {
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
			//g_SquelchLost = true;
			BK4819_ToggleGpioOut(BK4819_GPIO6_PIN2_GREEN, true);
			//LogUartf("sqlLost \r\n");
			main_push_message(RADIO_SQUELCH_LOST);
		}

		if (interrupts.sqlFound) {
			//g_SquelchLost = false;
			BK4819_ToggleGpioOut(BK4819_GPIO6_PIN2_GREEN, false);
			//LogUartf("sqlFound \r\n");
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

void APP_EndTransmission(bool inmediately)
{
	RADIO_SendEndOfTransmission();

	if (gMonitor) {
		 //turn the monitor back on
		gFlagReconfigureVfos = true;
	}

	if (inmediately || gEeprom.REPEATER_TAIL_TONE_ELIMINATION == 0) {
		FUNCTION_Select(FUNCTION_FOREGROUND);
	} else {
		gRTTECountdown = gEeprom.REPEATER_TAIL_TONE_ELIMINATION * 10;
	}
}

void APP_StartListening(FUNCTION_Type_t function)
{
	const unsigned int vfo = gEeprom.RX_VFO;

#ifdef ENABLE_DTMF_CALLING
	if (gSetting_KILLED)
		return;
#endif

#ifdef ENABLE_FMRADIO
	if (gFmRadioMode)
		BK1080_Init0();
#endif

	// clear the other vfo's rssi level (to hide the antenna symbol)
	gVFO_RSSI_bar_level[!vfo] = 0;

	AUDIO_AudioPathOn();
	gEnableSpeaker = true;

	if (gSetting_backlight_on_tx_rx & BACKLIGHT_ON_TR_RX) {
		BACKLIGHT_TurnOn();
	}

	/*if (gScanStateDir != SCAN_OFF)
		CHFRSCANNER_Found();

	if (gScanStateDir == SCAN_OFF &&
	    gEeprom.DUAL_WATCH != DUAL_WATCH_OFF)
	{	// not scanning, dual watch is enabled

		gDualWatchCountdown_10ms = dual_watch_count_after_2_10ms;
		gScheduleDualWatch       = false;

		// when crossband is active only the main VFO should be used for TX
		if(gEeprom.CROSS_BAND_RX_TX == CROSS_BAND_OFF)
			gRxVfoIsActive = true;

		// let the user see DW is not active
		gDualWatchActive = false;
		//gUpdateStatus    = true;
	}*/

	BK4819_WriteRegister(BK4819_REG_48,
		(11u << 12)                |     // ??? .. 0 to 15, doesn't seem to make any difference
		( 0u << 10)                |     // AF Rx Gain-1
		(gEeprom.VOLUME_GAIN << 4) |     // AF Rx Gain-2
		(gEeprom.DAC_GAIN    << 0));     // AF DAC Gain (after Gain-1 and Gain-2)

		RADIO_SetModulation(gRxVfo->Modulation);  // no need, set it now

	FUNCTION_Select(function);

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
}

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


	GPIO_ClearBit(&GPIOA->DATA, GPIOA_PIN_VOICE_0);

	//gUpdateStatus = true;

}

static void CheckForIncoming(void)
{
	/*if (!g_SquelchLost)
		return;          // squelch is closed
*/
	// squelch is open

	//if (gScanStateDir == SCAN_OFF)
	{	// not RF scanning
		if (gEeprom.DUAL_WATCH == DUAL_WATCH_OFF)
		{	// dual watch is disabled

			if (gCurrentFunction != FUNCTION_INCOMING)
			{
				FUNCTION_Select(FUNCTION_INCOMING);
				//gUpdateDisplay = true;
			}

			return;
		}

		// dual watch is enabled and we're RX'ing a signal

		if (gRxReceptionMode != RX_MODE_NONE)
		{
			if (gCurrentFunction != FUNCTION_INCOMING)
			{
				FUNCTION_Select(FUNCTION_INCOMING);
				//gUpdateDisplay = true;
			}
			return;
		}

		gDualWatchCountdown_10ms = dual_watch_count_after_rx_10ms;
		gScheduleDualWatch       = false;

		// let the user see DW is not active
		gDualWatchActive = false;
		//gUpdateStatus    = true;
	}
	/*else
	{	// RF scanning
		if (gRxReceptionMode != RX_MODE_NONE)
		{
			if (gCurrentFunction != FUNCTION_INCOMING)
			{
				FUNCTION_Select(FUNCTION_INCOMING);
				//gUpdateDisplay = true;
			}
			return;
		}

		gScanPauseDelayIn_10ms = scan_pause_delay_in_3_10ms;
		gScheduleScanListen    = false;
	}*/

	gRxReceptionMode = RX_MODE_DETECTED;

	if (gCurrentFunction != FUNCTION_INCOMING)
	{
		FUNCTION_Select(FUNCTION_INCOMING);
		//gUpdateDisplay = true;
	}
}

static void HandlePowerSave()
{
	if (!gRxIdleMode) {
		CheckForIncoming();
	}
}


static void HandleReceive(void)
{
	#define END_OF_RX_MODE_SKIP 0
	#define END_OF_RX_MODE_END  1
	#define END_OF_RX_MODE_TTE  2

	uint8_t Mode = END_OF_RX_MODE_SKIP;

	if (gFlagTailNoteEliminationComplete)
	{
		Mode = END_OF_RX_MODE_END;
		goto Skip;
	}

	/*if (gScanStateDir != SCAN_OFF && IS_FREQ_CHANNEL(gNextMrChannel))
	{ // we are scanning in the frequency mode
		if (g_SquelchLost)
			return;

		Mode = END_OF_RX_MODE_END;
		goto Skip;
	}*/

	if (gCurrentCodeType != CODE_TYPE_OFF
		&& ((gFoundCTCSS && gFoundCTCSSCountdown_10ms == 0)
			|| (gFoundCDCSS && gFoundCDCSSCountdown_10ms == 0))
	){
		gFoundCTCSS = false;
		gFoundCDCSS = false;
		Mode        = END_OF_RX_MODE_END;
		goto Skip;
	}

	if (g_SquelchLost)
	{
		if (!gEndOfRxDetectedMaybe) {
			switch (gCurrentCodeType)
			{
				case CODE_TYPE_OFF:
					if (gEeprom.SQUELCH_LEVEL)
					{
						if (g_CxCSS_TAIL_Found)
						{
							Mode               = END_OF_RX_MODE_TTE;
							g_CxCSS_TAIL_Found = false;
						}
					}
					break;

				case CODE_TYPE_CONTINUOUS_TONE:
					if (g_CTCSS_Lost)
					{
						gFoundCTCSS = false;
					}
					else
					if (!gFoundCTCSS)
					{
						gFoundCTCSS               = true;
						gFoundCTCSSCountdown_10ms = 100;   // 1 sec
					}

					if (g_CxCSS_TAIL_Found)
					{
						Mode               = END_OF_RX_MODE_TTE;
						g_CxCSS_TAIL_Found = false;
					}
					break;

				case CODE_TYPE_DIGITAL:
				case CODE_TYPE_REVERSE_DIGITAL:
					if (g_CDCSS_Lost && gCDCSSCodeType == CDCSS_POSITIVE_CODE)
					{
						gFoundCDCSS = false;
					}
					else
					if (!gFoundCDCSS)
					{
						gFoundCDCSS               = true;
						gFoundCDCSSCountdown_10ms = 100;   // 1 sec
					}

					if (g_CxCSS_TAIL_Found)
					{
						if (BK4819_GetCTCType() == 1)
							Mode = END_OF_RX_MODE_TTE;

						g_CxCSS_TAIL_Found = false;
					}

					break;
			}
		}
	}
	else
		Mode = END_OF_RX_MODE_END;

	if (!gEndOfRxDetectedMaybe         &&
	     Mode == END_OF_RX_MODE_SKIP   &&
	     gNextTimeslice40ms            &&
	     gEeprom.TAIL_TONE_ELIMINATION &&
	    (gCurrentCodeType == CODE_TYPE_DIGITAL || gCurrentCodeType == CODE_TYPE_REVERSE_DIGITAL) &&
	     BK4819_GetCTCType() == 1)
		Mode = END_OF_RX_MODE_TTE;
	else
		gNextTimeslice40ms = false;

Skip:
	switch (Mode)
	{
		case END_OF_RX_MODE_SKIP:
			break;

		case END_OF_RX_MODE_END:
			RADIO_SetupRegisters(true);

			//gUpdateDisplay = true;

			/*if (gScanStateDir != SCAN_OFF)
			{
				switch (gEeprom.SCAN_RESUME_MODE)
				{
					case SCAN_RESUME_TO:
						break;

					case SCAN_RESUME_CO:
						gScanPauseDelayIn_10ms = scan_pause_delay_in_7_10ms;
						gScheduleScanListen    = false;
						break;

					case SCAN_RESUME_SE:
						CHFRSCANNER_Stop();
						break;
				}
			}*/

			break;

		case END_OF_RX_MODE_TTE:
			if (gEeprom.TAIL_TONE_ELIMINATION)
			{
				AUDIO_AudioPathOff();

				gTailNoteEliminationCountdown_10ms = 20;
				gFlagTailNoteEliminationComplete   = false;
				gEndOfRxDetectedMaybe = true;
				gEnableSpeaker        = false;
			}
			break;
	}
}

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
};

void APP_Function(FUNCTION_Type_t function) {	
	HandleFunction_fn_table[function]();
}


void COMMON_SwitchVFOMode()
{

    if (gEeprom.VFO_OPEN) {
        if (IS_MR_CHANNEL(gTxVfo->CHANNEL_SAVE))
        {	// swap to frequency mode
            gEeprom.ScreenChannel[gEeprom.TX_VFO] = gEeprom.FreqChannel[gEeprom.TX_VFO];
            //gRequestSaveVFO            = true;
            //gVfoConfigureMode          = VFO_CONFIGURE_RELOAD;
            return;
        }

        uint8_t Channel = RADIO_FindNextChannel(gEeprom.MrChannel[gEeprom.TX_VFO], 1, false, 0);
        if (Channel != 0xFF)
        {	// swap to channel mode
            gEeprom.ScreenChannel[gEeprom.TX_VFO] = Channel;
            //gRequestSaveVFO     = true;
            //gVfoConfigureMode   = VFO_CONFIGURE_RELOAD;
            return;
        }
    }
}

void COMMON_SwitchVFOs()
{
/*#ifdef ENABLE_SCAN_RANGES    
    gScanRangeStart = 0;
#endif*/
    gEeprom.TX_VFO ^= 1;

    if (gEeprom.CROSS_BAND_RX_TX != CROSS_BAND_OFF)
        gEeprom.CROSS_BAND_RX_TX = gEeprom.TX_VFO + 1;
    if (gEeprom.DUAL_WATCH != DUAL_WATCH_OFF)
        gEeprom.DUAL_WATCH = gEeprom.TX_VFO + 1;

    //gRequestSaveSettings  = 1;
    //gFlagReconfigureVfos  = true;
    //gScheduleDualWatch = true;

    //gRequestDisplayScreen = DISPLAY_MAIN;
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

	for (;;) {
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
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
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
