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

#include "app/dtmf.h"
#ifdef ENABLE_FMRADIO
	#include "app/fm.h"
#endif
#include "driver/bk1080.h"
#include "driver/bk4819.h"
#include "driver/eeprom.h"
#include "misc.h"
#include "settings.h"


Settings gSettings;

VFO_Info_t	gVfoInfo[2];
uint8_t		gScreenChannel[2]; // current channels set in the radio (memory or frequency channels)
uint8_t		gFreqChannel[2]; // last frequency channels used
uint8_t		gMrChannel[2]; // last memory channels used


/*static const uint32_t gDefaultFrequencyTable[] =
{
	14500000,    //
	14550000,    //
	43300000,    //
	43320000,    //
	43350000     //
};*/

EEPROM_Config_t gEeprom = { 0 };

void SETTINGS_InitEEPROM(void) {

#if 1
	EEPROM_ReadBuffer(SETTINGS_OFFSET, &gSettings, SETTINGS_SIZE);
	if ( gSettings.settingsVersion != SETTINGS_VERSION ) {
        gSettings.backlight = 0x9;
		gSettings.backlightTime = 0x6;
    }

	// 0E80..0E87
	//EEPROM_ReadBuffer(0x0E80, Data, 8);
	gScreenChannel[0]   = /*IS_VALID_CHANNEL(Data[0]) ? Data[0] : */(FREQ_CHANNEL_FIRST + BAND6_400MHz);
	gScreenChannel[1]   = /*IS_VALID_CHANNEL(Data[3]) ? Data[3] : */(FREQ_CHANNEL_FIRST + BAND6_400MHz);
	gMrChannel[0]       = /*IS_MR_CHANNEL(Data[1])    ? Data[1] : */MR_CHANNEL_FIRST;
	gMrChannel[1]       = /*IS_MR_CHANNEL(Data[4])    ? Data[4] : */MR_CHANNEL_FIRST;
	gFreqChannel[0]     = /*IS_FREQ_CHANNEL(Data[2])  ? Data[2] : */(FREQ_CHANNEL_FIRST + BAND6_400MHz);
	gFreqChannel[1]     = /*IS_FREQ_CHANNEL(Data[5])  ? Data[5] : */(FREQ_CHANNEL_FIRST + BAND6_400MHz);

#else
	uint8_t Data[16] = {0};
	// 0E70..0E77
	EEPROM_ReadBuffer(0x0E70, Data, 8);
	gSettings.CHAN_1_CALL          = IS_MR_CHANNEL(Data[0]) ? Data[0] : MR_CHANNEL_FIRST;
	gSettings.squelch        = (Data[1] < 10) ? Data[1] : 1;
	gSettings.txTime     = (Data[2] < 11) ? Data[2] : 1;

	gSettings.KEY_LOCK             = (Data[4] <  2) ? Data[4] : false;
	#ifdef ENABLE_VOX
		gSettings.VOX_SWITCH       = (Data[5] <  2) ? Data[5] : false;
		gSettings.VOX_LEVEL        = (Data[6] < 10) ? Data[6] : 1;
	#endif
	gSettings.micGain      = (Data[7] <  5) ? Data[7] : 4;

	// 0E78..0E7F
	EEPROM_ReadBuffer(0x0E78, Data, 8);
	gSettings.BACKLIGHT_MAX 		  = (Data[0] & 0xF) <= 10 ? (Data[0] & 0xF) : 10;
	gSettings.BACKLIGHT_MIN 		  = (Data[0] >> 4) < gSettings.BACKLIGHT_MAX ? (Data[0] >> 4) : 0;
#ifdef ENABLE_BLMIN_TMP_OFF
	gSettings.BACKLIGHT_MIN_STAT	  = BLMIN_STAT_ON;
#endif
	gSettings.CHANNEL_DISPLAY_MODE  = (Data[1] < 4) ? Data[1] : MDF_FREQUENCY;    // 4 instead of 3 - extra display mode
	gSettings.CROSS_BAND_RX_TX      = (Data[2] < 3) ? Data[2] : CROSS_BAND_OFF;
	gSettings.batSave          = (Data[3] < 5) ? Data[3] : 4;
	gSettings.DUAL_WATCH            = (Data[4] < 3) ? Data[4] : DUAL_WATCH_CHAN_A;
	gSettings.backlightTime        = (Data[5] < ARRAY_SIZE(gSubMenu_BACKLIGHT)) ? Data[5] : 3;

	//gSettings.ste = (Data[6] < 2) ? Data[6] : false;
	gSettings.VFO_OPEN              = (Data[7] < 2) ? Data[7] : true;

	gSettings.ste = (Data[6] >> 7) & 1;

	gSettings.contrast		  = Data[6] & 0x7F;


	// 0E80..0E87
	EEPROM_ReadBuffer(0x0E80, Data, 8);
	gScreenChannel[0]   = IS_VALID_CHANNEL(Data[0]) ? Data[0] : (FREQ_CHANNEL_FIRST + BAND6_400MHz);
	gScreenChannel[1]   = IS_VALID_CHANNEL(Data[3]) ? Data[3] : (FREQ_CHANNEL_FIRST + BAND6_400MHz);
	gMrChannel[0]       = IS_MR_CHANNEL(Data[1])    ? Data[1] : MR_CHANNEL_FIRST;
	gMrChannel[1]       = IS_MR_CHANNEL(Data[4])    ? Data[4] : MR_CHANNEL_FIRST;
	gFreqChannel[0]     = IS_FREQ_CHANNEL(Data[2])  ? Data[2] : (FREQ_CHANNEL_FIRST + BAND6_400MHz);
	gFreqChannel[1]     = IS_FREQ_CHANNEL(Data[5])  ? Data[5] : (FREQ_CHANNEL_FIRST + BAND6_400MHz);

#ifdef ENABLE_FMRADIO
	{	// 0E88..0E8F
		struct
		{
			uint16_t selFreq;
			uint8_t  selChn;
			uint8_t  isMrMode:1;
			uint8_t  band:2;
			//uint8_t  space:2;
		} __attribute__((packed)) fmCfg;
		EEPROM_ReadBuffer(0x0E88, &fmCfg, 4);

		gSettings.FM_Band = fmCfg.band;
		//gSettings.FM_Space = fmCfg.space;
		gSettings.FM_SelectedFrequency =
			(fmCfg.selFreq >= BK1080_GetFreqLoLimit(gSettings.FM_Band) && fmCfg.selFreq <= BK1080_GetFreqHiLimit(gSettings.FM_Band)) ?
				fmCfg.selFreq : BK1080_GetFreqLoLimit(gSettings.FM_Band);

		gSettings.FM_SelectedChannel = fmCfg.selChn;
		gSettings.FM_IsMrMode        = fmCfg.isMrMode;
	}

	// 0E40..0E67
	EEPROM_ReadBuffer(0x0E40, gFM_Channels, sizeof(gFM_Channels));
	FM_ConfigureChannelState();
#endif

	// 0E90..0E97
	EEPROM_ReadBuffer(0x0E90, Data, 8);
	gSettings.beep                 = Data[0] & 1;
	gSettings.KEY_M_LONG_PRESS_ACTION      = ((Data[0] >> 1) < ACTION_OPT_LEN) ? (Data[0] >> 1) : ACTION_OPT_NONE;
	gSettings.KEY_1_SHORT_PRESS_ACTION     = (Data[1] < ACTION_OPT_LEN) ? Data[1] : ACTION_OPT_MONITOR;
	gSettings.KEY_1_LONG_PRESS_ACTION      = (Data[2] < ACTION_OPT_LEN) ? Data[2] : ACTION_OPT_NONE;
	gSettings.KEY_2_SHORT_PRESS_ACTION     = (Data[3] < ACTION_OPT_LEN) ? Data[3] : ACTION_OPT_SCAN;
	gSettings.KEY_2_LONG_PRESS_ACTION      = (Data[4] < ACTION_OPT_LEN) ? Data[4] : ACTION_OPT_NONE;
	gSettings.SCAN_RESUME_MODE             = (Data[5] < 3)              ? Data[5] : SCAN_RESUME_CO;
	gSettings.AUTO_KEYPAD_LOCK             = (Data[6] < 2)              ? Data[6] : false;
	gSettings.POWER_ON_DISPLAY_MODE        = (Data[7] < 4)              ? Data[7] : POWER_ON_DISPLAY_MODE_VOLTAGE;

	// 0E98..0E9F
	EEPROM_ReadBuffer(0x0E98, Data, 8);
	memcpy(&gSettings.POWER_ON_PASSWORD, Data, 4);

	// 0EA0..0EA7
	EEPROM_ReadBuffer(0x0EA0, Data, 8);

	if((Data[1] < 200 && Data[1] > 90) && (Data[2] < Data[1]-9 && Data[1] < 160  && Data[2] > 50)) {
		gSettings.S0_LEVEL = Data[1];
		gSettings.S9_LEVEL = Data[2];
	}

	// 0EA8..0EAF
	EEPROM_ReadBuffer(0x0EA8, Data, 8);
	#ifdef ENABLE_ALARM
		gSettings.ALARM_MODE                 = (Data[0] <  2) ? Data[0] : true;
	#endif
	gSettings.roger                          = (Data[1] <  3) ? Data[1] : ROGER_MODE_OFF;
	gSettings.repeaterSte = (Data[2] < 11) ? Data[2] : 0;
	gSettings.activeVFO                         = (Data[3] <  2) ? Data[3] : 0;
	gSettings.batteryType                   = (Data[4] < BATTERY_TYPE_UNKNOWN) ? Data[4] : BATTERY_TYPE_1600_MAH;

	// 0ED0..0ED7
	EEPROM_ReadBuffer(0x0ED0, Data, 8);
	gSettings.DTMF_SIDE_TONE               = (Data[0] <   2) ? Data[0] : true;

/*
#ifdef ENABLE_DTMF_CALLING
	gSettings.DTMF_SEPARATE_CODE           = DTMF_ValidateCodes((char *)(Data + 1), 1) ? Data[1] : '*';
	gSettings.DTMF_GROUP_CALL_CODE         = DTMF_ValidateCodes((char *)(Data + 2), 1) ? Data[2] : '#';
	gSettings.DTMF_DECODE_RESPONSE         = (Data[3] <   4) ? Data[3] : 0;
	gSettings.DTMF_auto_reset_time         = (Data[4] <  61) ? Data[4] : (Data[4] >= 5) ? Data[4] : 10;
#endif
*/
	gSettings.DTMF_PRELOAD_TIME            = (Data[5] < 101) ? Data[5] * 10 : 300;
	gSettings.DTMF_FIRST_CODE_PERSIST_TIME = (Data[6] < 101) ? Data[6] * 10 : 100;
	gSettings.DTMF_HASH_CODE_PERSIST_TIME  = (Data[7] < 101) ? Data[7] * 10 : 100;

	// 0ED8..0EDF
	EEPROM_ReadBuffer(0x0ED8, Data, 8);
	gSettings.DTMF_CODE_PERSIST_TIME  = (Data[0] < 101) ? Data[0] * 10 : 100;
	gSettings.DTMF_CODE_INTERVAL_TIME = (Data[1] < 101) ? Data[1] * 10 : 100;
/*
#ifdef ENABLE_DTMF_CALLING
	gSettings.PERMIT_REMOTE_KILL      = (Data[2] <   2) ? Data[2] : true;

	// 0EE0..0EE7

	EEPROM_ReadBuffer(0x0EE0, Data, sizeof(gSettings.ANI_DTMF_ID));
	if (DTMF_ValidateCodes((char *)Data, sizeof(gSettings.ANI_DTMF_ID))) {
		memcpy(gSettings.ANI_DTMF_ID, Data, sizeof(gSettings.ANI_DTMF_ID));
	} else {
		strcpy(gSettings.ANI_DTMF_ID, "123");
	}


	// 0EE8..0EEF
	EEPROM_ReadBuffer(0x0EE8, Data, sizeof(gSettings.KILL_CODE));
	if (DTMF_ValidateCodes((char *)Data, sizeof(gSettings.KILL_CODE))) {
		memcpy(gSettings.KILL_CODE, Data, sizeof(gSettings.KILL_CODE));
	} else {
		strcpy(gSettings.KILL_CODE, "ABCD9");
	}

	// 0EF0..0EF7
	EEPROM_ReadBuffer(0x0EF0, Data, sizeof(gSettings.REVIVE_CODE));
	if (DTMF_ValidateCodes((char *)Data, sizeof(gSettings.REVIVE_CODE))) {
		memcpy(gSettings.REVIVE_CODE, Data, sizeof(gSettings.REVIVE_CODE));
	} else {
		strcpy(gSettings.REVIVE_CODE, "9DCBA");
	}
#endif
*/
	// 0EF8..0F07
	/*EEPROM_ReadBuffer(0x0EF8, Data, sizeof(gSettings.DTMF_UP_CODE));
	if (DTMF_ValidateCodes((char *)Data, sizeof(gSettings.DTMF_UP_CODE))) {
		memcpy(gSettings.DTMF_UP_CODE, Data, sizeof(gSettings.DTMF_UP_CODE));
	} else {
		strcpy(gSettings.DTMF_UP_CODE, "12345");
	}

	// 0F08..0F17
	EEPROM_ReadBuffer(0x0F08, Data, sizeof(gSettings.DTMF_DOWN_CODE));
	if (DTMF_ValidateCodes((char *)Data, sizeof(gSettings.DTMF_DOWN_CODE))) {
		memcpy(gSettings.DTMF_DOWN_CODE, Data, sizeof(gSettings.DTMF_DOWN_CODE));
	} else {
		strcpy(gSettings.DTMF_DOWN_CODE, "54321");
	}*/

	strcpy(gSettings.DTMF_UP_CODE, "12345");
	strcpy(gSettings.DTMF_DOWN_CODE, "54321");

	// 0F18..0F1F
	EEPROM_ReadBuffer(0x0F18, Data, 8);
	gSettings.SCAN_LIST_DEFAULT = (Data[0] < 3) ? Data[0] : 0;  // we now have 'all' channel scan option
	for (unsigned int i = 0; i < 2; i++)
	{
		const unsigned int j = 1 + (i * 3);
		gSettings.SCAN_LIST_ENABLED[i]     = (Data[j + 0] < 2) ? Data[j] : false;
		gSettings.SCANLIST_PRIORITY_CH1[i] =  Data[j + 1];
		gSettings.SCANLIST_PRIORITY_CH2[i] =  Data[j + 2];
	}

	// 0F40..0F47
	EEPROM_ReadBuffer(0x0F40, Data, 8);
	gSetting_F_LOCK            = (Data[0] < F_LOCK_LEN) ? Data[0] : F_LOCK_DEF;
	gSetting_350TX             = (Data[1] < 2) ? Data[1] : false;  // was true
#ifdef ENABLE_DTMF_CALLING
	gSetting_KILLED            = (Data[2] < 2) ? Data[2] : false;
#endif
	gSetting_200TX             = (Data[3] < 2) ? Data[3] : false;
	gSetting_500TX             = (Data[4] < 2) ? Data[4] : false;
	gSetting_350EN             = (Data[5] < 2) ? Data[5] : true;

	gSetting_ScrambleEnable    = (Data[6] < 2) ? Data[6] : true;

	//gSetting_TX_EN             = (Data[7] & (1u << 0)) ? true : false;
	gSetting_live_DTMF_decoder = !!(Data[7] & (1u << 1));
	gSetting_battery_text      = (((Data[7] >> 2) & 3u) <= 2) ? (Data[7] >> 2) & 3 : 2;

	gSetting_backlight_on_tx_rx = (Data[7] >> 6) & 3u;

	if (!gSettings.VFO_OPEN)
	{
		gScreenChannel[0] = gMrChannel[0];
		gScreenChannel[1] = gMrChannel[1];
	}

	// 0D60..0E27
	EEPROM_ReadBuffer(0x0D60, gMR_ChannelAttributes, sizeof(gMR_ChannelAttributes));
	for(uint16_t i = 0; i < sizeof(gMR_ChannelAttributes); i++) {
		ChannelAttributes_t *att = &gMR_ChannelAttributes[i];
		if(att->__val == 0xff){
			att->__val = 0;
			att->band = 0xf;
		}
	}

	// 0F30..0F3F
	EEPROM_ReadBuffer(0x0F30, gCustomAesKey, sizeof(gCustomAesKey));
	bHasCustomAesKey = false;
	for (unsigned int i = 0; i < ARRAY_SIZE(gCustomAesKey); i++)
	{
		if (gCustomAesKey[i] != 0xFFFFFFFFu)
		{
			bHasCustomAesKey = true;
			return;
		}
	}
#endif
}

