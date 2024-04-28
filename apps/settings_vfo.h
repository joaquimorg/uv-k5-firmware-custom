/* Copyright 2024 joaquim.org
 * https://github.com/joaquimorg
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


#include "apps.h"

#include "misc.h"
#include "gui/ui.h"
#include "gui/gui.h"
#include "applications_task.h"
#include "driver/st7565.h"
#include "dcs.h"
#include "frequencies.h"
#include "radio.h"

#define SUB_MENU_X 68

enum MENU_Option_e {
	MENU_SQL = 0,
	MENU_STEP,
	MENU_TXP,
	MENU_R_DCS,
	MENU_R_CTCS,
	MENU_T_DCS,
	MENU_T_CTCS,
	MENU_SFT_D,
	MENU_OFFSET,
	MENU_TOT,
	MENU_W_N,
	MENU_SCR,
	MENU_BCL,
	MENU_MEM_CH,
	MENU_DEL_CH,
	MENU_MEM_NAME,
	//MENU_MDF,
	MENU_SAVE,
#ifdef ENABLE_VOX
	MENU_VOX,
#endif
	MENU_ABR,
	MENU_ABR_ON_TX_RX,
	MENU_ABR_MIN,
	MENU_ABR_MAX,
	MENU_CONTRAST,
	MENU_TDR,
	MENU_BEEP,
	MENU_SC_REV,
	MENU_AUTOLK,
	MENU_S_ADD1,
	MENU_S_ADD2,
	MENU_STE,
	MENU_RP_STE,
	MENU_MIC,
#ifdef ENABLE_AUDIO_BAR
	MENU_MIC_BAR,
#endif
	MENU_COMPAND,
	MENU_1_CALL,
	MENU_S_LIST,
	MENU_SLIST1,
	MENU_SLIST2,
#ifdef ENABLE_ALARM
	MENU_AL_MOD,
#endif
#ifdef ENABLE_DTMF_CALLING
	MENU_ANI_ID,
#endif
	MENU_UPCODE,
	MENU_DWCODE,
	MENU_PTT_ID,
	MENU_D_ST,
#ifdef ENABLE_DTMF_CALLING
	MENU_D_RSP,
	MENU_D_HOLD,
#endif
	MENU_D_PRE,
#ifdef ENABLE_DTMF_CALLING	
	MENU_D_DCD,
	MENU_D_LIST,
#endif
	MENU_D_LIVE_DEC,
	MENU_PONMSG,
	MENU_ROGER,
	MENU_VOL,
	MENU_BAT_TXT,
	MENU_AM,
#ifdef ENABLE_AM_FIX
	MENU_AM_FIX,
#endif
	MENU_RESET,
	MENU_F_LOCK,
	MENU_200TX,
	MENU_350TX,
	MENU_500TX,
	MENU_350EN,
	MENU_SCREN,
#ifdef ENABLE_F_CAL_MENU
	MENU_F_CALI,  // reference xtal calibration
#endif
	MENU_BATCAL,  // battery voltage calibration
	MENU_F1SHRT,
	MENU_F1LONG,
	MENU_F2SHRT,
	MENU_F2LONG,
	MENU_MLONG,
	MENU_BATTYP,
};

typedef enum MENU_Option_e MENU_Option_t;

typedef struct {
	const char      name[11];
	MENU_Option_t   menu_id;
} menu_item_t;

const menu_item_t MenuList[] =
{
//   text,     menu ID
	{"Step",   		MENU_STEP          },
	//{"TxPwr",  MENU_TXP           }, // was "TXP"
	{"Rx DCS",  	MENU_R_DCS         }, // was "R_DCS"
	{"Rx CTCS", 	MENU_R_CTCS        }, // was "R_CTCS"
	{"Tx DCS",  	MENU_T_DCS         }, // was "T_DCS"
	{"Tx CTCS", 	MENU_T_CTCS        }, // was "T_CTCS"
	{"Tx ODir", 	MENU_SFT_D         }, // was "SFT_D"
	{"Tx Offset", 	MENU_OFFSET        }, // was "OFFSET"
	//{"Bandwi", MENU_W_N           },
	{"Scrambler", 	MENU_SCR           }, // was "SCR"
	{"Busy CL", 	MENU_BCL           }, // was "BCL"
	{"Compand", 	MENU_COMPAND       },
//	{"Demodu", MENU_AM            }, // was "AM"
//	{"Scan Add1", 	MENU_S_ADD1        },
//	{"Scan Add2", 	MENU_S_ADD2        },
//	{"Chan. Save", 	MENU_MEM_CH        }, // was "MEM-CH"
//	{"Chan. Dele", 	MENU_DEL_CH        }, // was "DEL-CH"
//	{"Chan. Name", 	MENU_MEM_NAME      },

//	{"Scan List",  	MENU_S_LIST        },
//	{"Scan List1", 	MENU_SLIST1        },
//	{"Scan List2", 	MENU_SLIST2        },
//	{"Scan Rev", 	MENU_SC_REV        },

//	{"F1 Short", 	MENU_F1SHRT        },
//	{"F1 Long", 	MENU_F1LONG        },
//	{"F2 Short", 	MENU_F2SHRT        },
//	{"F2 Long", 	MENU_F2LONG        },
//	{"M Long", 		MENU_MLONG         },

//	{"Key Lock", 	MENU_AUTOLK        }, // was "AUTOLk"
	{"Tx TimeOut", 	MENU_TOT           }, // was "TOT"
//	{"Bat Save", 	MENU_SAVE          }, // was "SAVE"
//	{"Mic",    		MENU_MIC           },
//#ifdef ENABLE_AUDIO_BAR
//	{"Mic Bar", 	MENU_MIC_BAR       },
//#endif
	//{"ChDisp", MENU_MDF           }, // was "MDF"
//	{"Pow. onMsg", 	MENU_PONMSG        },
//	{"Bat Txt", 	MENU_BAT_TXT       },
//	{"Back Lt", 	MENU_ABR           }, // was "ABR"
//	{"BL Min",  	MENU_ABR_MIN       },
//	{"BL Max",  	MENU_ABR_MAX       },

//	{"LCD Ctr", 	MENU_CONTRAST      },

//	{"Blt TRX", 	MENU_ABR_ON_TX_RX  },
//	{"Beep",   		MENU_BEEP          },
//	{"Roger",  		MENU_ROGER         },
//	{"STE",    		MENU_STE           },
//	{"RP STE", 		MENU_RP_STE        },
//	{"1 Call", 		MENU_1_CALL        },
//#ifdef ENABLE_ALARM
//	{"Alarm T", 	MENU_AL_MOD        },
//#endif
//#ifdef ENABLE_DTMF_CALLING
//	{"ANI ID", 		MENU_ANI_ID        },
//#endif
//	{"UP Code", 	MENU_UPCODE        },
//	{"DW Code", 	MENU_DWCODE        },
	{"PTT ID", 		MENU_PTT_ID        },
	{"D ST",   		MENU_D_ST          },
//#ifdef ENABLE_DTMF_CALLING
//    {"DTMF Resp", 	MENU_D_RSP         },
//	{"DTMF Hold", 	MENU_D_HOLD        },
//#endif
//	{"DTMF Prel", 	MENU_D_PRE         },
//#ifdef ENABLE_DTMF_CALLING
//	{"DTMF Decod", 	MENU_D_DCD         },
//	{"DTMF List", 	MENU_D_LIST        },
//#endif
	{"DTMF Live", 	MENU_D_LIVE_DEC    }, // live DTMF decoder
//#ifdef ENABLE_AM_FIX
//	{"AM Fix", 		MENU_AM_FIX        },
//#endif
//#ifdef ENABLE_VOX
//	{"VOX",    		MENU_VOX           },
//#endif
	//{"Bat Vol", 	MENU_VOL           }, // was "VOL"
	{"Rx Mode", 	MENU_TDR           },
	{"SQL",    		MENU_SQL           },

	// hidden menu items from here on
	// enabled if pressing both the PTT and upper side button at power-on
//	{"F Lock", 		MENU_F_LOCK        },
//	{"Tx 200", 		MENU_200TX         }, // was "200TX"
//	{"Tx 350", 		MENU_350TX         }, // was "350TX"
//	{"Tx 500", 		MENU_500TX         }, // was "500TX"
//	{"350 En", 		MENU_350EN         }, // was "350EN"
//	{"Scra En", 	MENU_SCREN         }, // was "SCREN"
//#ifdef ENABLE_F_CAL_MENU
//	{"Frq Cali", 	MENU_F_CALI        }, // reference xtal calibration
//#endif
//	{"Bat Cali", 	MENU_BATCAL        }, // battery voltage calibration
//	{"Bat Type", 	MENU_BATTYP        }, // battery type 1600/2200mAh
//	{"Reset",  		MENU_RESET         }, // might be better to move this to the hidden menu items ?

//	{"",       0xff               }  // end of list - DO NOT delete or move this this
};

#define MENU_VFO_SIZE 16

const char* const gSubMenu_OFF_ON[] =
{
	"OFF",
	"ON"
};

const char* const gSubMenu_RX_TX[] =
{
	"OFF",
	"TX",
	"RX",
	"TX/RX"
};

const char* const gSubMenu_F_LOCK[] =
{
	/*"DEFAULT+ 137-174 400-470",
	"FCC HAM 144-148 420-450",
	"CE HAM 144-146 430-440",
	"GB HAM 144-148 430-440",
	"137-174 400-430",
	"137-174 400-438",*/

	"DEFAULT+",
	"DISABLE ALL",
	//"UNLOCK\nALL",
};

