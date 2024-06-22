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
/*
enum Main_MENU_Option_e {
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

typedef enum Main_MENU_Option_e MENU_Option_t;

typedef struct {
	const char      name[11];
	MENU_Option_t   menu_id;
} menu_item_t;
*/
const menu_item_t MainMenuList[] =
{
//   text,     menu ID
//	{"Step",   		MENU_STEP          },
	//{"TxPwr",  MENU_TXP           }, // was "TXP"
//	{"Rx DCS",  	MENU_R_DCS         }, // was "R_DCS"
//	{"Rx CTCS", 	MENU_R_CTCS        }, // was "R_CTCS"
//	{"Tx DCS",  	MENU_T_DCS         }, // was "T_DCS"
//	{"Tx CTCS", 	MENU_T_CTCS        }, // was "T_CTCS"
//	{"Tx ODir", 	MENU_SFT_D         }, // was "SFT_D"
//	{"Tx Offset", 	MENU_OFFSET        }, // was "OFFSET"
	//{"Bandwi", MENU_W_N           },
//	{"Scrambler", 	MENU_SCR           }, // was "SCR"
//	{"Busy CL", 	MENU_BCL           }, // was "BCL"
//	{"Compand", 	MENU_COMPAND       },
//	{"Demodu", MENU_AM            }, // was "AM"
//	{"Scan Add1", 	MENU_S_ADD1        },
//	{"Scan Add2", 	MENU_S_ADD2        },
//	{"Chan. Save", 	MENU_MEM_CH        }, // was "MEM-CH"
//	{"Chan. Dele", 	MENU_DEL_CH        }, // was "DEL-CH"
//	{"Chan. Name", 	MENU_MEM_NAME      },

	{"Scan List",  	MENU_S_LIST        },
	{"Scan List1", 	MENU_SLIST1        },
	{"Scan List2", 	MENU_SLIST2        },
	{"Scan Rev", 	MENU_SC_REV        },

//	{"F1 Short", 	MENU_F1SHRT        },
//	{"F1 Long", 	MENU_F1LONG        },
//	{"F2 Short", 	MENU_F2SHRT        },
//	{"F2 Long", 	MENU_F2LONG        },
//	{"M Long", 		MENU_MLONG         },

//	{"Key Lock", 	MENU_AUTOLK        }, // was "AUTOLk"
	{"Tx TimeOut", 	MENU_TOT           }, // was "TOT"
	{"Bat Save", 	MENU_SAVE          }, // was "SAVE"
	{"Mic",    		MENU_MIC           },
//#ifdef ENABLE_AUDIO_BAR
	{"Mic Bar", 	MENU_MIC_BAR       },
//#endif
	//{"ChDisp", MENU_MDF           }, // was "MDF"
//	{"Pow. onMsg", 	MENU_PONMSG        },
//	{"Bat Txt", 	MENU_BAT_TXT       },
	{"Back Lt", 	MENU_ABR           }, // was "ABR"
	{"BL Min",  	MENU_ABR_MIN       },
	{"BL Max",  	MENU_ABR_MAX       },

	{"LCD Ctr", 	MENU_CONTRAST      },

	{"Blt TRX", 	MENU_ABR_ON_TX_RX  },
	{"Beep",   		MENU_BEEP          },
	{"Roger",  		MENU_ROGER         },
	{"STE",    		MENU_STE           },
	{"RP STE", 		MENU_RP_STE        },
//	{"1 Call", 		MENU_1_CALL        },
//#ifdef ENABLE_ALARM
//	{"Alarm T", 	MENU_AL_MOD        },
//#endif
//#ifdef ENABLE_DTMF_CALLING
//	{"ANI ID", 		MENU_ANI_ID        },
//#endif
//	{"UP Code", 	MENU_UPCODE        },
//	{"DW Code", 	MENU_DWCODE        },
//	{"PTT ID", 		MENU_PTT_ID        },
//	{"D ST",   		MENU_D_ST          },
//#ifdef ENABLE_DTMF_CALLING
//    {"DTMF Resp", 	MENU_D_RSP         },
//	{"DTMF Hold", 	MENU_D_HOLD        },
//#endif
//	{"DTMF Prel", 	MENU_D_PRE         },
//#ifdef ENABLE_DTMF_CALLING
//	{"DTMF Decod", 	MENU_D_DCD         },
//	{"DTMF List", 	MENU_D_LIST        },
//#endif
//	{"DTMF Live", 	MENU_D_LIVE_DEC    }, // live DTMF decoder
//#ifdef ENABLE_VOX
//	{"VOX",    		MENU_VOX           },
//#endif
	{"Bat Vol", 	MENU_VOL           }, // was "VOL"
//	{"Rx Mode", 	MENU_TDR           },
//	{"SQL",    		MENU_SQL           },

	// hidden menu items from here on
	// enabled if pressing both the PTT and upper side button at power-on
	{"F Lock", 		MENU_F_LOCK        },
	{"Tx 200", 		MENU_200TX         }, // was "200TX"
	{"Tx 350", 		MENU_350TX         }, // was "350TX"
	{"Tx 500", 		MENU_500TX         }, // was "500TX"
	{"350 En", 		MENU_350EN         }, // was "350EN"
//	{"Scra En", 	MENU_SCREN         }, // was "SCREN"
//#ifdef ENABLE_F_CAL_MENU
//	{"Frq Cali", 	MENU_F_CALI        }, // reference xtal calibration
//#endif
//	{"Bat Cali", 	MENU_BATCAL        }, // battery voltage calibration
	{"Bat Type", 	MENU_BATTYP        }, // battery type 1600/2200mAh
//	{"Reset",  		MENU_RESET         }, // might be better to move this to the hidden menu items ?

//	{"",       0xff               }  // end of list - DO NOT delete or move this this
};