void SETTINGS_LoadCalibration(void)
{
//	uint8_t Mic;

	EEPROM_ReadBuffer(0x1EC0, gEEPROM_RSSI_CALIB[3], 8);
	memcpy(gEEPROM_RSSI_CALIB[4], gEEPROM_RSSI_CALIB[3], 8);
	memcpy(gEEPROM_RSSI_CALIB[5], gEEPROM_RSSI_CALIB[3], 8);
	memcpy(gEEPROM_RSSI_CALIB[6], gEEPROM_RSSI_CALIB[3], 8);

	EEPROM_ReadBuffer(0x1EC8, gEEPROM_RSSI_CALIB[0], 8);
	memcpy(gEEPROM_RSSI_CALIB[1], gEEPROM_RSSI_CALIB[0], 8);
	memcpy(gEEPROM_RSSI_CALIB[2], gEEPROM_RSSI_CALIB[0], 8);

	EEPROM_ReadBuffer(0x1F40, gBatteryCalibration, 12);
	if (gBatteryCalibration[0] >= 5000)
	{
		gBatteryCalibration[0] = 1900;
		gBatteryCalibration[1] = 2000;
	}
	gBatteryCalibration[5] = 2300;

	/*#ifdef ENABLE_VOX
		EEPROM_ReadBuffer(0x1F50 + (gSettings.VOX_LEVEL * 2), &gSettings.VOX1_THRESHOLD, 2);
		EEPROM_ReadBuffer(0x1F68 + (gSettings.VOX_LEVEL * 2), &gSettings.VOX0_THRESHOLD, 2);
	#endif

	//EEPROM_ReadBuffer(0x1F80 + gSettings.micGain, &Mic, 1);
	//gSettings.MIC_SENSITIVITY_TUNING = (Mic < 32) ? Mic : 15;
	gSettings.MIC_SENSITIVITY_TUNING = gMicGain_dB2[gSettings.micGain];
	*/
	{
		struct
		{
			int16_t  BK4819_XtalFreqLow;
			uint16_t EEPROM_1F8A;
			uint16_t EEPROM_1F8C;
			uint8_t  VOLUME_GAIN;
			uint8_t  DAC_GAIN;
		} __attribute__((packed)) Misc;

		// radio 1 .. 04 00 46 00 50 00 2C 0E
		// radio 2 .. 05 00 46 00 50 00 2C 0E
		EEPROM_ReadBuffer(0x1F88, &Misc, 8);

		uint16_t BK4819_XTAL_FREQ_LOW = (uint16_t)((Misc.BK4819_XtalFreqLow >= -1000 && Misc.BK4819_XtalFreqLow <= 1000) ? Misc.BK4819_XtalFreqLow : 0);
		gEEPROM_1F8A                 = Misc.EEPROM_1F8A & 0x01FF;
		gEEPROM_1F8C                 = Misc.EEPROM_1F8C & 0x01FF;
		//gSettings.VOLUME_GAIN          = (Misc.VOLUME_GAIN < 64) ? Misc.VOLUME_GAIN : 58;
		//gSettings.DAC_GAIN             = (Misc.DAC_GAIN    < 16) ? Misc.DAC_GAIN    : 8;

		BK4819_WriteRegister(BK4819_REG_3B, (uint16_t)(22656 + BK4819_XTAL_FREQ_LOW));
//		BK4819_WriteRegister(BK4819_REG_3C, gSettings.BK4819_XTAL_FREQ_HIGH);
	}
}

