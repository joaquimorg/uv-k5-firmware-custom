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

#include <string.h>
#include "FreeRTOS.h"
#include "task.h"

#if !defined(ENABLE_OVERLAY)
	#include "ARMCM0.h"
#endif
#include "app/dtmf.h"
#include "app/generic.h"
#include "app/menu.h"
#include "app/scanner.h"
#include "audio.h"
#include "board.h"
#include "bsp/dp32g030/gpio.h"
#include "driver/backlight.h"
#include "driver/bk4819.h"
#include "driver/eeprom.h"
#include "driver/gpio.h"
#include "driver/keyboard.h"
#include "driver/st7565.h"
#include "frequencies.h"
#include "helper/battery.h"
#include "misc.h"
#include "settings.h"
#if defined(ENABLE_OVERLAY)
	#include "sram-overlay.h"
#endif
#include "ui/inputbox.h"
#include "ui/menu.h"
#include "ui/ui.h"

#ifdef ENABLE_UART
	#include "driver/uart.h"
#endif


#ifndef ARRAY_SIZE
	#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#endif

uint8_t gUnlockAllTxConfCnt;

#ifdef ENABLE_F_CAL_MENU
	void writeXtalFreqCal(const int32_t value, const bool update_eeprom)
	{
		BK4819_WriteRegister(BK4819_REG_3B, 22656 + value);

		if (update_eeprom)
		{
			struct
			{
				int16_t  BK4819_XtalFreqLow;
				uint16_t EEPROM_1F8A;
				uint16_t EEPROM_1F8C;
				uint8_t  VOLUME_GAIN;
				uint8_t  DAC_GAIN;
			} __attribute__((packed)) misc;

			gEeprom.BK4819_XTAL_FREQ_LOW = value;

			// radio 1 .. 04 00 46 00 50 00 2C 0E
			// radio 2 .. 05 00 46 00 50 00 2C 0E
			//
			EEPROM_ReadBuffer(0x1F88, &misc, 8);
			misc.BK4819_XtalFreqLow = value;
			EEPROM_WriteBuffer(0x1F88, &misc);
		}
	}
#endif

void MENU_StartCssScan(void)
{
	SCANNER_Start(true);
	gUpdateStatus = true;
	gCssBackgroundScan = true;

	gRequestDisplayScreen = DISPLAY_MENU;
}

void MENU_CssScanFound(void)
{
	if(gScanCssResultType == CODE_TYPE_DIGITAL || gScanCssResultType == CODE_TYPE_REVERSE_DIGITAL) {
		settingsCurrentMenu = UI_MENU_GetMenuIdx(MENU_R_DCS);
	}
	else if(gScanCssResultType == CODE_TYPE_CONTINUOUS_TONE) {
		settingsCurrentMenu = UI_MENU_GetMenuIdx(MENU_R_CTCS);
	}

	MENU_ShowCurrentSetting();

	gUpdateStatus = true;
	//gUpdateDisplay = true;
}

void MENU_StopCssScan(void)
{
	gCssBackgroundScan = false;

	//gUpdateDisplay = true;
	gUpdateStatus = true;
}

int MENU_GetLimits(uint8_t menu_id, uint16_t *pMin, uint16_t *pMax)
{
	*pMin = 0;
	switch (menu_id)
	{
		case MENU_SQL:
			*pMax = 9;
			break;

		case MENU_STEP:
			*pMax = STEP_N_ELEM - 1;
			break;

		case MENU_ABR:
			*pMax = ARRAY_SIZE(gSubMenu_BACKLIGHT);
			break;

		case MENU_ABR_MIN:
			*pMax = 9;
			break;

		case MENU_ABR_MAX:
			*pMin = 1;
			*pMax = 10;
			break;

		case MENU_CONTRAST:
			*pMin = 1;
			*pMax = 63;
			break;	

		case MENU_F_LOCK:
			*pMax = ARRAY_SIZE(gSubMenu_F_LOCK);
			break;

		/*case MENU_MDF:
			*pMax = ARRAY_SIZE(gSubMenu_MDF);
			break;*/

		/*case MENU_TXP:
			*pMax = ARRAY_SIZE(gSubMenu_TXP);
			break;*/

		case MENU_SFT_D:
			*pMax = ARRAY_SIZE(gSubMenu_SFT_D);
			break;

		case MENU_TDR:
			*pMax = ARRAY_SIZE(gSubMenu_RXMode);
			break;

		case MENU_SC_REV:
			*pMax = ARRAY_SIZE(gSubMenu_SC_REV);
			break;

		case MENU_ROGER:
			*pMax = ARRAY_SIZE(gSubMenu_ROGER);
			break;

		case MENU_PONMSG:
			*pMax = ARRAY_SIZE(gSubMenu_PONMSG);
			break;

		case MENU_R_DCS:
		case MENU_T_DCS:
			*pMax = 208;
			//*pMax = (ARRAY_SIZE(DCS_Options) * 2);
			break;

		case MENU_R_CTCS:
		case MENU_T_CTCS:
			*pMax = ARRAY_SIZE(CTCSS_Options);
			break;

		case MENU_W_N:
			*pMax = ARRAY_SIZE(gSubMenu_W_N);
			break;

		#ifdef ENABLE_ALARM
			case MENU_AL_MOD:

				*pMax = ARRAY_SIZE(gSubMenu_AL_MOD);
				break;
		#endif

		case MENU_RESET:
			*pMax = ARRAY_SIZE(gSubMenu_RESET);
			break;

		case MENU_COMPAND:
		case MENU_ABR_ON_TX_RX:
			*pMax = ARRAY_SIZE(gSubMenu_RX_TX);
			break;
		#ifdef ENABLE_AM_FIX
			case MENU_AM_FIX:
		#endif
		#ifdef ENABLE_AUDIO_BAR
			case MENU_MIC_BAR:
		#endif
		case MENU_BCL:
		case MENU_BEEP:
		case MENU_AUTOLK:
		case MENU_S_ADD1:
		case MENU_S_ADD2:
		case MENU_STE:
		case MENU_D_ST:
#ifdef ENABLE_DTMF_CALLING
		case MENU_D_DCD:
#endif
		case MENU_D_LIVE_DEC:
		case MENU_350TX:
		case MENU_200TX:
		case MENU_500TX:
		case MENU_350EN:
		case MENU_SCREN:
			*pMax = ARRAY_SIZE(gSubMenu_OFF_ON);
			break;

		case MENU_AM:
			*pMax = ARRAY_SIZE(gModulationStr);
			break;

		case MENU_SCR:
			*pMax = ARRAY_SIZE(gSubMenu_SCRAMBLER);
			break;

		case MENU_TOT:
			*pMax = ARRAY_SIZE(gSubMenu_TOT);
			break;

		#ifdef ENABLE_VOX
			case MENU_VOX:
		#endif
		case MENU_RP_STE:
			*pMax = 10;
			break;

		case MENU_MEM_CH:
		case MENU_1_CALL:
		case MENU_DEL_CH:
		case MENU_MEM_NAME:
			*pMax = MR_CHANNEL_LAST;
			break;

		case MENU_SLIST1:
		case MENU_SLIST2:
			*pMin = -1;
			*pMax = MR_CHANNEL_LAST;
			break;

		case MENU_SAVE:
			*pMax = ARRAY_SIZE(gSubMenu_SAVE);
			break;

		case MENU_MIC:
			*pMax = 6;
			break;

		case MENU_S_LIST:
			*pMax = 2;
			break;

#ifdef ENABLE_DTMF_CALLING
		case MENU_D_RSP:
			*pMax = ARRAY_SIZE(gSubMenu_D_RSP);
			break;
#endif
		case MENU_PTT_ID:
			*pMax = ARRAY_SIZE(gSubMenu_PTT_ID);
			break;

		case MENU_BAT_TXT:
			*pMax = ARRAY_SIZE(gSubMenu_BAT_TXT);
			break;

#ifdef ENABLE_DTMF_CALLING
		case MENU_D_HOLD:
			*pMin = 5;
			*pMax = 60;
			break;
#endif
		case MENU_D_PRE:
			*pMin = 3;
			*pMax = 99;
			break;

#ifdef ENABLE_DTMF_CALLING
		case MENU_D_LIST:
			*pMin = 1;
			*pMax = 16;
			break;
#endif
		#ifdef ENABLE_F_CAL_MENU
			case MENU_F_CALI:
				*pMin = -50;
				*pMax = +50;
				break;
		#endif

		case MENU_BATCAL:
			*pMin = 1600;
			*pMax = 2200;
			break;

		case MENU_BATTYP:
			*pMax = 1;
			break;

		case MENU_F1SHRT:
		case MENU_F1LONG:
		case MENU_F2SHRT:
		case MENU_F2LONG:
		case MENU_MLONG:
			*pMax = gSubMenu_SIDEFUNCTIONS_size-1;
			break;

		default:
			return -1;
	}

	return 0;
}