#define MENU_SETTINGS_SIZE 24

/*const char* const gSubMenu_OFF_ON[] =
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
*/

uint8_t mainSettingsCurrentMenu = 0;

uint16_t mainSettingsCurrentSubMenu = 0;
uint16_t mainSettingsSubmenuMin = 0;
uint16_t mainSettingsSubmenuSize = 0;

#define SETTINGS_TIMESHOW_SUB 1000
TickType_t mainSettingsSubMenuTime;
bool mainSettingsShowSubMenu;
bool mainSettingsSubMenuActive;
bool mainSettingsNeedToSave = false;

uint8_t getMainMenuID(void) {
	return MainMenuList[mainSettingsCurrentMenu].menu_id;
}

void MainSettingsMenu_showList() {
    uint8_t yPos = 15;
	const uint8_t offset = (uint8_t)Clamp(mainSettingsCurrentMenu - 2, 0, MENU_SETTINGS_SIZE - 5);

    for (uint8_t i = 0; i < 5; i++) {
		if ( (i + offset) < MENU_SETTINGS_SIZE ) {
			bool isFill = mainSettingsCurrentMenu == i + offset ? true : false;
		
			UI_printf(&font_small, TEXT_ALIGN_LEFT, 8, 0, yPos - 1, true, false, "%0.2i", (i + 1) + offset);
		
			UI_printf(&font_10, TEXT_ALIGN_LEFT, 18, 0, yPos, !isFill, isFill,
				"%s", MainMenuList[i + offset].name);
			yPos += 11;
		}
    }

    yPos = 10;
    yPos += (uint8_t)((( (100 * mainSettingsCurrentMenu) / MENU_SETTINGS_SIZE ) / 100.0) * 54);

    UI_drawFastVLine(1, yPos - 1, 3, true);
    UI_drawFastVLine(3, yPos - 1, 3, true);
}

void MainSettingsMenu_showSubListCalc(uint8_t *yPos, uint8_t *listCount, uint8_t *offset) {
    if (mainSettingsSubmenuSize <= 2 ) {
        if (mainSettingsSubmenuSize == 2 ) {
            *yPos = *yPos + 6;
        } else if (mainSettingsSubmenuSize == 1 ) {
            *yPos = *yPos + 11;
        }
        *listCount = (uint8_t)mainSettingsSubmenuSize + 1;
    } else {
		*offset = (uint8_t)Clamp(mainSettingsCurrentSubMenu - 2, 0, mainSettingsSubmenuSize - 3);
    }
}


void MainSettingsMenu_showSubList(const char* const subList[]) {
	uint8_t yPos = 21;
    uint8_t listCount = 4;
	uint8_t offset = 0;
	MainSettingsMenu_showSubListCalc(&yPos, &listCount, &offset);
    for (uint8_t i = 0; i < listCount; i++) {
		bool isFill = mainSettingsCurrentSubMenu == i + offset ? true : false;
		UI_printf(&font_10, TEXT_ALIGN_CENTER, SUB_MENU_X, 125, yPos, !isFill, isFill,
			"%s", subList[i + offset]);
        yPos += 11;
    }
}