const char* const gSubMenu_RXMode[] =
{
	"MAIN RX TX", 		// TX and RX on main only
	"DUAL RX TX", // Watch both and respond
	"CROSS B.", 		// TX on main, RX on secondary
	"TX MAIN" 	// always TX on main, but RX on both
};

const char* const gSubMenu_ROGER[] =
{
	"OFF",
	"DEFAULT"
};

const char* const gSubMenu_RESET[] =
{
	"VFO",
	"ALL"
};

const char* const gSubMenu_SAVE[] =
{
	"OFF",
	"1:1",
	"1:2",
	"1:3",
	"1:4"
};

const char* const gSubMenu_BAT_TXT[] =
{
	"NONE",
	"VOLTAGE",
	"PERCENT"
};


const char* const gSubMenu_BATTYP[] =
{
	"1600mAh",
	"2200mAh"
};

const char* const gSubMenu_SCRAMBLER[] =
{
	"OFF",
	"2600Hz",
	"2700Hz",
	"2800Hz",
	"2900Hz",
	"3000Hz",
	"3100Hz",
	"3200Hz",
	"3300Hz",
	"3400Hz",
	"3500Hz"
};

const char* const gSubMenu_PONMSG[] =
{
	"FULL",
	"MESSAGE",
	"VOLTAGE",
	"NONE"
};