void MENU_AcceptSetting(void)
{
	//int32_t        Min;
	//int32_t        Max;
	FREQ_Config_t *pConfig = &gTxVfo->freq_config_RX;

	/*if (!MENU_GetLimits(UI_MENU_GetCurrentMenuId(), &Min, &Max))
	{
		if (settingsCurrentSubMenu < Min) settingsCurrentSubMenu = Min;
		else
		if (settingsCurrentSubMenu > Max) settingsCurrentSubMenu = Max;
	}*/

	switch (UI_MENU_GetCurrentMenuId())
	{
		default:
			return;

		case MENU_SQL:
			gEeprom.SQUELCH_LEVEL = settingsCurrentSubMenu;
			gVfoConfigureMode     = VFO_CONFIGURE;
			break;

		case MENU_STEP:
			gTxVfo->STEP_SETTING = FREQUENCY_GetStepIdxFromSortedIdx(settingsCurrentSubMenu);
			if (IS_FREQ_CHANNEL(gTxVfo->CHANNEL_SAVE))
			{
				gRequestSaveChannel = 1;
				return;
			}
			return;

		/*case MENU_TXP:
			gTxVfo->OUTPUT_POWER = settingsCurrentSubMenu;
			gRequestSaveChannel = 1;
			return;*/

		case MENU_T_DCS:
			pConfig = &gTxVfo->freq_config_TX;

			// Fallthrough

		case MENU_R_DCS: {
			if (settingsCurrentSubMenu == 0) {
				if (pConfig->CodeType == CODE_TYPE_CONTINUOUS_TONE) {
					return;
				}
				pConfig->Code = 0;
				pConfig->CodeType = CODE_TYPE_OFF;
			}
			else if (settingsCurrentSubMenu < 105) {
				pConfig->CodeType = CODE_TYPE_DIGITAL;
				pConfig->Code = settingsCurrentSubMenu - 1;
			}
			else {
				pConfig->CodeType = CODE_TYPE_REVERSE_DIGITAL;
				pConfig->Code = settingsCurrentSubMenu - 105;
			}

			gRequestSaveChannel = 1;
			return;
		}
		case MENU_T_CTCS:
			pConfig = &gTxVfo->freq_config_TX;
			[[fallthrough]];
		case MENU_R_CTCS: {
			if (settingsCurrentSubMenu == 0) {
				if (pConfig->CodeType != CODE_TYPE_CONTINUOUS_TONE) {
					return;
				}
				pConfig->Code     = 0;
				pConfig->CodeType = CODE_TYPE_OFF;
			}
			else {
				pConfig->Code     = settingsCurrentSubMenu - 1;
				pConfig->CodeType = CODE_TYPE_CONTINUOUS_TONE;
			}

			gRequestSaveChannel = 1;
			return;
		}
		case MENU_SFT_D:
			gTxVfo->TX_OFFSET_FREQUENCY_DIRECTION = settingsCurrentSubMenu;
			gRequestSaveChannel                   = 1;
			return;

		case MENU_OFFSET:
			gTxVfo->TX_OFFSET_FREQUENCY = settingsCurrentSubMenu;
			gRequestSaveChannel         = 1;
			return;

		case MENU_W_N:
			gTxVfo->CHANNEL_BANDWIDTH = settingsCurrentSubMenu;
			gRequestSaveChannel       = 1;
			return;

		case MENU_SCR:
			gTxVfo->SCRAMBLING_TYPE = settingsCurrentSubMenu;
			#if 0
				if (settingsCurrentSubMenu > 0 && gSetting_ScrambleEnable)
					BK4819_EnableScramble(settingsCurrentSubMenu - 1);
				else
					BK4819_DisableScramble();
			#endif
			gRequestSaveChannel     = 1;
			return;

		case MENU_BCL:
			gTxVfo->BUSY_CHANNEL_LOCK = settingsCurrentSubMenu;
			gRequestSaveChannel       = 1;
			return;

		case MENU_MEM_CH:
			gTxVfo->CHANNEL_SAVE = settingsCurrentSubMenu;
			#if 0
				gEeprom.MrChannel[0] = settingsCurrentSubMenu;
			#else
				gEeprom.MrChannel[gEeprom.TX_VFO] = settingsCurrentSubMenu;
			#endif
			gRequestSaveChannel = 2;
			gVfoConfigureMode   = VFO_CONFIGURE_RELOAD;
			gFlagResetVfos      = true;
			return;

		case MENU_MEM_NAME:
			for (int i = 9; i >= 0; i--) {
				if (edit[i] != ' ' && edit[i] != '_' && edit[i] != 0x00 && edit[i] != 0xff)
					break;
				edit[i] = ' ';
			}

			SETTINGS_SaveChannelName(settingsCurrentSubMenu, edit);
			return;

		case MENU_SAVE:
			gEeprom.BATTERY_SAVE = settingsCurrentSubMenu;
			break;

		#ifdef ENABLE_VOX
			case MENU_VOX:
				gEeprom.VOX_SWITCH = settingsCurrentSubMenu != 0;
				if (gEeprom.VOX_SWITCH)
					gEeprom.VOX_LEVEL = settingsCurrentSubMenu - 1;
				SETTINGS_LoadCalibration();
				gFlagReconfigureVfos = true;
				gUpdateStatus        = true;
				break;
		#endif

		case MENU_ABR:
			gEeprom.BACKLIGHT_TIME = settingsCurrentSubMenu;
			break;

		case MENU_ABR_MIN:
			gEeprom.BACKLIGHT_MIN = settingsCurrentSubMenu;
			gEeprom.BACKLIGHT_MAX = MAX(settingsCurrentSubMenu + 1 , gEeprom.BACKLIGHT_MAX);
			break;

		case MENU_ABR_MAX:
			gEeprom.BACKLIGHT_MAX = settingsCurrentSubMenu;
			gEeprom.BACKLIGHT_MIN = MIN(settingsCurrentSubMenu - 1, gEeprom.BACKLIGHT_MIN);
			break;			

		case MENU_CONTRAST:
			gEeprom.LCD_CONTRAST = settingsCurrentSubMenu;
			ST7565_SetContrast(gEeprom.LCD_CONTRAST);
			break;

		case MENU_ABR_ON_TX_RX:
			gSetting_backlight_on_tx_rx = settingsCurrentSubMenu;
			break;

		case MENU_TDR:
			gEeprom.DUAL_WATCH = (gEeprom.TX_VFO + 1) * (settingsCurrentSubMenu & 1);
			gEeprom.CROSS_BAND_RX_TX = (gEeprom.TX_VFO + 1) * ((settingsCurrentSubMenu & 2) > 0);

			gFlagReconfigureVfos = true;
			gUpdateStatus        = true;
			break;

		case MENU_BEEP:
			gEeprom.BEEP_CONTROL = settingsCurrentSubMenu;
			break;

		case MENU_TOT:
			gEeprom.TX_TIMEOUT_TIMER = settingsCurrentSubMenu;
			break;

		case MENU_SC_REV:
			gEeprom.SCAN_RESUME_MODE = settingsCurrentSubMenu;
			break;

		/*case MENU_MDF:
			gEeprom.CHANNEL_DISPLAY_MODE = settingsCurrentSubMenu;
			break;*/

		case MENU_AUTOLK:
			gEeprom.AUTO_KEYPAD_LOCK = settingsCurrentSubMenu;
			gKeyLockCountdown        = 30;
			break;

		case MENU_S_ADD1:
			gTxVfo->SCANLIST1_PARTICIPATION = settingsCurrentSubMenu;
			SETTINGS_UpdateChannel(gTxVfo->CHANNEL_SAVE, gTxVfo, true);
			gVfoConfigureMode = VFO_CONFIGURE;
			gFlagResetVfos    = true;
			return;

		case MENU_S_ADD2:
			gTxVfo->SCANLIST2_PARTICIPATION = settingsCurrentSubMenu;
			SETTINGS_UpdateChannel(gTxVfo->CHANNEL_SAVE, gTxVfo, true);
			gVfoConfigureMode = VFO_CONFIGURE;
			gFlagResetVfos    = true;
			return;

		case MENU_STE:
			gEeprom.TAIL_TONE_ELIMINATION = settingsCurrentSubMenu;
			break;

		case MENU_RP_STE:
			gEeprom.REPEATER_TAIL_TONE_ELIMINATION = settingsCurrentSubMenu;
			break;

		case MENU_MIC:
			gEeprom.MIC_SENSITIVITY = settingsCurrentSubMenu;
			SETTINGS_LoadCalibration();
			gFlagReconfigureVfos = true;
			break;

		#ifdef ENABLE_AUDIO_BAR
			case MENU_MIC_BAR:
				gSetting_mic_bar = settingsCurrentSubMenu;
				break;
		#endif

		case MENU_COMPAND:
			gTxVfo->Compander = settingsCurrentSubMenu;
			SETTINGS_UpdateChannel(gTxVfo->CHANNEL_SAVE, gTxVfo, true);
			gVfoConfigureMode = VFO_CONFIGURE;
			gFlagResetVfos    = true;
//			gRequestSaveChannel = 1;
			return;

		case MENU_1_CALL:
			gEeprom.CHAN_1_CALL = settingsCurrentSubMenu;
			break;

		case MENU_S_LIST:
			gEeprom.SCAN_LIST_DEFAULT = settingsCurrentSubMenu;
			break;

		#ifdef ENABLE_ALARM
			case MENU_AL_MOD:
				gEeprom.ALARM_MODE = settingsCurrentSubMenu;
				break;
		#endif

		case MENU_D_ST:
			gEeprom.DTMF_SIDE_TONE = settingsCurrentSubMenu;
			break;

#ifdef ENABLE_DTMF_CALLING
		case MENU_D_RSP:
			gEeprom.DTMF_DECODE_RESPONSE = settingsCurrentSubMenu;
			break;

		case MENU_D_HOLD:
			gEeprom.DTMF_auto_reset_time = settingsCurrentSubMenu;
			break;
#endif
		case MENU_D_PRE:
			gEeprom.DTMF_PRELOAD_TIME = settingsCurrentSubMenu * 10;
			break;

		case MENU_PTT_ID:
			gTxVfo->DTMF_PTT_ID_TX_MODE = settingsCurrentSubMenu;
			gRequestSaveChannel         = 1;
			return;

		case MENU_BAT_TXT:
			gSetting_battery_text = settingsCurrentSubMenu;
			break;

#ifdef ENABLE_DTMF_CALLING
		case MENU_D_DCD:
			gTxVfo->DTMF_DECODING_ENABLE = settingsCurrentSubMenu;
			DTMF_clear_RX();
			gRequestSaveChannel = 1;
			return;
#endif

		case MENU_D_LIVE_DEC:
			gSetting_live_DTMF_decoder = settingsCurrentSubMenu;
			gDTMF_RX_live_timeout = 0;
			memset(gDTMF_RX_live, 0, sizeof(gDTMF_RX_live));
			if (!gSetting_live_DTMF_decoder)
				BK4819_DisableDTMF();
			gFlagReconfigureVfos     = true;
			gUpdateStatus            = true;
			break;

#ifdef ENABLE_DTMF_CALLING
		case MENU_D_LIST:
			gDTMF_chosen_contact = settingsCurrentSubMenu - 1;
			if (gIsDtmfContactValid)
			{
				GUI_SelectNextDisplay(DISPLAY_MAIN);
				gDTMF_InputMode       = true;
				gDTMF_InputBox_Index  = 3;
				memcpy(gDTMF_InputBox, gDTMF_ID, 4);
				gRequestDisplayScreen = DISPLAY_INVALID;
			}
			return;
#endif
		case MENU_PONMSG:
			gEeprom.POWER_ON_DISPLAY_MODE = settingsCurrentSubMenu;
			break;

		case MENU_ROGER:
			gEeprom.ROGER = settingsCurrentSubMenu;
			break;

		case MENU_AM:
			gTxVfo->Modulation     = settingsCurrentSubMenu;
			gRequestSaveChannel = 1;
			return;

		#ifdef ENABLE_AM_FIX
			case MENU_AM_FIX:
				gSetting_AM_fix = settingsCurrentSubMenu;
				gVfoConfigureMode = VFO_CONFIGURE_RELOAD;
				gFlagResetVfos    = true;
				break;
		#endif

		case MENU_DEL_CH:
			SETTINGS_UpdateChannel(settingsCurrentSubMenu, NULL, false);
			gVfoConfigureMode = VFO_CONFIGURE_RELOAD;
			gFlagResetVfos    = true;
			return;

		case MENU_RESET:
			SETTINGS_FactoryReset(settingsCurrentSubMenu);
			return;

		case MENU_350TX:
			gSetting_350TX = settingsCurrentSubMenu;
			break;

		case MENU_F_LOCK: {
			if(settingsCurrentSubMenu == F_LOCK_NONE) { // select 10 times to enable
				gUnlockAllTxConfCnt++;
				if(gUnlockAllTxConfCnt < 10)
					return;
			}
			else
				gUnlockAllTxConfCnt = 0;

			gSetting_F_LOCK = settingsCurrentSubMenu;
			break;
		}
		case MENU_200TX:
			gSetting_200TX = settingsCurrentSubMenu;
			break;

		case MENU_500TX:
			gSetting_500TX = settingsCurrentSubMenu;
			break;

		case MENU_350EN:
			gSetting_350EN       = settingsCurrentSubMenu;
			gVfoConfigureMode    = VFO_CONFIGURE_RELOAD;
			gFlagResetVfos       = true;
			break;

		case MENU_SCREN:
			gSetting_ScrambleEnable = settingsCurrentSubMenu;
			gFlagReconfigureVfos    = true;
			break;

		#ifdef ENABLE_F_CAL_MENU
			case MENU_F_CALI:
				writeXtalFreqCal(settingsCurrentSubMenu, true);
				return;
		#endif

		case MENU_BATCAL:
		{																 // voltages are averages between discharge curves of 1600 and 2200 mAh
			// gBatteryCalibration[0] = (520ul * settingsCurrentSubMenu) / 760;  // 5.20V empty, blinking above this value, reduced functionality below
			// gBatteryCalibration[1] = (689ul * settingsCurrentSubMenu) / 760;  // 6.89V,  ~5%, 1 bars above this value
			// gBatteryCalibration[2] = (724ul * settingsCurrentSubMenu) / 760;  // 7.24V, ~17%, 2 bars above this value
			gBatteryCalibration[3] =          settingsCurrentSubMenu;         // 7.6V,  ~29%, 3 bars above this value
			// gBatteryCalibration[4] = (771ul * settingsCurrentSubMenu) / 760;  // 7.71V, ~65%, 4 bars above this value
			// gBatteryCalibration[5] = 2300;
			SETTINGS_SaveBatteryCalibration(gBatteryCalibration);
			return;
		}

		case MENU_BATTYP:
			gEeprom.BATTERY_TYPE = settingsCurrentSubMenu;
			break;

		case MENU_F1SHRT:
		case MENU_F1LONG:
		case MENU_F2SHRT:
		case MENU_F2LONG:
		case MENU_MLONG:
			{
				uint8_t * fun[]= {
					&gEeprom.KEY_1_SHORT_PRESS_ACTION,
					&gEeprom.KEY_1_LONG_PRESS_ACTION,
					&gEeprom.KEY_2_SHORT_PRESS_ACTION,
					&gEeprom.KEY_2_LONG_PRESS_ACTION,
					&gEeprom.KEY_M_LONG_PRESS_ACTION};
				*fun[UI_MENU_GetCurrentMenuId()-MENU_F1SHRT] = gSubMenu_SIDEFUNCTIONS[settingsCurrentSubMenu].id;
			}
			break;

	}

	gRequestSaveSettings = true;
}
/*
static void MENU_ClampSelection(int8_t Direction)
{
	int32_t Min;
	int32_t Max;

	if (!MENU_GetLimits(UI_MENU_GetCurrentMenuId(), &Min, &Max))
	{
		int32_t Selection = settingsCurrentSubMenu;
		if (Selection < Min) Selection = Min;
		else
		if (Selection > Max) Selection = Max;
		settingsCurrentSubMenu = NUMBER_AddWithWraparound(Selection, Direction, Min, Max);
	}
}
*/
void MENU_ShowCurrentSetting(void)
{
	if (!MENU_GetLimits(UI_MENU_GetCurrentMenuId(), &settingsSubmenuMin, &settingsSubmenuSize))
	{
		if (settingsCurrentSubMenu < settingsSubmenuMin) settingsCurrentSubMenu = settingsSubmenuMin;
		else
		if (settingsCurrentSubMenu > settingsSubmenuSize) settingsCurrentSubMenu = settingsSubmenuSize;
	}

	switch (UI_MENU_GetCurrentMenuId())
	{
		case MENU_SQL:
			settingsCurrentSubMenu = gEeprom.SQUELCH_LEVEL;
			break;

		case MENU_STEP:
			settingsCurrentSubMenu = FREQUENCY_GetSortedIdxFromStepIdx(gTxVfo->STEP_SETTING);
			break;

		/*case MENU_TXP:
			settingsCurrentSubMenu = gTxVfo->OUTPUT_POWER;
			break;*/

		case MENU_RESET:
			settingsCurrentSubMenu = 0;
			break;

		case MENU_R_DCS:
		case MENU_R_CTCS:
		{
			DCS_CodeType_t type = gTxVfo->freq_config_RX.CodeType;
			uint8_t code = gTxVfo->freq_config_RX.Code;
			int menuid = UI_MENU_GetCurrentMenuId();

			if(gScanUseCssResult) {
				gScanUseCssResult = false;
				type = gScanCssResultType;
				code = gScanCssResultCode;
			}
			if((menuid==MENU_R_CTCS) ^ (type==CODE_TYPE_CONTINUOUS_TONE)) { //not the same type
				settingsCurrentSubMenu = 0;
				break;
			}

			switch (type) {
				case CODE_TYPE_CONTINUOUS_TONE:
				case CODE_TYPE_DIGITAL:
					settingsCurrentSubMenu = code + 1;
					break;
				case CODE_TYPE_REVERSE_DIGITAL:
					settingsCurrentSubMenu = code + 105;
					break;
				default:
					settingsCurrentSubMenu = 0;
					break;
			}
		break;
		}

		case MENU_T_DCS:
			switch (gTxVfo->freq_config_TX.CodeType)
			{
				case CODE_TYPE_DIGITAL:
					settingsCurrentSubMenu = gTxVfo->freq_config_TX.Code + 1;
					break;
				case CODE_TYPE_REVERSE_DIGITAL:
					settingsCurrentSubMenu = gTxVfo->freq_config_TX.Code + 105;
					break;
				default:
					settingsCurrentSubMenu = 0;
					break;
			}
			break;

		case MENU_T_CTCS:
			settingsCurrentSubMenu = (gTxVfo->freq_config_TX.CodeType == CODE_TYPE_CONTINUOUS_TONE) ? gTxVfo->freq_config_TX.Code + 1 : 0;
			break;

		case MENU_SFT_D:
			settingsCurrentSubMenu = gTxVfo->TX_OFFSET_FREQUENCY_DIRECTION;
			break;

		case MENU_OFFSET:
			settingsCurrentSubMenu = gTxVfo->TX_OFFSET_FREQUENCY;
			break;

		case MENU_W_N:
			settingsCurrentSubMenu = gTxVfo->CHANNEL_BANDWIDTH;
			break;

		case MENU_SCR:
			settingsCurrentSubMenu = gTxVfo->SCRAMBLING_TYPE;
			break;

		case MENU_BCL:
			settingsCurrentSubMenu = gTxVfo->BUSY_CHANNEL_LOCK;
			break;

		case MENU_MEM_CH:
			#if 0
				settingsCurrentSubMenu = gEeprom.MrChannel[0];
			#else
				settingsCurrentSubMenu = gEeprom.MrChannel[gEeprom.TX_VFO];
			#endif
			break;

		case MENU_MEM_NAME:
			settingsCurrentSubMenu = gEeprom.MrChannel[gEeprom.TX_VFO];
			break;

		case MENU_SAVE:
			settingsCurrentSubMenu = gEeprom.BATTERY_SAVE;
			break;

#ifdef ENABLE_VOX
		case MENU_VOX:
			settingsCurrentSubMenu = gEeprom.VOX_SWITCH ? gEeprom.VOX_LEVEL + 1 : 0;
			break;
#endif

		case MENU_ABR:
			settingsCurrentSubMenu = gEeprom.BACKLIGHT_TIME;
			break;

		case MENU_ABR_MIN:
			settingsCurrentSubMenu = gEeprom.BACKLIGHT_MIN;
			break;

		case MENU_ABR_MAX:
			settingsCurrentSubMenu = gEeprom.BACKLIGHT_MAX;
			break;

		case MENU_CONTRAST:
			settingsCurrentSubMenu = gEeprom.LCD_CONTRAST;
			break;		

		case MENU_ABR_ON_TX_RX:
			settingsCurrentSubMenu = gSetting_backlight_on_tx_rx;
			break;

		case MENU_TDR:
			settingsCurrentSubMenu = (gEeprom.DUAL_WATCH != DUAL_WATCH_OFF) + (gEeprom.CROSS_BAND_RX_TX != CROSS_BAND_OFF) * 2;
			break;

		case MENU_BEEP:
			settingsCurrentSubMenu = gEeprom.BEEP_CONTROL;
			break;

		case MENU_TOT:
			settingsCurrentSubMenu = gEeprom.TX_TIMEOUT_TIMER;
			break;

		case MENU_SC_REV:
			settingsCurrentSubMenu = gEeprom.SCAN_RESUME_MODE;
			break;

		/*case MENU_MDF:
			settingsCurrentSubMenu = gEeprom.CHANNEL_DISPLAY_MODE;
			break;*/

		case MENU_AUTOLK:
			settingsCurrentSubMenu = gEeprom.AUTO_KEYPAD_LOCK;
			break;

		case MENU_S_ADD1:
			settingsCurrentSubMenu = gTxVfo->SCANLIST1_PARTICIPATION;
			break;

		case MENU_S_ADD2:
			settingsCurrentSubMenu = gTxVfo->SCANLIST2_PARTICIPATION;
			break;

		case MENU_STE:
			settingsCurrentSubMenu = gEeprom.TAIL_TONE_ELIMINATION;
			break;

		case MENU_RP_STE:
			settingsCurrentSubMenu = gEeprom.REPEATER_TAIL_TONE_ELIMINATION;
			break;

		case MENU_MIC:
			settingsCurrentSubMenu = gEeprom.MIC_SENSITIVITY;
			break;

#ifdef ENABLE_AUDIO_BAR
		case MENU_MIC_BAR:
			settingsCurrentSubMenu = gSetting_mic_bar;
			break;
#endif

		case MENU_COMPAND:
			settingsCurrentSubMenu = gTxVfo->Compander;
			return;

		case MENU_1_CALL:
			settingsCurrentSubMenu = gEeprom.CHAN_1_CALL;
			break;

		case MENU_S_LIST:
			settingsCurrentSubMenu = gEeprom.SCAN_LIST_DEFAULT;
			break;

		case MENU_SLIST1:
			settingsCurrentSubMenu = RADIO_FindNextChannel(0, 1, true, 0);
			break;

		case MENU_SLIST2:
			settingsCurrentSubMenu = RADIO_FindNextChannel(0, 1, true, 1);
			break;

		#ifdef ENABLE_ALARM
			case MENU_AL_MOD:
				settingsCurrentSubMenu = gEeprom.ALARM_MODE;
				break;
		#endif

		case MENU_D_ST:
			settingsCurrentSubMenu = gEeprom.DTMF_SIDE_TONE;
			break;

#ifdef ENABLE_DTMF_CALLING
		case MENU_D_RSP:
			settingsCurrentSubMenu = gEeprom.DTMF_DECODE_RESPONSE;
			break;

		case MENU_D_HOLD:
			settingsCurrentSubMenu = gEeprom.DTMF_auto_reset_time;
			break;
#endif
		case MENU_D_PRE:
			settingsCurrentSubMenu = gEeprom.DTMF_PRELOAD_TIME / 10;
			break;

		case MENU_PTT_ID:
			settingsCurrentSubMenu = gTxVfo->DTMF_PTT_ID_TX_MODE;
			break;

		case MENU_BAT_TXT:
			settingsCurrentSubMenu = gSetting_battery_text;
			return;

#ifdef ENABLE_DTMF_CALLING
		case MENU_D_DCD:
			settingsCurrentSubMenu = gTxVfo->DTMF_DECODING_ENABLE;
			break;

		case MENU_D_LIST:
			settingsCurrentSubMenu = gDTMF_chosen_contact + 1;
			break;
#endif
		case MENU_D_LIVE_DEC:
			settingsCurrentSubMenu = gSetting_live_DTMF_decoder;
			break;

		case MENU_PONMSG:
			settingsCurrentSubMenu = gEeprom.POWER_ON_DISPLAY_MODE;
			break;

		case MENU_ROGER:
			settingsCurrentSubMenu = gEeprom.ROGER;
			break;

		case MENU_AM:
			settingsCurrentSubMenu = gTxVfo->Modulation;
			break;

#ifdef ENABLE_AM_FIX
		case MENU_AM_FIX:
			settingsCurrentSubMenu = gSetting_AM_fix;
			break;
#endif

		case MENU_DEL_CH:
			#if 0
				settingsCurrentSubMenu = RADIO_FindNextChannel(gEeprom.MrChannel[0], 1, false, 1);
			#else
				settingsCurrentSubMenu = RADIO_FindNextChannel(gEeprom.MrChannel[gEeprom.TX_VFO], 1, false, 1);
			#endif
			break;

		case MENU_350TX:
			settingsCurrentSubMenu = gSetting_350TX;
			break;

		case MENU_F_LOCK:
			settingsCurrentSubMenu = gSetting_F_LOCK;
			break;

		case MENU_200TX:
			settingsCurrentSubMenu = gSetting_200TX;
			break;

		case MENU_500TX:
			settingsCurrentSubMenu = gSetting_500TX;
			break;

		case MENU_350EN:
			settingsCurrentSubMenu = gSetting_350EN;
			break;

		case MENU_SCREN:
			settingsCurrentSubMenu = gSetting_ScrambleEnable;
			break;

		#ifdef ENABLE_F_CAL_MENU
			case MENU_F_CALI:
				settingsCurrentSubMenu = gEeprom.BK4819_XTAL_FREQ_LOW;
				break;
		#endif

		case MENU_BATCAL:
			settingsCurrentSubMenu = gBatteryCalibration[3];
			break;

		case MENU_BATTYP:
			settingsCurrentSubMenu = gEeprom.BATTERY_TYPE;
			break;

		case MENU_F1SHRT:
		case MENU_F1LONG:
		case MENU_F2SHRT:
		case MENU_F2LONG:
		case MENU_MLONG:
		{
			uint8_t * fun[]= {
				&gEeprom.KEY_1_SHORT_PRESS_ACTION,
				&gEeprom.KEY_1_LONG_PRESS_ACTION,
				&gEeprom.KEY_2_SHORT_PRESS_ACTION,
				&gEeprom.KEY_2_LONG_PRESS_ACTION,
				&gEeprom.KEY_M_LONG_PRESS_ACTION};
			uint8_t id = *fun[UI_MENU_GetCurrentMenuId()-MENU_F1SHRT];

			for(int i = 0; i < gSubMenu_SIDEFUNCTIONS_size; i++) {
				if(gSubMenu_SIDEFUNCTIONS[i].id==id) {
					settingsCurrentSubMenu = i;
					break;
				}

			}
			break;
		}

		default:
			return;
	}
}