uint32_t SETTINGS_FetchChannelFrequency(const int channel)
{
	struct
	{
		uint32_t frequency;
		uint32_t offset;
	} __attribute__((packed)) info;

	EEPROM_ReadBuffer((uint16_t)(channel * 16), &info, sizeof(info));

	return info.frequency;
}

void SETTINGS_FetchChannelName(char *s, const int channel)
{
	if (s == NULL)
		return;

	s[0] = 0;

	if (channel < 0)
		return;

	if (!RADIO_CheckValidChannel((uint16_t)channel, false, 0))
		return;

	EEPROM_ReadBuffer((uint16_t)(0x0F50 + (channel * 16)), s, 10);

	int i;
	for (i = 0; i < 10; i++)
		if (s[i] < 32 || s[i] > 127)
			break;                // invalid char

	s[i--] = 0;                   // null term

	while (i >= 0 && s[i] == 32)  // trim trailing spaces
		s[i--] = 0;               // null term
}

void SETTINGS_FactoryReset(bool bIsAll)
{
	MEMFAULT_UNUSED(bIsAll);

	if (bIsAll) {
		gSettings = (Settings){
			.settingsVersion = SETTINGS_VERSION,
			.squelch = 4,
			.scrambler = 0,
			.batSave = 4,
			.vox = 0,
			.backlight = 9,
			.backlightTime = 4,
			.txTime = 0,
			.micGain = 15,
			.currentScanlist = 15,
			.roger = 0,
			.scanMode = 0,
			.dw = false,
			.crossBand = false,
			.beep = true,
			.keylock = false,
			.busyChannelTxLock = false,
			.ste = false,
			.repeaterSte = false,
			.dtmfDecode = false,
			.contrast = 8,
			.activePreset = 1,
			.batteryCalibration = 2000,
			.batteryType = BAT_1600,
			.batteryStyle = BAT_PERCENT,
			.backlightOnSquelch = BL_SQL_ON,
			.sqlOpenTime = 1,
			.sqlCloseTime = 1,
			.activeVFO = 0,
			.pmrLock = false,
			.reserved1 = 0xFF,
			.reserved2 = 0xFF,
			.reserved3 = 0xFF,
			.reserved4 = 0xFF,
			.radioName = "UV-K5/8",
		};
		SETTINGS_SaveSettings();
	}
	/*uint16_t i;
	uint8_t  Template[8];

	memset(Template, 0xFF, sizeof(Template));

	for (i = 0x0C80; i < 0x1E00; i += 8)
	{
		if (
			!(i >= 0x0EE0 && i < 0x0F18) &&         // ANI ID + DTMF codes
			!(i >= 0x0F30 && i < 0x0F50) &&         // AES KEY + F LOCK + Scramble Enable
			!(i >= 0x1C00 && i < 0x1E00) &&         // DTMF contacts
			!(i >= 0x0EB0 && i < 0x0ED0) &&         // Welcome strings
			!(i >= 0x0EA0 && i < 0x0EA8) &&         // Voice Prompt
			(bIsAll ||
			(
				!(i >= 0x0D60 && i < 0x0E28) &&     // MR Channel Attributes
				!(i >= 0x0F18 && i < 0x0F30) &&     // Scan List
				!(i >= 0x0F50 && i < 0x1C00) &&     // MR Channel Names
				!(i >= 0x0E40 && i < 0x0E70) &&     // FM Channels
				!(i >= 0x0E88 && i < 0x0E90)        // FM settings
				))
			)
		{
			EEPROM_WriteBuffer(i, Template);
		}
	}

	if (bIsAll)
	{
		RADIO_InitInfo(gRxVfo, FREQ_CHANNEL_FIRST + BAND6_400MHz, 43350000);

		// set the first few memory channels
		for (i = 0; i < ARRAY_SIZE(gDefaultFrequencyTable); i++)
		{
			const uint32_t Frequency   = gDefaultFrequencyTable[i];
			gRxVfo->freq_config_RX.Frequency = Frequency;
			gRxVfo->freq_config_TX.Frequency = Frequency;
			gRxVfo->Band               = FREQUENCY_GetBand(Frequency);
			SETTINGS_SaveChannel((uint8_t)(MR_CHANNEL_FIRST + i), 0, gRxVfo, 2);
		}
	}*/
}