const char* const gSubMenu_SC_REV[] =
{
	"TIMEOUT",
	"CARRIER",
	"STOP"
};

const char* const gSubMenu_PTT_ID[] =
{
	"OFF",
	"UP CODE",
	"DOWN CODE",
	"U+D CODE",
	"APOLLO"
};


uint8_t settingsCurrentMenu = 0;

uint16_t settingsCurrentSubMenu = 0;
uint16_t settingsSubmenuMin = 0;
uint16_t settingsSubmenuSize = 0;

#define SETTINGS_TIMESHOW_SUB 1000
TickType_t settingsSubMenuTime;
bool settingsShowSubMenu;
bool settingsSubMenuActive;
bool needToSave = false;

uint8_t getMenuID(void) {
	return MenuList[settingsCurrentMenu].menu_id;
}

void SettingsMenu_showList() {
    uint8_t yPos = 15;
	const uint8_t offset = Clamp(settingsCurrentMenu - 2, 0, MENU_VFO_SIZE - 5);

    for (uint8_t i = 0; i < 5; i++) {
		if ( (i + offset) < MENU_VFO_SIZE ) {
			bool isFill = settingsCurrentMenu == i + offset ? true : false;
		
			UI_printf(&font_small, TEXT_ALIGN_LEFT, 8, 0, yPos - 1, true, false, "%0.2i", (i + 1) + offset);
		
			UI_printf(&font_10, TEXT_ALIGN_LEFT, 18, 0, yPos, !isFill, isFill,
				"%s", MenuList[i + offset].name);
			yPos += 11;
		}
    }

    yPos = 10;
    yPos += ((( (100 * settingsCurrentMenu) / MENU_VFO_SIZE ) / 100.0) * 54);

    UI_drawFastVLine(1, yPos - 1, 3, true);
    UI_drawFastVLine(3, yPos - 1, 3, true);
}

void SettingsMenu_showSubListCalc(uint8_t *yPos, uint8_t *listCount, uint8_t *offset) {
    if (settingsSubmenuSize <= 2 ) {
        if (settingsSubmenuSize == 2 ) {
            *yPos = *yPos + 6;
        } else if (settingsSubmenuSize == 1 ) {
            *yPos = *yPos + 11;
        }
        *listCount = settingsSubmenuSize + 1;
    } else {
		*offset = Clamp(settingsCurrentSubMenu - 2, 0, settingsSubmenuSize - 3);
    }
}


void SettingsMenu_showSubList(const char* const subList[]) {
	uint8_t yPos = 21;
    uint8_t listCount = 4;
	uint8_t offset = 0;
	SettingsMenu_showSubListCalc(&yPos, &listCount, &offset);
    for (uint8_t i = 0; i < listCount; i++) {
		bool isFill = settingsCurrentSubMenu == i + offset ? true : false;
		UI_printf(&font_10, TEXT_ALIGN_CENTER, SUB_MENU_X, 125, yPos, !isFill, isFill,
			"%s", subList[i + offset]);
        yPos += 11;
    }
}

void SettingsMenu_showSubListValue(void) {
	uint8_t yPos = 21;
    uint8_t listCount = 4;
	uint8_t offset = 0;
	SettingsMenu_showSubListCalc(&yPos, &listCount, &offset);    
    for (uint8_t i = 0; i < listCount; i++) {
		bool isFill = settingsCurrentSubMenu == i + offset ? true : false;

		if ( (i + offset == 0) && (getMenuID() != MENU_STEP) ) {
			UI_printf(&font_10, TEXT_ALIGN_CENTER, SUB_MENU_X, 125, yPos, !isFill, isFill, "OFF" );
		} else {

			switch (getMenuID())
			{
				case MENU_STEP: {
					uint16_t step = gStepFrequencyTable[FREQUENCY_GetStepIdxFromSortedIdx(i + offset)];
					UI_printf(&font_10, TEXT_ALIGN_CENTER, SUB_MENU_X, 125, yPos, !isFill, isFill, "%d.%02ukHz", step / 100, step % 100 );
					break;
				}

				case MENU_R_DCS:
				case MENU_T_DCS:
					if (settingsCurrentSubMenu < 105) {
						UI_printf(&font_10, TEXT_ALIGN_CENTER, SUB_MENU_X, 125, yPos, !isFill, isFill, "D %03o N", DCS_Options[(i + offset) - 1]);
					} else {
						UI_printf(&font_10, TEXT_ALIGN_CENTER, SUB_MENU_X, 125, yPos, !isFill, isFill, "D %03o I", DCS_Options[(i + offset) - 105]);
					}
				break;

				case MENU_R_CTCS:
				case MENU_T_CTCS:
					UI_printf(&font_10, TEXT_ALIGN_CENTER, SUB_MENU_X, 125, yPos, !isFill, isFill, "%u.%uHz", CTCSS_Options[(i + offset) - 1] / 10, CTCSS_Options[(i + offset) - 1] % 10);
				break;

				case MENU_MIC:
					UI_printf(&font_10, TEXT_ALIGN_CENTER, SUB_MENU_X, 125, yPos, !isFill, isFill, "+%u.%01u dB", gMicGain_dB2[(i + offset) - 1] / 2, gMicGain_dB2[(i + offset) - 1] % 2);
				break;

				default:
            	break;
			}
		}
        yPos += 11;
    }
}