static void MENU_Key_0_to_9(KEY_Code_t Key, bool bKeyPressed, bool bKeyHeld)
{
	uint8_t  Offset;
	uint16_t  Min;
	uint16_t  Max;
	uint16_t Value = 0;

	if (bKeyHeld || !bKeyPressed)
		return;

	gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;

	if (UI_MENU_GetCurrentMenuId() == MENU_MEM_NAME && edit_index >= 0)
	{	// currently editing the channel name

		if (edit_index < 10)
		{
			if (Key <= KEY_9)
			{
				edit[edit_index] = '0' + Key - KEY_0;

				if (++edit_index >= 10)
				{	// exit edit
					gFlagAcceptSetting  = false;
					gAskForConfirmation = 1;
				}

				gRequestDisplayScreen = DISPLAY_MENU;
			}
		}

		return;
	}

	INPUTBOX_Append(Key);

	gRequestDisplayScreen = DISPLAY_MENU;

	if (!settingsSubMenuActive)
	{
		switch (gInputBoxIndex)
		{
			case 2:
				gInputBoxIndex = 0;

				Value = (gInputBox[0] * 10) + gInputBox[1];

				if (Value > 0 && Value <= gMenuListCount)
				{
					settingsCurrentMenu         = Value - 1;
					gFlagRefreshSetting = true;
					return;
				}

				if (Value <= gMenuListCount)
					break;

				gInputBox[0]   = gInputBox[1];
				gInputBoxIndex = 1;
				[[fallthrough]];
			case 1:
				Value = gInputBox[0];
				if (Value > 0 && Value <= gMenuListCount)
				{
					settingsCurrentMenu         = Value - 1;
					gFlagRefreshSetting = true;
					return;
				}
				break;
		}

		gInputBoxIndex = 0;

		gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
		return;
	}

	if (UI_MENU_GetCurrentMenuId() == MENU_OFFSET)
	{
		uint32_t Frequency;

		if (gInputBoxIndex < 6)
		{	// invalid frequency
			return;
		}

		Frequency = StrToUL(INPUTBOX_GetAscii())*100;
		settingsCurrentSubMenu = FREQUENCY_RoundToStep(Frequency, gTxVfo->StepFrequency);

		gInputBoxIndex = 0;
		return;
	}

	if (UI_MENU_GetCurrentMenuId() == MENU_MEM_CH ||
		UI_MENU_GetCurrentMenuId() == MENU_DEL_CH ||
		UI_MENU_GetCurrentMenuId() == MENU_1_CALL ||
		UI_MENU_GetCurrentMenuId() == MENU_MEM_NAME)
	{	// enter 3-digit channel number

		if (gInputBoxIndex < 3)
		{
			gRequestDisplayScreen = DISPLAY_MENU;
			return;
		}

		gInputBoxIndex = 0;

		Value = ((gInputBox[0] * 100) + (gInputBox[1] * 10) + gInputBox[2]);

		if (IS_MR_CHANNEL(Value))
		{
			settingsCurrentSubMenu = Value;
			return;
		}

		gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
		return;
	}

	if (MENU_GetLimits(UI_MENU_GetCurrentMenuId(), &Min, &Max))
	{
		gInputBoxIndex = 0;
		gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
		return;
	}

	Offset = (Max >= 100) ? 3 : (Max >= 10) ? 2 : 1;

	switch (gInputBoxIndex)
	{
		case 1:
			Value = gInputBox[0];
			break;
		case 2:
			Value = (gInputBox[0] *  10) + gInputBox[1];
			break;
		case 3:
			Value = (gInputBox[0] * 100) + (gInputBox[1] * 10) + gInputBox[2];
			break;
	}

	if (Offset == gInputBoxIndex)
		gInputBoxIndex = 0;

	if (Value <= Max)
	{
		settingsCurrentSubMenu = Value;
		return;
	}

	gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
}