void SETTINGS_SaveVfoIndices(void)
{
	/*uint8_t State[8];

	EEPROM_ReadBuffer(0x0E80, State, sizeof(State));

	State[0] = gScreenChannel[0];
	State[1] = gMrChannel[0];
	State[2] = gFreqChannel[0];
	State[3] = gScreenChannel[1];
	State[4] = gMrChannel[1];
	State[5] = gFreqChannel[1];

	EEPROM_WriteBuffer(0x0E80, State);*/
}

void SETTINGS_SaveSettings(void) {

	EEPROM_WriteBuffer(SETTINGS_OFFSET, &gSettings, SETTINGS_SIZE);

	/*uint8_t  State[8];
	uint32_t Password[2];

	State[0] = gSettings.CHAN_1_CALL;
	State[1] = gSettings.squelch;
	State[2] = gSettings.txTime;
	State[3] = false;
	State[4] = gSettings.KEY_LOCK;
	#ifdef ENABLE_VOX
		State[5] = gSettings.VOX_SWITCH;
		State[6] = gSettings.VOX_LEVEL;
	#else
		State[5] = false;
		State[6] = 0;
	#endif
	State[7] = gSettings.micGain;
	EEPROM_WriteBuffer(0x0E70, State);

	State[0] = (uint8_t)((gSettings.BACKLIGHT_MIN << 4) + gSettings.BACKLIGHT_MAX);
	State[1] = gSettings.CHANNEL_DISPLAY_MODE;
	State[2] = gSettings.CROSS_BAND_RX_TX;
	State[3] = gSettings.batSave;
	State[4] = gSettings.DUAL_WATCH;
	State[5] = gSettings.backlightTime;
	//State[6] = gSettings.ste;
	State[7] = gSettings.VFO_OPEN;

	State[6] = (uint8_t)((gSettings.contrast & 0x7F) | (gSettings.ste << 7));


	EEPROM_WriteBuffer(0x0E78, State);

	State[0] = gSettings.beep;
	State[0] |= (uint8_t)(gSettings.KEY_M_LONG_PRESS_ACTION << 1);
	State[1] = gSettings.KEY_1_SHORT_PRESS_ACTION;
	State[2] = gSettings.KEY_1_LONG_PRESS_ACTION;
	State[3] = gSettings.KEY_2_SHORT_PRESS_ACTION;
	State[4] = gSettings.KEY_2_LONG_PRESS_ACTION;
	State[5] = gSettings.SCAN_RESUME_MODE;
	State[6] = gSettings.AUTO_KEYPAD_LOCK;
	State[7] = gSettings.POWER_ON_DISPLAY_MODE;
	EEPROM_WriteBuffer(0x0E90, State);

	memset(Password, 0xFF, sizeof(Password));
	#ifdef ENABLE_PWRON_PASSWORD
		Password[0] = gSettings.POWER_ON_PASSWORD;
	#endif
	EEPROM_WriteBuffer(0x0E98, Password);

	memset(State, 0xFF, sizeof(State));

	State[1] = gSettings.S0_LEVEL;
	State[2] = gSettings.S9_LEVEL;

	EEPROM_WriteBuffer(0x0EA0, State);


	#if defined(ENABLE_ALARM) || defined(ENABLE_TX1750)
		State[0] = gSettings.ALARM_MODE;
	#else
		State[0] = false;
	#endif
	State[1] = gSettings.roger;
	State[2] = gSettings.repeaterSte;
	State[3] = gSettings.activeVFO;
	State[4] = gSettings.batteryType;
	EEPROM_WriteBuffer(0x0EA8, State);

	State[0] = gSettings.DTMF_SIDE_TONE;
#ifdef ENABLE_DTMF_CALLING
	State[1] = gSettings.DTMF_SEPARATE_CODE;
	State[2] = gSettings.DTMF_GROUP_CALL_CODE;
	State[3] = gSettings.DTMF_DECODE_RESPONSE;
	State[4] = gSettings.DTMF_auto_reset_time;
#endif
	State[5] = (uint8_t)(gSettings.DTMF_PRELOAD_TIME / 10U);
	State[6] = (uint8_t)(gSettings.DTMF_FIRST_CODE_PERSIST_TIME / 10U);
	State[7] = (uint8_t)(gSettings.DTMF_HASH_CODE_PERSIST_TIME / 10U);
	EEPROM_WriteBuffer(0x0ED0, State);

	memset(State, 0xFF, sizeof(State));
	State[0] = (uint8_t)(gSettings.DTMF_CODE_PERSIST_TIME / 10U);
	State[1] = (uint8_t)(gSettings.DTMF_CODE_INTERVAL_TIME / 10U);
#ifdef ENABLE_DTMF_CALLING
	State[2] = gSettings.PERMIT_REMOTE_KILL;
#endif
	EEPROM_WriteBuffer(0x0ED8, State);

	State[0] = gSettings.SCAN_LIST_DEFAULT;
	State[1] = gSettings.SCAN_LIST_ENABLED[0];
	State[2] = gSettings.SCANLIST_PRIORITY_CH1[0];
	State[3] = gSettings.SCANLIST_PRIORITY_CH2[0];
	State[4] = gSettings.SCAN_LIST_ENABLED[1];
	State[5] = gSettings.SCANLIST_PRIORITY_CH1[1];
	State[6] = gSettings.SCANLIST_PRIORITY_CH2[1];
	State[7] = 0xFF;
	EEPROM_WriteBuffer(0x0F18, State);

	memset(State, 0xFF, sizeof(State));
	State[0]  = gSetting_F_LOCK;
	State[1]  = gSetting_350TX;
#ifdef ENABLE_DTMF_CALLING
	State[2]  = gSetting_KILLED;
#endif
	State[3]  = gSetting_200TX;
	State[4]  = gSetting_500TX;
	State[5]  = gSetting_350EN;
	State[6]  = gSetting_ScrambleEnable;
	//if (!gSetting_TX_EN)             State[7] &= ~(1u << 0);
	if (!gSetting_live_DTMF_decoder) State[7] &= (uint8_t)~(1u << 1);
	State[7] = (uint8_t)((State[7] & ~(3u << 2)) | ((gSetting_battery_text & 3u) << 2));
	State[7] = (uint8_t)((State[7] & ~(3u << 6)) | ((gSetting_backlight_on_tx_rx & 3u) << 6));

	EEPROM_WriteBuffer(0x0F40, State);*/
}