void SettingsMenu_showSubListNumber() {
	uint8_t yPos = 21;
    uint8_t listCount = 4;
	uint8_t offset = 0;
	SettingsMenu_showSubListCalc(&yPos, &listCount, &offset);
    for (uint8_t i = 0; i < listCount; i++) {
		UI_printf(&font_10, TEXT_ALIGN_CENTER, SUB_MENU_X, 125, yPos,
			settingsCurrentSubMenu == i + offset ? false : true,
			settingsCurrentSubMenu == i + offset ? true : false,
			"%i", (i + offset));
        yPos += 11;
    }
}

int SettingsMenu_GetLimits(uint8_t menu_id, uint16_t *pMin, uint16_t *pMax);

void SettingsMenu_loadSubList() {

	UI_fillRect(SUB_MENU_X - 2, 11, 61, 48, false);
    if ( settingsSubMenuActive ) {
    	UI_drawRoundRect(SUB_MENU_X - 2, 11, 61, 48, 4, true);
	}
    UI_drawRoundRect(SUB_MENU_X - 1, 12, 59, 46, 4, true);

	SettingsMenu_GetLimits(getMenuID(), &settingsSubmenuMin, &settingsSubmenuSize);
	
	bool showNumber = false;
	bool showValue = false;

	switch (getMenuID())
    {

		case MENU_ABR:
			//gSubMenu_BACKLIGHT
			
            SettingsMenu_showSubList(gSubMenu_BACKLIGHT);
			break;

		case MENU_SCR:
			//gSubMenu_SCRAMBLER
			
            SettingsMenu_showSubList(gSubMenu_SCRAMBLER);
			break;

		case MENU_SQL:
		case MENU_ABR_MIN:
		case MENU_ABR_MAX:
		case MENU_RP_STE:
		case MENU_CONTRAST:
		case MENU_BATCAL:
			showNumber = true;
			break;

		case MENU_F_LOCK:
			//gSubMenu_F_LOCK
			
            SettingsMenu_showSubList(gSubMenu_F_LOCK);
			break;

		case MENU_PTT_ID:
			
            SettingsMenu_showSubList(gSubMenu_PTT_ID);
			break;

		case MENU_SFT_D:
			//gSubMenu_SFT_D
			
            SettingsMenu_showSubList(gSubMenu_SFT_D);
			break;

		case MENU_TDR:
			//gSubMenu_RXMode
			
            SettingsMenu_showSubList(gSubMenu_RXMode);
			break;

		case MENU_ROGER:
			//gSubMenu_ROGER
			
            SettingsMenu_showSubList(gSubMenu_ROGER);
			break;

		case MENU_STEP:
		case MENU_R_DCS:
		case MENU_T_DCS:
		case MENU_R_CTCS:
		case MENU_T_CTCS:
		case MENU_MIC:
			showValue = true;
			break;

		case MENU_RESET:
			//gSubMenu_RESET
			
            SettingsMenu_showSubList(gSubMenu_RESET);
			break;

		case MENU_COMPAND:
		case MENU_ABR_ON_TX_RX:
			//gSubMenu_RX_TX
			
            SettingsMenu_showSubList(gSubMenu_RX_TX);
			break;


		case MENU_D_LIVE_DEC:
		case MENU_350TX:
		case MENU_200TX:
		case MENU_500TX:
		case MENU_350EN:
		case MENU_SCREN:
#ifdef ENABLE_DTMF_CALLING
		case MENU_D_DCD:
#endif
		case MENU_D_ST:
		case MENU_S_ADD1:
		case MENU_S_ADD2:
		case MENU_AM_FIX:
		case MENU_MIC_BAR:
		case MENU_BCL:
		case MENU_BEEP:
		case MENU_AUTOLK:
		case MENU_STE:
			//gSubMenu_OFF_ON
			
        	SettingsMenu_showSubList(gSubMenu_OFF_ON);
			break;

		case MENU_TOT:
			//gSubMenu_TOT
			
            SettingsMenu_showSubList(gSubMenu_TOT);
			break;

		case MENU_SAVE:
			//gSubMenu_SAVE
			
            SettingsMenu_showSubList(gSubMenu_SAVE);
			break;

		case MENU_BAT_TXT:
			//gSubMenu_BAT_TXT
			
            SettingsMenu_showSubList(gSubMenu_BAT_TXT);
			break;

		case MENU_PONMSG:
			
            SettingsMenu_showSubList(gSubMenu_PONMSG);
			break;

		case MENU_SC_REV:
			
            SettingsMenu_showSubList(gSubMenu_SC_REV);
			break;


		case MENU_OFFSET:
			//TODO:
			//SettingsMenu_showSubValue();
			break;

		case MENU_S_LIST:
			//TODO:
			break;

		case MENU_SLIST1:
		case MENU_SLIST2:
			//TODO:
			break;

		case MENU_MEM_CH:
		case MENU_1_CALL:
		case MENU_DEL_CH:
			//TODO:
			break;

		case MENU_MEM_NAME:
			//TODO:
			break;

		case MENU_VOL: {
			const uint8_t gBatteryPercentage = BATTERY_VoltsToPercent(gBatteryVoltageAverage);
			UI_printf(&font_10, TEXT_ALIGN_CENTER, SUB_MENU_X, 125, 26, true, false,	"%u.%02uV", gBatteryVoltageAverage / 100, gBatteryVoltageAverage % 100);
			UI_printf(&font_10, TEXT_ALIGN_CENTER, SUB_MENU_X, 125, 37, true, false,	"%3i%%", gBatteryPercentage);
			UI_printf(&font_10, TEXT_ALIGN_CENTER, SUB_MENU_X, 125, 48, true, false,	"%dmA", gBatteryCurrent);
			break;
		}

		case MENU_BATTYP:
			// 0 .. 1;
			//gSubMenu_BATTYP
			
            SettingsMenu_showSubList(gSubMenu_BATTYP);
			break;


        default:
            break;
    }
	if (showValue) {
		SettingsMenu_showSubListValue();
	} else if (showNumber) {
		SettingsMenu_showSubListNumber();
	}

}