void MainSettingsMenu_showSubListValue(void) {
	uint8_t yPos = 21;
    uint8_t listCount = 4;
	uint8_t offset = 0;
	MainSettingsMenu_showSubListCalc(&yPos, &listCount, &offset);    
    for (uint8_t i = 0; i < listCount; i++) {
		bool isFill = mainSettingsCurrentSubMenu == i + offset ? true : false;

		if ( (i + offset == 0) && (getMainMenuID() != MENU_STEP) ) {
			UI_printf(&font_10, TEXT_ALIGN_CENTER, SUB_MENU_X, 125, yPos, !isFill, isFill, "OFF" );
		} else {

			switch (getMainMenuID())
			{
				case MENU_STEP: {
					uint16_t step = gStepFrequencyTable[FREQUENCY_GetStepIdxFromSortedIdx(i + offset)];
					UI_printf(&font_10, TEXT_ALIGN_CENTER, SUB_MENU_X, 125, yPos, !isFill, isFill, "%d.%02ukHz", step / 100, step % 100 );
					break;
				}

				case MENU_R_DCS:
				case MENU_T_DCS:
					if (mainSettingsCurrentSubMenu < 105) {
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


void MainSettingsMenu_showSubListNumber() {
	uint8_t yPos = 21;
    uint8_t listCount = 4;
	uint8_t offset = 0;
	MainSettingsMenu_showSubListCalc(&yPos, &listCount, &offset);
    for (uint8_t i = 0; i < listCount; i++) {
		UI_printf(&font_10, TEXT_ALIGN_CENTER, SUB_MENU_X, 125, yPos,
			mainSettingsCurrentSubMenu == i + offset ? false : true,
			mainSettingsCurrentSubMenu == i + offset ? true : false,
			"%i", (i + offset));
        yPos += 11;
    }
}

int MainSettingsMenu_GetLimits(uint8_t menu_id, uint16_t *pMin, uint16_t *pMax);

void MainSettingsMenu_loadSubList() {

	UI_fillRect(SUB_MENU_X - 2, 11, 61, 48, false);
    if ( mainSettingsSubMenuActive ) {
    	UI_drawRoundRect(SUB_MENU_X - 2, 11, 61, 48, 4, true);
	}
    UI_drawRoundRect(SUB_MENU_X - 1, 12, 59, 46, 4, true);

	MainSettingsMenu_GetLimits(getMainMenuID(), &mainSettingsSubmenuMin, &mainSettingsSubmenuSize);
	
	bool showNumber = false;
	bool showValue = false;

	switch (getMainMenuID())
    {

		case MENU_ABR:
			//gSubMenu_BACKLIGHT
			
            MainSettingsMenu_showSubList(gSubMenu_BACKLIGHT);
			break;

		case MENU_SCR:
			//gSubMenu_SCRAMBLER
			
            MainSettingsMenu_showSubList(gSubMenu_SCRAMBLER);
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
			
            MainSettingsMenu_showSubList(gSubMenu_F_LOCK);
			break;

		case MENU_PTT_ID:
			
            MainSettingsMenu_showSubList(gSubMenu_PTT_ID);
			break;

		case MENU_SFT_D:
			//gSubMenu_SFT_D
			
            MainSettingsMenu_showSubList(gSubMenu_SFT_D);
			break;

		case MENU_TDR:
			//gSubMenu_RXMode
			
            MainSettingsMenu_showSubList(gSubMenu_RXMode);
			break;

		case MENU_ROGER:
			//gSubMenu_ROGER
			
            MainSettingsMenu_showSubList(gSubMenu_ROGER);
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
			
            MainSettingsMenu_showSubList(gSubMenu_RESET);
			break;

		case MENU_COMPAND:
		case MENU_ABR_ON_TX_RX:
			//gSubMenu_RX_TX
			
            MainSettingsMenu_showSubList(gSubMenu_RX_TX);
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
		case MENU_BCL:
		case MENU_BEEP:
		case MENU_AUTOLK:
		case MENU_STE:
			//gSubMenu_OFF_ON
			
        	MainSettingsMenu_showSubList(gSubMenu_OFF_ON);
			break;

		case MENU_TOT:
			//gSubMenu_TOT
			
            MainSettingsMenu_showSubList(gSubMenu_TOT);
			break;

		case MENU_SAVE:
			//gSubMenu_SAVE
			
            MainSettingsMenu_showSubList(gSubMenu_SAVE);
			break;

		case MENU_BAT_TXT:
			//gSubMenu_BAT_TXT
			
            MainSettingsMenu_showSubList(gSubMenu_BAT_TXT);
			break;

		case MENU_PONMSG:
			
            MainSettingsMenu_showSubList(gSubMenu_PONMSG);
			break;

		case MENU_SC_REV:
			
            MainSettingsMenu_showSubList(gSubMenu_SC_REV);
			break;


		case MENU_OFFSET:
			//TODO:
			//MainSettingsMenu_showSubValue();
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
			const uint16_t gBatteryPercentage = (uint16_t)BATTERY_VoltsToPercent(gBatteryVoltageAverage);
			UI_printf(&font_10, TEXT_ALIGN_CENTER, SUB_MENU_X, 125, 26, true, false,	"%u.%02uV", gBatteryVoltageAverage / 100, gBatteryVoltageAverage % 100);
			UI_printf(&font_10, TEXT_ALIGN_CENTER, SUB_MENU_X, 125, 37, true, false,	"%3i%%", gBatteryPercentage);
			UI_printf(&font_10, TEXT_ALIGN_CENTER, SUB_MENU_X, 125, 48, true, false,	"%dmA", gBatteryCurrent);
			break;
		}

		case MENU_BATTYP:
			// 0 .. 1;
			//gSubMenu_BATTYP
			
            MainSettingsMenu_showSubList(gSubMenu_BATTYP);
			break;


        default:
            break;
    }
	if (showValue) {
		MainSettingsMenu_showSubListValue();
	} else if (showNumber) {
		MainSettingsMenu_showSubListNumber();
	}

}

void MainSettingsMenu_loadSubListValues() {

	mainSettingsSubmenuMin = 0;

	switch (getMainMenuID())
	{
		case MENU_SQL:
			mainSettingsCurrentSubMenu = gEeprom.SQUELCH_LEVEL;
			break;

		case MENU_STEP:
			mainSettingsCurrentSubMenu = (uint16_t)FREQUENCY_GetSortedIdxFromStepIdx(gTxVfo->STEP_SETTING);
			break;

		/*case MENU_TXP:
			mainSettingsCurrentSubMenu = gTxVfo->OUTPUT_POWER;
			break;*/

		/*case MENU_RESET:
			mainSettingsCurrentSubMenu = 0;
			break;*/

		case MENU_R_DCS:
		case MENU_R_CTCS:
		{
			DCS_CodeType_t type = gTxVfo->freq_config_RX.CodeType;
			uint8_t code = gTxVfo->freq_config_RX.Code;
			/*int menuid = MainMenuList[mainSettingsCurrentMenu].menu_id;

			if(gScanUseCssResult) {
				gScanUseCssResult = false;
				type = gScanCssResultType;
				code = gScanCssResultCode;
			}*/
			if((getMainMenuID() == MENU_R_CTCS) ^ (type==CODE_TYPE_CONTINUOUS_TONE)) { //not the same type
				mainSettingsCurrentSubMenu = 0;
				break;
			}

			switch (type) {
				case CODE_TYPE_CONTINUOUS_TONE:
				case CODE_TYPE_DIGITAL:
					mainSettingsCurrentSubMenu = code + 1;
					break;
				case CODE_TYPE_REVERSE_DIGITAL:
					mainSettingsCurrentSubMenu = code + 105;
					break;
				default:
					mainSettingsCurrentSubMenu = 0;
					break;
			}
		break;
		}

		case MENU_T_DCS:
			switch (gTxVfo->freq_config_TX.CodeType)
			{
				case CODE_TYPE_DIGITAL:
					mainSettingsCurrentSubMenu = gTxVfo->freq_config_TX.Code + 1;
					break;
				case CODE_TYPE_REVERSE_DIGITAL:
					mainSettingsCurrentSubMenu = gTxVfo->freq_config_TX.Code + 105;
					break;
				default:
					mainSettingsCurrentSubMenu = 0;
					break;
			}
			break;

		case MENU_T_CTCS:
			mainSettingsCurrentSubMenu = (gTxVfo->freq_config_TX.CodeType == CODE_TYPE_CONTINUOUS_TONE) ? gTxVfo->freq_config_TX.Code + 1 : 0;
			break;

		case MENU_SFT_D:
			mainSettingsCurrentSubMenu = gTxVfo->TX_OFFSET_FREQUENCY_DIRECTION;
			break;

		case MENU_OFFSET:
			mainSettingsCurrentSubMenu = (uint16_t)(gTxVfo->TX_OFFSET_FREQUENCY);
			break;

		/*case MENU_W_N:
			mainSettingsCurrentSubMenu = gTxVfo->CHANNEL_BANDWIDTH;
			break;*/

		case MENU_SCR:
			mainSettingsCurrentSubMenu = gTxVfo->SCRAMBLING_TYPE;
			break;

		case MENU_BCL:
			mainSettingsCurrentSubMenu = gTxVfo->BUSY_CHANNEL_LOCK;
			break;

		/*case MENU_MEM_CH:
			#if 0
				mainSettingsCurrentSubMenu = gEeprom.MrChannel[0];
			#else
				mainSettingsCurrentSubMenu = gEeprom.MrChannel[gEeprom.TX_VFO];
			#endif
			break;

		case MENU_MEM_NAME:
			mainSettingsCurrentSubMenu = gEeprom.MrChannel[gEeprom.TX_VFO];
			break;

		case MENU_SAVE:
			mainSettingsCurrentSubMenu = gEeprom.BATTERY_SAVE;
			break;

#ifdef ENABLE_VOX
		case MENU_VOX:
			mainSettingsCurrentSubMenu = gEeprom.VOX_SWITCH ? gEeprom.VOX_LEVEL + 1 : 0;
			break;
#endif

		case MENU_ABR:
			mainSettingsCurrentSubMenu = gEeprom.BACKLIGHT_TIME;
			break;

		case MENU_ABR_MIN:
			mainSettingsCurrentSubMenu = gEeprom.BACKLIGHT_MIN;
			break;

		case MENU_ABR_MAX:
			mainSettingsCurrentSubMenu = gEeprom.BACKLIGHT_MAX;
			break;

		case MENU_CONTRAST:
			mainSettingsCurrentSubMenu = gEeprom.LCD_CONTRAST;
			break;		

		case MENU_ABR_ON_TX_RX:
			mainSettingsCurrentSubMenu = gSetting_backlight_on_tx_rx;
			break;*/

		case MENU_TDR:
			mainSettingsCurrentSubMenu = (uint16_t)((gEeprom.DUAL_WATCH != DUAL_WATCH_OFF) + (gEeprom.CROSS_BAND_RX_TX != CROSS_BAND_OFF) * 2);
			break;

		/*case MENU_BEEP:
			mainSettingsCurrentSubMenu = gEeprom.BEEP_CONTROL;
			break;*/

		case MENU_TOT:
			mainSettingsCurrentSubMenu = gEeprom.TX_TIMEOUT_TIMER;
			break;

		/*case MENU_SC_REV:
			mainSettingsCurrentSubMenu = gEeprom.SCAN_RESUME_MODE;
			break;*/

		/*case MENU_MDF:
			mainSettingsCurrentSubMenu = gEeprom.CHANNEL_DISPLAY_MODE;
			break;*/

		/*case MENU_AUTOLK:
			mainSettingsCurrentSubMenu = gEeprom.AUTO_KEYPAD_LOCK;
			break;

		case MENU_S_ADD1:
			mainSettingsCurrentSubMenu = gTxVfo->SCANLIST1_PARTICIPATION;
			break;

		case MENU_S_ADD2:
			mainSettingsCurrentSubMenu = gTxVfo->SCANLIST2_PARTICIPATION;
			break;

		case MENU_STE:
			mainSettingsCurrentSubMenu = gEeprom.TAIL_TONE_ELIMINATION;
			break;

		case MENU_RP_STE:
			mainSettingsCurrentSubMenu = gEeprom.REPEATER_TAIL_TONE_ELIMINATION;
			break;

		case MENU_MIC:
			mainSettingsCurrentSubMenu = gEeprom.MIC_SENSITIVITY;
			break;

#ifdef ENABLE_AUDIO_BAR
		case MENU_MIC_BAR:
			mainSettingsCurrentSubMenu = gSetting_mic_bar;
			break;
#endif
*/
		case MENU_COMPAND:
			mainSettingsCurrentSubMenu = gTxVfo->Compander;
			return;

		/*case MENU_1_CALL:
			mainSettingsCurrentSubMenu = gEeprom.CHAN_1_CALL;
			break;

		case MENU_S_LIST:
			mainSettingsCurrentSubMenu = gEeprom.SCAN_LIST_DEFAULT;
			break;

		case MENU_SLIST1:
			mainSettingsCurrentSubMenu = RADIO_FindNextChannel(0, 1, true, 0);
			break;

		case MENU_SLIST2:
			mainSettingsCurrentSubMenu = RADIO_FindNextChannel(0, 1, true, 1);
			break;

		#ifdef ENABLE_ALARM
			case MENU_AL_MOD:
				mainSettingsCurrentSubMenu = gEeprom.ALARM_MODE;
				break;
		#endif

		case MENU_D_ST:
			mainSettingsCurrentSubMenu = gEeprom.DTMF_SIDE_TONE;
			break;

#ifdef ENABLE_DTMF_CALLING
		case MENU_D_RSP:
			mainSettingsCurrentSubMenu = gEeprom.DTMF_DECODE_RESPONSE;
			break;

		case MENU_D_HOLD:
			mainSettingsCurrentSubMenu = gEeprom.DTMF_auto_reset_time;
			break;
#endif
		case MENU_D_PRE:
			mainSettingsCurrentSubMenu = gEeprom.DTMF_PRELOAD_TIME / 10;
			break;
*/
		case MENU_PTT_ID:
			mainSettingsCurrentSubMenu = gTxVfo->DTMF_PTT_ID_TX_MODE;
			break;

		/*case MENU_BAT_TXT:
			mainSettingsCurrentSubMenu = gSetting_battery_text;
			return;*/

#ifdef ENABLE_DTMF_CALLING
		case MENU_D_DCD:
			mainSettingsCurrentSubMenu = gTxVfo->DTMF_DECODING_ENABLE;
			break;

		case MENU_D_LIST:
			mainSettingsCurrentSubMenu = gDTMF_chosen_contact + 1;
			break;
#endif
		case MENU_D_LIVE_DEC:
			mainSettingsCurrentSubMenu = gSetting_live_DTMF_decoder;
			break;

		/*case MENU_PONMSG:
			mainSettingsCurrentSubMenu = gEeprom.POWER_ON_DISPLAY_MODE;
			break;

		case MENU_ROGER:
			mainSettingsCurrentSubMenu = gEeprom.ROGER;
			break;

		case MENU_AM:
			mainSettingsCurrentSubMenu = gTxVfo->Modulation;
			break;*/

		/*case MENU_DEL_CH:
			#if 0
				mainSettingsCurrentSubMenu = RADIO_FindNextChannel(gEeprom.MrChannel[0], 1, false, 1);
			#else
				mainSettingsCurrentSubMenu = RADIO_FindNextChannel(gEeprom.MrChannel[gEeprom.TX_VFO], 1, false, 1);
			#endif
			break;

		case MENU_350TX:
			mainSettingsCurrentSubMenu = gSetting_350TX;
			break;

		case MENU_F_LOCK:
			mainSettingsCurrentSubMenu = gSetting_F_LOCK;
			break;

		case MENU_200TX:
			mainSettingsCurrentSubMenu = gSetting_200TX;
			break;

		case MENU_500TX:
			mainSettingsCurrentSubMenu = gSetting_500TX;
			break;

		case MENU_350EN:
			mainSettingsCurrentSubMenu = gSetting_350EN;
			break;

		case MENU_SCREN:
			mainSettingsCurrentSubMenu = gSetting_ScrambleEnable;
			break;

		#ifdef ENABLE_F_CAL_MENU
			case MENU_F_CALI:
				mainSettingsCurrentSubMenu = gEeprom.BK4819_XTAL_FREQ_LOW;
				break;
		#endif

		case MENU_BATCAL:
			mainSettingsCurrentSubMenu = gBatteryCalibration[3];
			break;

		case MENU_BATTYP:
			mainSettingsCurrentSubMenu = gEeprom.BATTERY_TYPE;
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
			uint8_t id = *fun[MainMenuList[mainSettingsCurrentMenu].menu_id-MENU_F1SHRT];

			for(int i = 0; i < gSubMenu_SIDEFUNCTIONS_size; i++) {
				if(gSubMenu_SIDEFUNCTIONS[i].id==id) {
					mainSettingsCurrentSubMenu = i;
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


int MainSettingsMenu_GetLimits(uint8_t menu_id, uint16_t *pMin, uint16_t *pMax)
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
			*pMin = 0;
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

void mainSettingsCompareValuesInt(uint32_t value1) {
	if ( mainSettingsNeedToSave ) return;
	mainSettingsNeedToSave = (value1 |= mainSettingsCurrentSubMenu);
}

void MainSettings_saveSetting(void) {
	FREQ_Config_t *pConfig = &gTxVfo->freq_config_RX;

	switch (getMainMenuID())
	{
		default:
			return;

		case MENU_SQL:
			gEeprom.SQUELCH_LEVEL = (uint8_t)mainSettingsCurrentSubMenu;
			//gVfoConfigureMode     = VFO_CONFIGURE;
			break;

		case MENU_STEP:
			gTxVfo->STEP_SETTING = FREQUENCY_GetStepIdxFromSortedIdx((uint8_t)mainSettingsCurrentSubMenu);
			if (IS_FREQ_CHANNEL(gTxVfo->CHANNEL_SAVE))
			{
				//gRequestSaveChannel = 1;
				return;
			}
			return;

		/*case MENU_TXP:
			gTxVfo->OUTPUT_POWER = mainSettingsCurrentSubMenu;
			gRequestSaveChannel = 1;
			return;*/

		case MENU_T_DCS:
			pConfig = &gTxVfo->freq_config_TX;

			// Fallthrough

		case MENU_R_DCS: {
			if (mainSettingsCurrentSubMenu == 0) {
				if (pConfig->CodeType == CODE_TYPE_CONTINUOUS_TONE) {
					return;
				}
				pConfig->Code = 0;
				pConfig->CodeType = CODE_TYPE_OFF;
			}
			else if (mainSettingsCurrentSubMenu < 105) {
				pConfig->CodeType = CODE_TYPE_DIGITAL;
				pConfig->Code = (uint8_t)mainSettingsCurrentSubMenu - 1;
			}
			else {
				pConfig->CodeType = CODE_TYPE_REVERSE_DIGITAL;
				pConfig->Code = (uint8_t)mainSettingsCurrentSubMenu - 105;
			}

			//gRequestSaveChannel = 1;
			return;
		}
		case MENU_T_CTCS:
			pConfig = &gTxVfo->freq_config_TX;
			[[fallthrough]];
		case MENU_R_CTCS: {
			if (mainSettingsCurrentSubMenu == 0) {
				if (pConfig->CodeType != CODE_TYPE_CONTINUOUS_TONE) {
					return;
				}
				pConfig->Code     = 0;
				pConfig->CodeType = CODE_TYPE_OFF;
			}
			else {
				pConfig->Code     = (uint8_t)mainSettingsCurrentSubMenu - 1;
				pConfig->CodeType = CODE_TYPE_CONTINUOUS_TONE;
			}

			//gRequestSaveChannel = 1;
			return;
		}
		case MENU_SFT_D:
			gTxVfo->TX_OFFSET_FREQUENCY_DIRECTION = (uint8_t)mainSettingsCurrentSubMenu;
			//gRequestSaveChannel                   = 1;
			return;

		case MENU_OFFSET:
			gTxVfo->TX_OFFSET_FREQUENCY = (uint32_t)mainSettingsCurrentSubMenu;
			//gRequestSaveChannel         = 1;
			return;

		case MENU_W_N:
			gTxVfo->CHANNEL_BANDWIDTH = (uint8_t)mainSettingsCurrentSubMenu;
			//gRequestSaveChannel       = 1;
			return;

		case MENU_SCR:
			gTxVfo->SCRAMBLING_TYPE = (uint8_t)mainSettingsCurrentSubMenu;
			//gRequestSaveChannel     = 1;
			return;

		case MENU_BCL:
			gTxVfo->BUSY_CHANNEL_LOCK = (uint8_t)mainSettingsCurrentSubMenu;
			//gRequestSaveChannel       = 1;
			return;

		case MENU_MEM_CH:
			gTxVfo->CHANNEL_SAVE = (uint8_t)mainSettingsCurrentSubMenu;
			#if 0
				gEeprom.MrChannel[0] = (uint8_t)mainSettingsCurrentSubMenu;
			#else
				gEeprom.MrChannel[gEeprom.TX_VFO] = (uint8_t)mainSettingsCurrentSubMenu;
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

			//SETTINGS_SaveChannelName(mainSettingsCurrentSubMenu, edit);
			return;

		case MENU_SAVE:
			gEeprom.BATTERY_SAVE = (uint8_t)mainSettingsCurrentSubMenu;
			break;

		#ifdef ENABLE_VOX
			case MENU_VOX:
				gEeprom.VOX_SWITCH = mainSettingsCurrentSubMenu != 0;
				if (gEeprom.VOX_SWITCH)
					gEeprom.VOX_LEVEL = (uint8_t)mainSettingsCurrentSubMenu - 1;
				//SETTINGS_LoadCalibration();
				//gFlagReconfigureVfos = true;
				//gUpdateStatus        = true;
				break;
		#endif

		case MENU_ABR:
			gEeprom.BACKLIGHT_TIME = (uint8_t)mainSettingsCurrentSubMenu;
			app_push_message(APP_MSG_BACKLIGHT);
			break;

		case MENU_ABR_MIN:
			gEeprom.BACKLIGHT_MIN = (uint8_t)mainSettingsCurrentSubMenu;
			gEeprom.BACKLIGHT_MAX = (uint8_t)MAX(mainSettingsCurrentSubMenu + 1 , gEeprom.BACKLIGHT_MAX);
			break;

		case MENU_ABR_MAX:
			gEeprom.BACKLIGHT_MAX = (uint8_t)mainSettingsCurrentSubMenu;
			gEeprom.BACKLIGHT_MIN = (uint8_t)MIN(mainSettingsCurrentSubMenu - 1, gEeprom.BACKLIGHT_MIN);
			break;			

		case MENU_CONTRAST:
			gEeprom.LCD_CONTRAST = (uint8_t)mainSettingsCurrentSubMenu;
			//ST7565_SetContrast(gEeprom.LCD_CONTRAST);
			break;

		case MENU_ABR_ON_TX_RX:
			gSetting_backlight_on_tx_rx = mainSettingsCurrentSubMenu;
			break;

		case MENU_TDR:
			gEeprom.DUAL_WATCH = (uint8_t)((gEeprom.TX_VFO + 1) * (mainSettingsCurrentSubMenu & 1));
			gEeprom.CROSS_BAND_RX_TX = (uint8_t)((gEeprom.TX_VFO + 1) * ((mainSettingsCurrentSubMenu & 2) > 0));

			//gFlagReconfigureVfos = true;
			//gUpdateStatus        = true;
			break;

		case MENU_BEEP:
			gEeprom.BEEP_CONTROL = mainSettingsCurrentSubMenu;
			break;

		case MENU_TOT:
			gEeprom.TX_TIMEOUT_TIMER = (uint8_t)mainSettingsCurrentSubMenu;
			break;

		case MENU_SC_REV:
			gEeprom.SCAN_RESUME_MODE = (uint8_t)mainSettingsCurrentSubMenu;
			break;

		/*case MENU_MDF:
			gEeprom.CHANNEL_DISPLAY_MODE = mainSettingsCurrentSubMenu;
			break;*/

		case MENU_AUTOLK:
			gEeprom.AUTO_KEYPAD_LOCK = mainSettingsCurrentSubMenu;
			//gKeyLockCountdown        = 30;
			break;

		case MENU_S_ADD1:
			gTxVfo->SCANLIST1_PARTICIPATION = (uint8_t)mainSettingsCurrentSubMenu;
			//SETTINGS_UpdateChannel(gTxVfo->CHANNEL_SAVE, gTxVfo, true);
			//gVfoConfigureMode = VFO_CONFIGURE;
			//gFlagResetVfos    = true;
			return;

		case MENU_S_ADD2:
			gTxVfo->SCANLIST2_PARTICIPATION = (uint8_t)mainSettingsCurrentSubMenu;
			//SETTINGS_UpdateChannel(gTxVfo->CHANNEL_SAVE, gTxVfo, true);
			//gVfoConfigureMode = VFO_CONFIGURE;
			//gFlagResetVfos    = true;
			return;

		case MENU_STE:
			gEeprom.TAIL_TONE_ELIMINATION = mainSettingsCurrentSubMenu;
			break;

		case MENU_RP_STE:
			gEeprom.REPEATER_TAIL_TONE_ELIMINATION = (uint8_t)mainSettingsCurrentSubMenu;
			break;

		case MENU_MIC:
			gEeprom.MIC_SENSITIVITY = (uint8_t)mainSettingsCurrentSubMenu;
			//SETTINGS_LoadCalibration();
			//gFlagReconfigureVfos = true;
			break;

		#ifdef ENABLE_AUDIO_BAR
			case MENU_MIC_BAR:
				gSetting_mic_bar = mainSettingsCurrentSubMenu;
				break;
		#endif

		case MENU_COMPAND:
			gTxVfo->Compander = (uint8_t)mainSettingsCurrentSubMenu;
			//SETTINGS_UpdateChannel(gTxVfo->CHANNEL_SAVE, gTxVfo, true);
			//gVfoConfigureMode = VFO_CONFIGURE;
			//gFlagResetVfos    = true;
//			gRequestSaveChannel = 1;
			return;

		case MENU_1_CALL:
			gEeprom.CHAN_1_CALL = (uint8_t)mainSettingsCurrentSubMenu;
			break;

		case MENU_S_LIST:
			gEeprom.SCAN_LIST_DEFAULT = (uint8_t)mainSettingsCurrentSubMenu;
			break;

		#ifdef ENABLE_ALARM
			case MENU_AL_MOD:
				gEeprom.ALARM_MODE = mainSettingsCurrentSubMenu;
				break;
		#endif

		case MENU_D_ST:
			gEeprom.DTMF_SIDE_TONE = mainSettingsCurrentSubMenu;
			break;

#ifdef ENABLE_DTMF_CALLING
		case MENU_D_RSP:
			gEeprom.DTMF_DECODE_RESPONSE = mainSettingsCurrentSubMenu;
			break;

		case MENU_D_HOLD:
			gEeprom.DTMF_auto_reset_time = mainSettingsCurrentSubMenu;
			break;
#endif
		case MENU_D_PRE:
			gEeprom.DTMF_PRELOAD_TIME = mainSettingsCurrentSubMenu * 10;
			break;

		case MENU_PTT_ID:
			gTxVfo->DTMF_PTT_ID_TX_MODE = mainSettingsCurrentSubMenu;
			//gRequestSaveChannel         = 1;
			return;

		case MENU_BAT_TXT:
			gSetting_battery_text = (uint8_t)mainSettingsCurrentSubMenu;
			break;

#ifdef ENABLE_DTMF_CALLING
		case MENU_D_DCD:
			gTxVfo->DTMF_DECODING_ENABLE = mainSettingsCurrentSubMenu;
			//DTMF_clear_RX();
			//gRequestSaveChannel = 1;
			return;
#endif

		case MENU_D_LIVE_DEC:
			gSetting_live_DTMF_decoder = mainSettingsCurrentSubMenu;
			/*gDTMF_RX_live_timeout = 0;
			memset(gDTMF_RX_live, 0, sizeof(gDTMF_RX_live));
			if (!gSetting_live_DTMF_decoder)
				BK4819_DisableDTMF();
			gFlagReconfigureVfos     = true;
			//gUpdateStatus            = true;*/
			break;

#ifdef ENABLE_DTMF_CALLING
		case MENU_D_LIST:
			gDTMF_chosen_contact = mainSettingsCurrentSubMenu - 1;
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
			gEeprom.POWER_ON_DISPLAY_MODE = mainSettingsCurrentSubMenu;
			break;

		case MENU_ROGER:
			gEeprom.ROGER = mainSettingsCurrentSubMenu;
			break;

		case MENU_AM:
			gTxVfo->Modulation     = mainSettingsCurrentSubMenu;
			//gRequestSaveChannel = 1;
			return;

		case MENU_DEL_CH:
			/*SETTINGS_UpdateChannel(mainSettingsCurrentSubMenu, NULL, false);
			gVfoConfigureMode = VFO_CONFIGURE_RELOAD;
			gFlagResetVfos    = true;*/
			return;

		case MENU_RESET:
			//SETTINGS_FactoryReset(mainSettingsCurrentSubMenu);
			return;

		case MENU_350TX:
			gSetting_350TX = mainSettingsCurrentSubMenu;
			break;

		case MENU_F_LOCK: {
			/*if(mainSettingsCurrentSubMenu == F_LOCK_NONE) { // select 10 times to enable
				gUnlockAllTxConfCnt++;
				if(gUnlockAllTxConfCnt < 10)
					return;
			}
			else
				gUnlockAllTxConfCnt = 0;

			gSetting_F_LOCK = mainSettingsCurrentSubMenu;*/
			break;
		}
		case MENU_200TX:
			//gSetting_200TX = mainSettingsCurrentSubMenu;
			break;

		case MENU_500TX:
			//gSetting_500TX = mainSettingsCurrentSubMenu;
			break;

		case MENU_350EN:
			/*gSetting_350EN       = mainSettingsCurrentSubMenu;
			gVfoConfigureMode    = VFO_CONFIGURE_RELOAD;
			gFlagResetVfos       = true;*/
			break;

		case MENU_SCREN:
			/*gSetting_ScrambleEnable = mainSettingsCurrentSubMenu;
			gFlagReconfigureVfos    = true;*/
			break;

		#ifdef ENABLE_F_CAL_MENU
			case MENU_F_CALI:
				//writeXtalFreqCal(mainSettingsCurrentSubMenu, true);
				return;
		#endif

		case MENU_BATCAL:
		{																 // voltages are averages between discharge curves of 1600 and 2200 mAh
			// gBatteryCalibration[0] = (520ul * mainSettingsCurrentSubMenu) / 760;  // 5.20V empty, blinking above this value, reduced functionality below
			// gBatteryCalibration[1] = (689ul * mainSettingsCurrentSubMenu) / 760;  // 6.89V,  ~5%, 1 bars above this value
			// gBatteryCalibration[2] = (724ul * mainSettingsCurrentSubMenu) / 760;  // 7.24V, ~17%, 2 bars above this value
			gBatteryCalibration[3] =          mainSettingsCurrentSubMenu;         // 7.6V,  ~29%, 3 bars above this value
			// gBatteryCalibration[4] = (771ul * mainSettingsCurrentSubMenu) / 760;  // 7.71V, ~65%, 4 bars above this value
			// gBatteryCalibration[5] = 2300;
			//SETTINGS_SaveBatteryCalibration(gBatteryCalibration);
			return;
		}

		case MENU_BATTYP:
			gEeprom.BATTERY_TYPE = mainSettingsCurrentSubMenu;
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

void MainSettings_initFunction() {
    mainSettingsSubMenuTime = xTaskGetTickCount();
    mainSettingsShowSubMenu = false;
    mainSettingsSubMenuActive = false;
}

void MainSettings_renderFunction() {

    UI_drawFastVLine(2, 9, 54, true);

    MainSettingsMenu_showList();

    if (mainSettingsShowSubMenu) {
        MainSettingsMenu_loadSubList();
    } else {
        if (xTaskGetTickCount() - mainSettingsSubMenuTime > pdMS_TO_TICKS(SETTINGS_TIMESHOW_SUB)) {
			if( GUI_inputGetSize() == 1 ) {
				const uint32_t inputValue = GUI_inputGetNumberClear();
				if ( inputValue > 0) {
					mainSettingsCurrentMenu = (uint8_t)(inputValue - 1);
					mainSettingsShowSubMenu = false;
					mainSettingsSubMenuTime = xTaskGetTickCount();
				}
			} else {
            	mainSettingsShowSubMenu = true;
            	mainSettingsCurrentSubMenu = 0;
				MainSettingsMenu_loadSubListValues();
			}
        }
    }

	//UI_printf(7, 125, 60, true, false,	"%i %i", mainSettingsCurrentSubMenu, mainSettingsSubmenuSize);
}

void MainSettings_keyHandlerFunction(KEY_Code_t key, KEY_State_t state) {

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
				if ( !mainSettingsSubMenuActive ) {
					GUI_inputAppendKey(key, 2, false);
					if( GUI_inputGetSize() == 2 ) {
						const uint32_t inputValue = GUI_inputGetNumberClear();
						if ( inputValue > 0 && inputValue < MENU_SETTINGS_SIZE ) {
							mainSettingsCurrentMenu = (uint8_t)(inputValue - 1);
						}
					}
					mainSettingsSubMenuTime = xTaskGetTickCount();
					mainSettingsShowSubMenu = false;
				}
			}
			break;
		case KEY_UP:
			if ( state == KEY_PRESSED || state == KEY_LONG_PRESSED_CONT ) {
				if ( mainSettingsSubMenuActive ) {
					if ( mainSettingsCurrentSubMenu > mainSettingsSubmenuMin ) {
						mainSettingsCurrentSubMenu--;
					} else {
						mainSettingsCurrentSubMenu = mainSettingsSubmenuSize;
					}
				} else {
					if ( mainSettingsCurrentMenu > 0 ) {
						mainSettingsCurrentMenu--;
					} else {
						mainSettingsCurrentMenu = MENU_SETTINGS_SIZE - 1;
					}
					mainSettingsSubMenuTime = xTaskGetTickCount();
					mainSettingsShowSubMenu = false;
				}
			}
			break;
		case KEY_DOWN:
			if ( state == KEY_PRESSED || state == KEY_LONG_PRESSED_CONT ) {
				if ( mainSettingsSubMenuActive ) {
					if ( mainSettingsCurrentSubMenu < mainSettingsSubmenuSize ) {
						mainSettingsCurrentSubMenu++;
					} else {
						mainSettingsCurrentSubMenu = mainSettingsSubmenuMin;
					}
				} else {
					if ( mainSettingsCurrentMenu < MENU_SETTINGS_SIZE - 1 ) {
						mainSettingsCurrentMenu++;
					} else {
						mainSettingsCurrentMenu = 0;
					}
					mainSettingsSubMenuTime = xTaskGetTickCount();
					mainSettingsShowSubMenu = false;
				}
			}
			break;
		case KEY_MENU:
			if ( state == KEY_PRESSED ) {
				if ( mainSettingsSubMenuActive ) {
					//save submenu
					MainSettings_saveSetting();
					mainSettingsSubMenuActive = false;
					app_push_message(APP_MSG_TIMEOUT_RETURN_MAIN);
				} else {
					mainSettingsSubMenuActive = true;
					mainSettingsShowSubMenu = true;
					mainSettingsCurrentSubMenu = 0;
					MainSettingsMenu_loadSubListValues();
					app_push_message(APP_MSG_TIMEOUT_NO_RETURN_MAIN);
				}
			}
			break;
		case KEY_EXIT:
			if ( state == KEY_PRESSED ) {
				if ( mainSettingsSubMenuActive ) {
					mainSettingsSubMenuActive = false;
				} else {
					load_application(APP_MAIN_VFO);
				}
			}
			break;
		default:
			break;
	}

}


app_t APPMainSettings = {
    .showStatusLine = true,
    .init = MainSettings_initFunction,
    .render = MainSettings_renderFunction,
    .keyHandler = MainSettings_keyHandlerFunction
};