void SETTINGS_SaveChannel(uint8_t Channel, uint8_t VFO, const VFO_Info_t *pVFO, uint8_t Mode)
{
	MEMFAULT_UNUSED(Channel);
	MEMFAULT_UNUSED(VFO);
	MEMFAULT_UNUSED(pVFO);
	MEMFAULT_UNUSED(Mode);
	/*uint16_t OffsetVFO = Channel * 16;

	if (IS_FREQ_CHANNEL(Channel)) { // it's a VFO, not a channel
		OffsetVFO  = (VFO == 0) ? 0x0C80 : 0x0C90;
		OffsetVFO += (uint16_t)((Channel - FREQ_CHANNEL_FIRST) * 32);
	}

	if (Mode >= 2 || IS_FREQ_CHANNEL(Channel)) { // copy VFO to a channel
		union {
			uint8_t _8[8];
			uint32_t _32[2];
		} State;

		State._32[0] = pVFO->freq_config_RX.Frequency;
		State._32[1] = pVFO->TX_OFFSET_FREQUENCY;
		EEPROM_WriteBuffer(OffsetVFO + 0, State._32);

		State._8[0] =  pVFO->freq_config_RX.Code;
		State._8[1] =  pVFO->freq_config_TX.Code;
		State._8[2] = (uint8_t)((pVFO->freq_config_TX.CodeType << 4) | pVFO->freq_config_RX.CodeType);
		State._8[3] = (uint8_t)((pVFO->Modulation << 4) | pVFO->TX_OFFSET_FREQUENCY_DIRECTION);
		State._8[4] = (uint8_t)(0
			| (pVFO->BUSY_CHANNEL_LOCK << 4)
			| (pVFO->OUTPUT_POWER      << 2)
			| (pVFO->CHANNEL_BANDWIDTH << 1)
			| (pVFO->FrequencyReverse  << 0));
		State._8[5] = ((pVFO->DTMF_PTT_ID_TX_MODE & 7u) << 1)
#ifdef ENABLE_DTMF_CALLING
			| ((pVFO->DTMF_DECODING_ENABLE & 1u) << 0)
#endif
		;
		State._8[6] =  pVFO->STEP_SETTING;
		State._8[7] =  pVFO->SCRAMBLING_TYPE;
		EEPROM_WriteBuffer(OffsetVFO + 8, State._8);

		SETTINGS_UpdateChannel(Channel, pVFO, true);

		if (IS_MR_CHANNEL(Channel)) {
#ifndef ENABLE_KEEP_MEM_NAME
			// clear/reset the channel name
			SETTINGS_SaveChannelName(Channel, "");
#else
			if (Mode >= 3) {
				SETTINGS_SaveChannelName(Channel, pVFO->Name);
			}
#endif
		}
	}*/

}