void SettingsMenu_loadSubListValues() {

	settingsSubmenuMin = 0;

	switch (getMenuID())
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

		/*case MENU_RESET:
			settingsCurrentSubMenu = 0;
			break;*/

		case MENU_R_DCS:
		case MENU_R_CTCS:
		{
			DCS_CodeType_t type = gTxVfo->freq_config_RX.CodeType;
			uint8_t code = gTxVfo->freq_config_RX.Code;
			/*int menuid = MenuList[settingsCurrentMenu].menu_id;

			if(gScanUseCssResult) {
				gScanUseCssResult = false;
				type = gScanCssResultType;
				code = gScanCssResultCode;
			}*/
			if((getMenuID() == MENU_R_CTCS) ^ (type==CODE_TYPE_CONTINUOUS_TONE)) { //not the same type
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

		/*case MENU_W_N:
			settingsCurrentSubMenu = gTxVfo->CHANNEL_BANDWIDTH;
			break;*/

		case MENU_SCR:
			settingsCurrentSubMenu = gTxVfo->SCRAMBLING_TYPE;
			break;

		case MENU_BCL:
			settingsCurrentSubMenu = gTxVfo->BUSY_CHANNEL_LOCK;
			break;

		/*case MENU_MEM_CH:
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
			break;*/

		case MENU_TDR:
			settingsCurrentSubMenu = (gEeprom.DUAL_WATCH != DUAL_WATCH_OFF) + (gEeprom.CROSS_BAND_RX_TX != CROSS_BAND_OFF) * 2;
			break;

		/*case MENU_BEEP:
			settingsCurrentSubMenu = gEeprom.BEEP_CONTROL;
			break;*/

		case MENU_TOT:
			settingsCurrentSubMenu = gEeprom.TX_TIMEOUT_TIMER;
			break;

		/*case MENU_SC_REV:
			settingsCurrentSubMenu = gEeprom.SCAN_RESUME_MODE;
			break;*/

		/*case MENU_MDF:
			settingsCurrentSubMenu = gEeprom.CHANNEL_DISPLAY_MODE;
			break;*/

		/*case MENU_AUTOLK:
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
*/
		case MENU_COMPAND:
			settingsCurrentSubMenu = gTxVfo->Compander;
			return;

		/*case MENU_1_CALL:
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
*/
		case MENU_PTT_ID:
			settingsCurrentSubMenu = gTxVfo->DTMF_PTT_ID_TX_MODE;
			break;

		/*case MENU_BAT_TXT:
			settingsCurrentSubMenu = gSetting_battery_text;
			return;*/

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

		/*case MENU_PONMSG:
			settingsCurrentSubMenu = gEeprom.POWER_ON_DISPLAY_MODE;
			break;

		case MENU_ROGER:
			settingsCurrentSubMenu = gEeprom.ROGER;
			break;

		case MENU_AM:
			settingsCurrentSubMenu = gTxVfo->Modulation;
			break;*/

#ifdef ENABLE_AM_FIX
		case MENU_AM_FIX:
			settingsCurrentSubMenu = gSetting_AM_fix;
			break;
#endif

		/*case MENU_DEL_CH:
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
		case MENU_MLONG:*/
		/*{
			uint8_t * fun[]= {
				&gEeprom.KEY_1_SHORT_PRESS_ACTION,
				&gEeprom.KEY_1_LONG_PRESS_ACTION,
				&gEeprom.KEY_2_SHORT_PRESS_ACTION,
				&gEeprom.KEY_2_LONG_PRESS_ACTION,
				&gEeprom.KEY_M_LONG_PRESS_ACTION};
			uint8_t id = *fun[MenuList[settingsCurrentMenu].menu_id-MENU_F1SHRT];

			for(int i = 0; i < gSubMenu_SIDEFUNCTIONS_size; i++) {
				if(gSubMenu_SIDEFUNCTIONS[i].id==id) {
					settingsCurrentSubMenu = i;
					break;
				}

			}
			break;
		}*/
			//break;

		default:
			return;
	}
}


int SettingsMenu_GetLimits(uint8_t menu_id, uint16_t *pMin, uint16_t *pMax)
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
			*pMax = ARRAY_SIZE(gSubMenu_BACKLIGHT) - 1;
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
			*pMax = ARRAY_SIZE(gSubMenu_F_LOCK) - 1;
			break;

		/*case MENU_MDF:
			*pMax = ARRAY_SIZE(gSubMenu_MDF) - 1;
			break;*/

		/*case MENU_TXP:
			*pMax = ARRAY_SIZE(gSubMenu_TXP) - 1;
			break;*/

		case MENU_SFT_D:
			*pMax = ARRAY_SIZE(gSubMenu_SFT_D) - 1;
			break;

		case MENU_TDR:
			*pMax = ARRAY_SIZE(gSubMenu_RXMode) - 1;
			break;

		case MENU_SC_REV:
			*pMax = ARRAY_SIZE(gSubMenu_SC_REV) - 1;
			break;

		case MENU_ROGER:
			*pMax = ARRAY_SIZE(gSubMenu_ROGER) - 1;
			break;

		case MENU_PONMSG:
			*pMax = ARRAY_SIZE(gSubMenu_PONMSG) - 1;
			break;

		case MENU_R_DCS:
		case MENU_T_DCS:
			*pMax = 208;
			//*pMax = (ARRAY_SIZE(DCS_Options) * 2) - 1;
			break;

		case MENU_R_CTCS:
		case MENU_T_CTCS:
			*pMax = ARRAY_SIZE(CTCSS_Options) - 1;
			break;

		case MENU_W_N:
			*pMax = ARRAY_SIZE(gSubMenu_W_N) - 1;
			break;

		#ifdef ENABLE_ALARM
			case MENU_AL_MOD:

				*pMax = ARRAY_SIZE(gSubMenu_AL_MOD) - 1;
				break;
		#endif

		case MENU_RESET:
			*pMax = ARRAY_SIZE(gSubMenu_RESET) - 1;
			break;

		case MENU_COMPAND:
		case MENU_ABR_ON_TX_RX:
			*pMax = ARRAY_SIZE(gSubMenu_RX_TX) - 1;
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
			*pMax = ARRAY_SIZE(gSubMenu_OFF_ON) - 1;
			break;

		/*case MENU_AM:
			*pMax = ARRAY_SIZE(gModulationStr) - 1;
			break;*/

		case MENU_SCR:
			*pMax = ARRAY_SIZE(gSubMenu_SCRAMBLER) - 1;
			break;

		case MENU_TOT:
			*pMax = ARRAY_SIZE(gSubMenu_TOT) - 1;
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
			*pMax = ARRAY_SIZE(gSubMenu_SAVE) - 1;
			break;

		case MENU_MIC:
			*pMax = 6;
			break;

		case MENU_S_LIST:
			*pMax = 2;
			break;

#ifdef ENABLE_DTMF_CALLING
		case MENU_D_RSP:
			*pMax = ARRAY_SIZE(gSubMenu_D_RSP) - 1;
			break;
#endif
		case MENU_PTT_ID:
			*pMax = ARRAY_SIZE(gSubMenu_PTT_ID) - 1;
			break;

		case MENU_BAT_TXT:
			*pMax = ARRAY_SIZE(gSubMenu_BAT_TXT) - 1;
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
			//*pMax = gSubMenu_SIDEFUNCTIONS_size-1;
			break;

		default:
			return -1;
	}

	return 0;
}