static void MENU_Key_EXIT(bool bKeyPressed, bool bKeyHeld)
{
	if (bKeyHeld || !bKeyPressed)
		return;

	gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;

	if ( settingsSubMenuActive ) {	
		settingsShowSubMenu = false;
		settingsSubMenuActive = false;
		gRequestDisplayScreen = DISPLAY_MENU;
		return;
	}
	settingsShowSubMenu = false;
	if (!gCssBackgroundScan)
	{
		/* Backlight related menus set full brightness. Set it back to the configured value,
		   just in case we are exiting from one of them. */
		BACKLIGHT_TurnOn();

		if (settingsSubMenuActive)
		{
			if (gInputBoxIndex == 0 || UI_MENU_GetCurrentMenuId() != MENU_OFFSET)
			{
				gAskForConfirmation = 0;
				settingsSubMenuActive = false;
				gInputBoxIndex      = 0;
				gFlagRefreshSetting = true;

			}
			else
				gInputBox[--gInputBoxIndex] = 10;

			// ***********************

			gRequestDisplayScreen = DISPLAY_MENU;
			return;
		}

		gRequestDisplayScreen = DISPLAY_MAIN;

		if (gEeprom.BACKLIGHT_TIME == 0) // backlight set to always off
		{
			BACKLIGHT_TurnOff();	// turn the backlight OFF
		}
	}
	else
	{
		//MENU_StopCssScan();
		gRequestDisplayScreen = DISPLAY_MENU;
	}

	gPttWasReleased = true;
}