void SETTINGS_SaveBatteryCalibration(const uint16_t * batteryCalibration)
{
	MEMFAULT_UNUSED(batteryCalibration);
	/*uint16_t buf[4];
	EEPROM_WriteBuffer(0x1F40, batteryCalibration);
	EEPROM_ReadBuffer( 0x1F48, buf, sizeof(buf));
	buf[0] = batteryCalibration[4];
	buf[1] = batteryCalibration[5];
	EEPROM_WriteBuffer(0x1F48, buf);*/
}

void SETTINGS_SaveChannelName(uint8_t channel, const char * name)
{
	MEMFAULT_UNUSED(channel);
	MEMFAULT_UNUSED(name);
	/*uint16_t offset = channel * 16;
	uint8_t buf[16] = {0};
	memcpy(buf, name, MIN(strlen(name), 10u));
	EEPROM_WriteBuffer(0x0F50 + offset, buf);
	EEPROM_WriteBuffer(0x0F58 + offset, buf + 8);*/
}

void SETTINGS_UpdateChannel(uint8_t channel, const VFO_Info_t *pVFO, bool keep)
{
	MEMFAULT_UNUSED(channel);
	MEMFAULT_UNUSED(pVFO);
	MEMFAULT_UNUSED(keep);
#if 0
	uint8_t  state[8];
	MEMFAULT_UNUSED(keep);
	MEMFAULT_UNUSED(pVFO);
	ChannelAttributes_t  att = {
		.band = 0xf,
		.compander = 0,
		.scanlist1 = 0,
		.scanlist2 = 0,
		};        // default attributes

	uint16_t offset = (uint16_t)(0x0D60 + (channel & ~7u));
	EEPROM_ReadBuffer(offset, state, sizeof(state));

	/*if (keep) {
		att.band = pVFO->Band;
		att.scanlist1 = pVFO->SCANLIST1_PARTICIPATION;
		att.scanlist2 = pVFO->SCANLIST2_PARTICIPATION;
		att.compander = pVFO->Compander;
		if (state[channel & 7u] == att.__val)
			return; // no change in the attributes
	}*/

	state[channel & 7u] = att.__val;
	EEPROM_WriteBuffer(offset, state);

	gMR_ChannelAttributes[channel] = att;

	if (IS_MR_CHANNEL(channel)) {	// it's a memory channel
		if (!keep) {
			// clear/reset the channel name
			SETTINGS_SaveChannelName(channel, "");
		}
	}
#endif
}

void SETTINGS_WriteBuildOptions(void)
{
	/*uint8_t buf[8] = {0};
buf[0] = 0
#ifdef ENABLE_FMRADIO
    | (1 << 0)
#endif
#ifdef ENABLE_VOX
    | (1 << 3)
#endif
#ifdef ENABLE_ALARM
    | (1 << 4)
#endif
#ifdef ENABLE_TX1750
    | (1 << 5)
#endif
#ifdef ENABLE_PWRON_PASSWORD
    | (1 << 6)
#endif
#ifdef ENABLE_DTMF_CALLING
    | (1 << 7)
#endif
;

buf[1] = 0
#ifdef ENABLE_FLASHLIGHT
    | (1 << 0)
#endif
#ifdef ENABLE_WIDE_RX
    | (1 << 1)
#endif
#ifdef ENABLE_BYP_RAW_DEMODULATORS
    | (1 << 2)
#endif
#ifdef ENABLE_BLMIN_TMP_OFF
    | (1 << 3)
#endif
;
	EEPROM_WriteBuffer(0x1FF0, buf);*/
}