void compareValuesInt(uint32_t value1) {
	if ( needToSave ) return;
	needToSave = (value1 |= settingsCurrentSubMenu);
}

void MenuVFO_saveSetting(void) {
	FREQ_Config_t *pConfig = &gTxVfo->freq_config_RX;

	switch (getMenuID())
	{
		default:
			return;

		case MENU_SQL:
			gEeprom.SQUELCH_LEVEL = settingsCurrentSubMenu;
			//gVfoConfigureMode     = VFO_CONFIGURE;
			break;

		case MENU_STEP:
			gTxVfo->STEP_SETTING = FREQUENCY_GetStepIdxFromSortedIdx(settingsCurrentSubMenu);
			if (IS_FREQ_CHANNEL(gTxVfo->CHANNEL_SAVE))
			{
				//gRequestSaveChannel = 1;
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

			//gRequestSaveChannel = 1;
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

			//gRequestSaveChannel = 1;
			return;
		}
		case MENU_SFT_D:
			gTxVfo->TX_OFFSET_FREQUENCY_DIRECTION = settingsCurrentSubMenu;
			//gRequestSaveChannel                   = 1;
			return;

		case MENU_OFFSET:
			gTxVfo->TX_OFFSET_FREQUENCY = settingsCurrentSubMenu;
			//gRequestSaveChannel         = 1;
			return;

		case MENU_W_N:
			gTxVfo->CHANNEL_BANDWIDTH = settingsCurrentSubMenu;
			//gRequestSaveChannel       = 1;
			return;

		case MENU_SCR:
			gTxVfo->SCRAMBLING_TYPE = settingsCurrentSubMenu;
			//gRequestSaveChannel     = 1;
			return;

		case MENU_BCL:
			gTxVfo->BUSY_CHANNEL_LOCK = settingsCurrentSubMenu;
			//gRequestSaveChannel       = 1;
			return;

		case MENU_MEM_CH:
			gTxVfo->CHANNEL_SAVE = settingsCurrentSubMenu;
			#if 0
				gEeprom.MrChannel[0] = settingsCurrentSubMenu;
			#else
				gEeprom.MrChannel[gEeprom.TX_VFO] = settingsCurrentSubMenu;
			#endif
			//gRequestSaveChannel = 2;
			//gVfoConfigureMode   = VFO_CONFIGURE_RELOAD;
			//gFlagResetVfos      = true;
			return;

		case MENU_MEM_NAME:
			/*for (int i = 9; i >= 0; i--) {
				if (edit[i] != ' ' && edit[i] != '_' && edit[i] != 0x00 && edit[i] != 0xff)
					break;
				edit[i] = ' ';
			}*/

			//SETTINGS_SaveChannelName(settingsCurrentSubMenu, edit);
			return;

		case MENU_SAVE:
			gEeprom.BATTERY_SAVE = settingsCurrentSubMenu;
			break;

		#ifdef ENABLE_VOX
			case MENU_VOX:
				gEeprom.VOX_SWITCH = settingsCurrentSubMenu != 0;
				if (gEeprom.VOX_SWITCH)
					gEeprom.VOX_LEVEL = settingsCurrentSubMenu - 1;
				//SETTINGS_LoadCalibration();
				//gFlagReconfigureVfos = true;
				//gUpdateStatus        = true;
				break;
		#endif

		case MENU_ABR:
			gEeprom.BACKLIGHT_TIME = settingsCurrentSubMenu;
			app_push_message(APP_MSG_BACKLIGHT);
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
			//ST7565_SetContrast(gEeprom.LCD_CONTRAST);
			break;

		case MENU_ABR_ON_TX_RX:
			gSetting_backlight_on_tx_rx = settingsCurrentSubMenu;
			break;

		case MENU_TDR:
			gEeprom.DUAL_WATCH = (gEeprom.TX_VFO + 1) * (settingsCurrentSubMenu & 1);
			gEeprom.CROSS_BAND_RX_TX = (gEeprom.TX_VFO + 1) * ((settingsCurrentSubMenu & 2) > 0);

			//gFlagReconfigureVfos = true;
			//gUpdateStatus        = true;
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
			//gKeyLockCountdown        = 30;
			break;

		case MENU_S_ADD1:
			gTxVfo->SCANLIST1_PARTICIPATION = settingsCurrentSubMenu;
			//SETTINGS_UpdateChannel(gTxVfo->CHANNEL_SAVE, gTxVfo, true);
			//gVfoConfigureMode = VFO_CONFIGURE;
			//gFlagResetVfos    = true;
			return;

		case MENU_S_ADD2:
			gTxVfo->SCANLIST2_PARTICIPATION = settingsCurrentSubMenu;
			//SETTINGS_UpdateChannel(gTxVfo->CHANNEL_SAVE, gTxVfo, true);
			//gVfoConfigureMode = VFO_CONFIGURE;
			//gFlagResetVfos    = true;
			return;

		case MENU_STE:
			gEeprom.TAIL_TONE_ELIMINATION = settingsCurrentSubMenu;
			break;

		case MENU_RP_STE:
			gEeprom.REPEATER_TAIL_TONE_ELIMINATION = settingsCurrentSubMenu;
			break;

		case MENU_MIC:
			gEeprom.MIC_SENSITIVITY = settingsCurrentSubMenu;
			//SETTINGS_LoadCalibration();
			//gFlagReconfigureVfos = true;
			break;

		#ifdef ENABLE_AUDIO_BAR
			case MENU_MIC_BAR:
				gSetting_mic_bar = settingsCurrentSubMenu;
				break;
		#endif

		case MENU_COMPAND:
			gTxVfo->Compander = settingsCurrentSubMenu;
			//SETTINGS_UpdateChannel(gTxVfo->CHANNEL_SAVE, gTxVfo, true);
			//gVfoConfigureMode = VFO_CONFIGURE;
			//gFlagResetVfos    = true;
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
			//gRequestSaveChannel         = 1;
			return;

		case MENU_BAT_TXT:
			gSetting_battery_text = settingsCurrentSubMenu;
			break;

#ifdef ENABLE_DTMF_CALLING
		case MENU_D_DCD:
			gTxVfo->DTMF_DECODING_ENABLE = settingsCurrentSubMenu;
			//DTMF_clear_RX();
			//gRequestSaveChannel = 1;
			return;
#endif

		case MENU_D_LIVE_DEC:
			gSetting_live_DTMF_decoder = settingsCurrentSubMenu;
			/*gDTMF_RX_live_timeout = 0;
			memset(gDTMF_RX_live, 0, sizeof(gDTMF_RX_live));
			if (!gSetting_live_DTMF_decoder)
				BK4819_DisableDTMF();
			gFlagReconfigureVfos     = true;
			gUpdateStatus            = true;*/
			break;

#ifdef ENABLE_DTMF_CALLING
		case MENU_D_LIST:
			gDTMF_chosen_contact = settingsCurrentSubMenu - 1;
			/*if (gIsDtmfContactValid)
			{
				GUI_SelectNextDisplay(DISPLAY_MAIN);
				gDTMF_InputMode       = true;
				gDTMF_InputBox_Index  = 3;
				memcpy(gDTMF_InputBox, gDTMF_ID, 4);
				gRequestDisplayScreen = DISPLAY_INVALID;
			}*/
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
			//gRequestSaveChannel = 1;
			return;

		#ifdef ENABLE_AM_FIX
			case MENU_AM_FIX:
				gSetting_AM_fix = settingsCurrentSubMenu;
				//gVfoConfigureMode = VFO_CONFIGURE_RELOAD;
				//gFlagResetVfos    = true;
				break;
		#endif

		case MENU_DEL_CH:
			/*SETTINGS_UpdateChannel(settingsCurrentSubMenu, NULL, false);
			gVfoConfigureMode = VFO_CONFIGURE_RELOAD;
			gFlagResetVfos    = true;*/
			return;

		case MENU_RESET:
			//SETTINGS_FactoryReset(settingsCurrentSubMenu);
			return;

		case MENU_350TX:
			gSetting_350TX = settingsCurrentSubMenu;
			break;

		case MENU_F_LOCK: {
			/*if(settingsCurrentSubMenu == F_LOCK_NONE) { // select 10 times to enable
				gUnlockAllTxConfCnt++;
				if(gUnlockAllTxConfCnt < 10)
					return;
			}
			else
				gUnlockAllTxConfCnt = 0;

			gSetting_F_LOCK = settingsCurrentSubMenu;*/
			break;
		}
		case MENU_200TX:
			//gSetting_200TX = settingsCurrentSubMenu;
			break;

		case MENU_500TX:
			//gSetting_500TX = settingsCurrentSubMenu;
			break;

		case MENU_350EN:
			/*gSetting_350EN       = settingsCurrentSubMenu;
			gVfoConfigureMode    = VFO_CONFIGURE_RELOAD;
			gFlagResetVfos       = true;*/
			break;

		case MENU_SCREN:
			/*gSetting_ScrambleEnable = settingsCurrentSubMenu;
			gFlagReconfigureVfos    = true;*/
			break;

		#ifdef ENABLE_F_CAL_MENU
			case MENU_F_CALI:
				//writeXtalFreqCal(settingsCurrentSubMenu, true);
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
			//SETTINGS_SaveBatteryCalibration(gBatteryCalibration);
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
			break;

	}

	//gRequestSaveSettings = true;
}