static void MENU_Key_MENU(const bool bKeyPressed, const bool bKeyHeld)
{
	if (bKeyHeld || !bKeyPressed)
		return;

	gBeepToPlay           = BEEP_1KHZ_60MS_OPTIONAL;
	gRequestDisplayScreen = DISPLAY_MENU;
	//gUpdateDisplay = true;

	if ( settingsSubMenuActive ) {
		gFlagAcceptSetting = true;
		settingsSubMenuActive = false;
	} else {
		settingsSubMenuActive = true;
		settingsShowSubMenu = true;
		settingsCurrentSubMenu = 0;
		MENU_ShowCurrentSetting();
	}
	/*
	if (!gIsInSubMenu)
	{
		#ifdef ENABLE_DTMF_CALLING
        	if (UI_MENU_GetCurrentMenuId() == MENU_ANI_ID || UI_MENU_GetCurrentMenuId() == MENU_UPCODE|| UI_MENU_GetCurrentMenuId() == MENU_DWCODE)
		#else
			if (UI_MENU_GetCurrentMenuId() == MENU_UPCODE|| UI_MENU_GetCurrentMenuId() == MENU_DWCODE)
		#endif
            return;
		#if 1
			if (UI_MENU_GetCurrentMenuId() == MENU_DEL_CH || UI_MENU_GetCurrentMenuId() == MENU_MEM_NAME)
				if (!RADIO_CheckValidChannel(settingsCurrentSubMenu, false, 0))
					return;  // invalid channel
		#endif

		gAskForConfirmation = 0;
		gIsInSubMenu        = true;

//		if (UI_MENU_GetCurrentMenuId() != MENU_D_LIST)
		{
			gInputBoxIndex      = 0;
			edit_index          = -1;
		}

		return;
	}

	if (UI_MENU_GetCurrentMenuId() == MENU_MEM_NAME)
	{
		if (edit_index < 0)
		{	// enter channel name edit mode
			if (!RADIO_CheckValidChannel(settingsCurrentSubMenu, false, 0))
				return;

			SETTINGS_FetchChannelName(edit, settingsCurrentSubMenu);

			// pad the channel name out with '_'
			edit_index = strlen(edit);
			while (edit_index < 10)
				edit[edit_index++] = '_';
			edit[edit_index] = 0;
			edit_index = 0;  // 'edit_index' is going to be used as the cursor position

			// make a copy so we can test for change when exiting the menu item
			memcpy(edit_original, edit, sizeof(edit_original));

			return;
		}
		else
		if (edit_index >= 0 && edit_index < 10)
		{	// editing the channel name characters

			if (++edit_index < 10)
				return;	// next char

			// exit
			gFlagAcceptSetting  = false;
			gAskForConfirmation = 0;
			if (memcmp(edit_original, edit, sizeof(edit_original)) == 0) {
				// no change - drop it
				gIsInSubMenu = false;
			}
		}
	}

	// exiting the sub menu

	if (gIsInSubMenu)
	{
		if (UI_MENU_GetCurrentMenuId() == MENU_RESET  ||
			UI_MENU_GetCurrentMenuId() == MENU_MEM_CH ||
			UI_MENU_GetCurrentMenuId() == MENU_DEL_CH ||
			UI_MENU_GetCurrentMenuId() == MENU_MEM_NAME)
		{
			switch (gAskForConfirmation)
			{
				case 0:
					gAskForConfirmation = 1;
					break;

				case 1:
					gAskForConfirmation = 2;

					UI_DisplayMenu();

					if (UI_MENU_GetCurrentMenuId() == MENU_RESET)
					{

						MENU_AcceptSetting();

						#if defined(ENABLE_OVERLAY)
							overlay_FLASH_RebootToBootloader();
						#else
							NVIC_SystemReset();
						#endif
					}

					gFlagAcceptSetting  = true;
					gIsInSubMenu        = false;
					gAskForConfirmation = 0;
			}
		}
		else
		{
			gFlagAcceptSetting = true;
			gIsInSubMenu       = false;
		}
	}

	SCANNER_Stop();

	gInputBoxIndex = 0;
	*/
}
/*
static void MENU_Key_STAR(const bool bKeyPressed, const bool bKeyHeld)
{
	if (bKeyHeld || !bKeyPressed)
		return;

	gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;

	if (UI_MENU_GetCurrentMenuId() == MENU_MEM_NAME && edit_index >= 0)
	{	// currently editing the channel name

		if (edit_index < 10)
		{
			edit[edit_index] = '-';

			if (++edit_index >= 10)
			{	// exit edit
				gFlagAcceptSetting  = false;
				gAskForConfirmation = 1;
			}

			gRequestDisplayScreen = DISPLAY_MENU;
		}

		return;
	}

	RADIO_SelectVfos();
	if (gRxVfo->Modulation ==  MODULATION_FM)
	{
		if ((UI_MENU_GetCurrentMenuId() == MENU_R_CTCS || UI_MENU_GetCurrentMenuId() == MENU_R_DCS) && gIsInSubMenu)
		{	// scan CTCSS or DCS to find the tone/code of the incoming signal
			if (!SCANNER_IsScanning())
				MENU_StartCssScan();
			else
				MENU_StopCssScan();
		}

		gPttWasReleased = true;
		return;
	}

	gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
}
*/
static void MENU_Key_UP_DOWN(bool bKeyPressed, bool bKeyHeld, int8_t Direction)
{

	if (!bKeyHeld)
	{
		if (!bKeyPressed)
			return;
		gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;		
	} else if (!bKeyPressed) {
		return;
	}

	if ( settingsSubMenuActive ) {
		settingsCurrentSubMenu = NUMBER_AddWithWraparound(settingsCurrentSubMenu, -Direction, settingsSubmenuMin, settingsSubmenuSize - 1);
	} else {
		settingsCurrentMenu = NUMBER_AddWithWraparound(settingsCurrentMenu, -Direction, 0, gMenuListCount - 1);
		settingsSubMenuTime = xTaskGetTickCount();
		settingsShowSubMenu = false;
	}
	gRequestDisplayScreen = DISPLAY_MENU;
	//gUpdateDisplay = true;

	/*uint8_t VFO;
	uint8_t Channel;
	bool    bCheckScanList;

	if (UI_MENU_GetCurrentMenuId() == MENU_MEM_NAME && gIsInSubMenu && edit_index >= 0)
	{	// change the character
		if (bKeyPressed && edit_index < 10 && Direction != 0)
		{
			const char   unwanted[] = "$%&!\"':;?^`|{}";
			char         c          = edit[edit_index] + Direction;
			unsigned int i          = 0;
			while (i < sizeof(unwanted) && c >= 32 && c <= 126)
			{
				if (c == unwanted[i++])
				{	// choose next character
					c += Direction;
					i = 0;
				}
			}
			edit[edit_index] = (c < 32) ? 126 : (c > 126) ? 32 : c;

			gRequestDisplayScreen = DISPLAY_MENU;
		}
		return;
	}

	if (!bKeyHeld)
	{
		if (!bKeyPressed)
			return;

		gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;

		gInputBoxIndex = 0;
	}
	else
	if (!bKeyPressed)
		return;

	if (SCANNER_IsScanning()) {
		return;
	}

	if (!gIsInSubMenu)
	{
		gMenuCursor = NUMBER_AddWithWraparound(gMenuCursor, -Direction, 0, gMenuListCount - 1);

		gFlagRefreshSetting = true;

		gRequestDisplayScreen = DISPLAY_MENU;

		if (UI_MENU_GetCurrentMenuId() != MENU_ABR
			&& UI_MENU_GetCurrentMenuId() != MENU_ABR_MIN
			&& UI_MENU_GetCurrentMenuId() != MENU_ABR_MAX
			&& gEeprom.BACKLIGHT_TIME == 0) // backlight always off and not in the backlight menu
		{
			BACKLIGHT_TurnOff();
		}

		return;
	}

	if (UI_MENU_GetCurrentMenuId() == MENU_OFFSET)
	{
		int32_t Offset = (Direction * gTxVfo->StepFrequency) + settingsCurrentSubMenu;
		if (Offset < 99999990)
		{
			if (Offset < 0)
				Offset = 99999990;
		}
		else
			Offset = 0;

		settingsCurrentSubMenu     = FREQUENCY_RoundToStep(Offset, gTxVfo->StepFrequency);
		gRequestDisplayScreen = DISPLAY_MENU;
		return;
	}

	VFO = 0;

	switch (UI_MENU_GetCurrentMenuId())
	{
		case MENU_DEL_CH:
		case MENU_1_CALL:
		case MENU_MEM_NAME:
			bCheckScanList = false;
			break;

		case MENU_SLIST2:
			VFO = 1;
			[[fallthrough]];
		case MENU_SLIST1:
			bCheckScanList = true;
			break;

		default:
			MENU_ClampSelection(Direction);
			gRequestDisplayScreen = DISPLAY_MENU;
			return;
	}

	Channel = RADIO_FindNextChannel(settingsCurrentSubMenu + Direction, Direction, bCheckScanList, VFO);
	if (Channel != 0xFF)
		settingsCurrentSubMenu = Channel;

	gRequestDisplayScreen = DISPLAY_MENU;*/
}