void MenuVFO_initFunction() {
    settingsSubMenuTime = xTaskGetTickCount();
    settingsShowSubMenu = false;
    settingsSubMenuActive = false;
}

void MenuVFO_renderFunction() {

    UI_displayClear();
    UI_drawFastVLine(2, 9, 53, true);

    SettingsMenu_showList();

    if (settingsShowSubMenu) {
        SettingsMenu_loadSubList();
    } else {
        if (xTaskGetTickCount() - settingsSubMenuTime > pdMS_TO_TICKS(SETTINGS_TIMESHOW_SUB)) {
			if( GUI_inputGetSize() == 1 ) {
				const uint8_t inputValue = GUI_inputGetNumber();
				if ( inputValue > 0) {
					settingsCurrentMenu = inputValue - 1;
					settingsShowSubMenu = false;
					settingsSubMenuTime = xTaskGetTickCount();
				}
			} else {
            	settingsShowSubMenu = true;
            	settingsCurrentSubMenu = 0;
				SettingsMenu_loadSubListValues();
			}
        }
    }

	//UI_printf(7, 125, 60, true, false,	"%i %i", settingsCurrentSubMenu, settingsSubmenuSize);
}

void MenuVFO_keyHandlerFunction(KEY_Code_t key, KEY_State_t state) {

	switch (key)
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
			if ( state == KEY_PRESSED ) {
				if ( !settingsSubMenuActive ) {
					GUI_inputAppendKey(key, 2, false);
					if( GUI_inputGetSize() == 2 ) {
						const uint8_t inputValue = GUI_inputGetNumber();
						if ( inputValue > 0 && inputValue < MENU_VFO_SIZE ) {
							settingsCurrentMenu = inputValue - 1;
						}
					}
					settingsSubMenuTime = xTaskGetTickCount();
					settingsShowSubMenu = false;
				}
			}
			break;
		case KEY_UP:
			if ( state == KEY_PRESSED || state == KEY_LONG_PRESSED_CONT ) {
				if ( settingsSubMenuActive ) {
					if ( settingsCurrentSubMenu > settingsSubmenuMin ) {
						settingsCurrentSubMenu--;
					} else {
						settingsCurrentSubMenu = settingsSubmenuSize;
					}
				} else {
					if ( settingsCurrentMenu > 0 ) {
						settingsCurrentMenu--;
					} else {
						settingsCurrentMenu = MENU_VFO_SIZE - 1;
					}
					settingsSubMenuTime = xTaskGetTickCount();
					settingsShowSubMenu = false;
				}
			}
			break;
		case KEY_DOWN:
			if ( state == KEY_PRESSED || state == KEY_LONG_PRESSED_CONT ) {
				if ( settingsSubMenuActive ) {
					if ( settingsCurrentSubMenu < settingsSubmenuSize ) {
						settingsCurrentSubMenu++;
					} else {
						settingsCurrentSubMenu = settingsSubmenuMin;
					}
				} else {
					if ( settingsCurrentMenu < MENU_VFO_SIZE - 1 ) {
						settingsCurrentMenu++;
					} else {
						settingsCurrentMenu = 0;
					}
					settingsSubMenuTime = xTaskGetTickCount();
					settingsShowSubMenu = false;
				}
			}
			break;
		case KEY_MENU:
			if ( state == KEY_PRESSED ) {
				if ( settingsSubMenuActive ) {
					//save submenu
					MenuVFO_saveSetting();
					settingsSubMenuActive = false;
					app_push_message(APP_MSG_TIMEOUT_RETURN_MAIN);
				} else {
					settingsSubMenuActive = true;
					settingsShowSubMenu = true;
					settingsCurrentSubMenu = 0;
					SettingsMenu_loadSubListValues();
					app_push_message(APP_MSG_TIMEOUT_NO_RETURN_MAIN);
				}
			}
			break;
		case KEY_EXIT:
			if ( state == KEY_PRESSED ) {
				if ( settingsSubMenuActive ) {
					settingsSubMenuActive = false;
				} else {
					load_application(APP_MAIN_VFO);
				}
			}
			break;
		default:
			break;
	}

}


app_t APPVFOSettings = {
    .showStatusLine = true,
    .init = MenuVFO_initFunction,
    .render = MenuVFO_renderFunction,
    .keyHandler = MenuVFO_keyHandlerFunction
};