void MENU_ProcessKeys(KEY_Code_t Key, bool bKeyPressed, bool bKeyHeld)
{
	switch (Key)
	{
		case KEY_0:
		case KEY_1:
		case KEY_2:
		case KEY_3:
		case KEY_4:
		case KEY_5:
		case KEY_6:
		case KEY_7:
		case KEY_8:
		case KEY_9:
			MENU_Key_0_to_9(Key, bKeyPressed, bKeyHeld);
			break;
		case KEY_MENU:
			MENU_Key_MENU(bKeyPressed, bKeyHeld);
			break;
		case KEY_UP:
			MENU_Key_UP_DOWN(bKeyPressed, bKeyHeld,  1);
			break;
		case KEY_DOWN:
			MENU_Key_UP_DOWN(bKeyPressed, bKeyHeld, -1);
			break;
		case KEY_EXIT:
			MENU_Key_EXIT(bKeyPressed, bKeyHeld);
			break;
		case KEY_STAR:
			//MENU_Key_STAR(bKeyPressed, bKeyHeld);
			break;
		case KEY_F:
			/*if (UI_MENU_GetCurrentMenuId() == MENU_MEM_NAME && edit_index >= 0)
			{	// currently editing the channel name
				if (!bKeyHeld && bKeyPressed)
				{
					gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;
					if (edit_index < 10)
					{
						edit[edit_index] = ' ';
						if (++edit_index >= 10)
						{	// exit edit
							gFlagAcceptSetting  = false;
							gAskForConfirmation = 1;
						}
						gRequestDisplayScreen = DISPLAY_MENU;
					}
				}
				break;
			}*/

			GENERIC_Key_F(bKeyPressed, bKeyHeld);
			break;
		case KEY_PTT:
			GENERIC_Key_PTT(bKeyPressed);
			break;
		default:
			if (!bKeyHeld && bKeyPressed)
				gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
			break;
	}

	if (gScreenToDisplay == DISPLAY_MENU)
	{
		/*if (UI_MENU_GetCurrentMenuId() == MENU_VOL ||
			#ifdef ENABLE_F_CAL_MENU
				UI_MENU_GetCurrentMenuId() == MENU_F_CALI ||
		    #endif
			UI_MENU_GetCurrentMenuId() == MENU_BATCAL)
		{
			gMenuCountdown = menu_timeout_long_500ms;
		}
		else
		{
			gMenuCountdown = menu_timeout_500ms;
		}*/
		if ( settingsSubMenuActive ) {
			gMenuCountdown = menu_timeout_long_500ms;
		} else {
			gMenuCountdown = menu_timeout_500ms;
		}		
	}
}
