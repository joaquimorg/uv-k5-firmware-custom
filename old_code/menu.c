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
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"

#include "../app/dtmf.h"
#include "../app/menu.h"
#include "../bitmaps.h"
#include "../board.h"
#include "../dcs.h"
#include "../driver/backlight.h"
#include "../driver/bk4819.h"
#include "../driver/eeprom.h"
#include "../driver/st7565.h"
#include "../external/printf/printf.h"
#include "../frequencies.h"
#include "../helper/battery.h"
#include "../misc.h"
#include "../settings.h"
#include "helper.h"
#include "inputbox.h"
#include "menu.h"
#include "gui/ui.h"
#include "gui/gui.h"
#include "ui.h"

const t_menu_item MenuList[] =
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
	{"Scan Add1", 	MENU_S_ADD1        },
	{"Scan Add2", 	MENU_S_ADD2        },
	{"Chan. Save", 	MENU_MEM_CH        }, // was "MEM-CH"
	{"Chan. Dele", 	MENU_DEL_CH        }, // was "DEL-CH"
	{"Chan. Name", 	MENU_MEM_NAME      },

	{"Scan List",  	MENU_S_LIST        },
	{"Scan List1", 	MENU_SLIST1        },
	{"Scan List2", 	MENU_SLIST2        },
	{"Scan Rev", 	MENU_SC_REV        },

	{"F1 Short", 	MENU_F1SHRT        },
	{"F1 Long", 	MENU_F1LONG        },
	{"F2 Short", 	MENU_F2SHRT        },
	{"F2 Long", 	MENU_F2LONG        },
	{"M Long", 		MENU_MLONG         },

	{"Key Lock", 	MENU_AUTOLK        }, // was "AUTOLk"
	{"Tx TimeOut", 	MENU_TOT           }, // was "TOT"
	{"Bat Save", 	MENU_SAVE          }, // was "SAVE"
	{"Mic",    		MENU_MIC           },
#ifdef ENABLE_AUDIO_BAR
	{"Mic Bar", 	MENU_MIC_BAR       },
#endif
	//{"ChDisp", MENU_MDF           }, // was "MDF"
	{"Pow. onMsg", 	MENU_PONMSG        },
	{"Bat Txt", 	MENU_BAT_TXT       },
	{"Back Lt", 	MENU_ABR           }, // was "ABR"
	{"BL Min",  	MENU_ABR_MIN       },
	{"BL Max",  	MENU_ABR_MAX       },
#ifdef ENABLE_C
	{"LCD Ctr", 	MENU_CONTRAST      },
#endif
	{"Blt TRX", 	MENU_ABR_ON_TX_RX  },
	{"Beep",   		MENU_BEEP          },
	{"Roger",  		MENU_ROGER         },
	{"STE",    		MENU_STE           },
	{"RP STE", 		MENU_RP_STE        },
	{"1 Call", 		MENU_1_CALL        },
#ifdef ENABLE_ALARM
	{"Alarm T", 	MENU_AL_MOD        },
#endif
#ifdef ENABLE_DTMF_CALLING
	{"ANI ID", 		MENU_ANI_ID        },
#endif
	{"UP Code", 	MENU_UPCODE        },
	{"DW Code", 	MENU_DWCODE        },
	{"PTT ID", 		MENU_PTT_ID        },
	{"D ST",   		MENU_D_ST          },
#ifdef ENABLE_DTMF_CALLING
    {"DTMF Resp", 	MENU_D_RSP         },
	{"DTMF Hold", 	MENU_D_HOLD        },
#endif
	{"DTMF Prel", 	MENU_D_PRE         },
#ifdef ENABLE_DTMF_CALLING
	{"DTMF Decod", 	MENU_D_DCD         },
	{"DTMF List", 	MENU_D_LIST        },
#endif
	{"DTMF Live", 	MENU_D_LIVE_DEC    }, // live DTMF decoder
#ifdef ENABLE_AM_FIX
	{"AM Fix", 		MENU_AM_FIX        },
#endif
#ifdef ENABLE_VOX
	{"VOX",    		MENU_VOX           },
#endif
	{"Bat Vol", 	MENU_VOL           }, // was "VOL"
	{"Rx Mode", 	MENU_TDR           },
	{"SQL",    		MENU_SQL           },

	// hidden menu items from here on
	// enabled if pressing both the PTT and upper side button at power-on
	{"F Lock", 		MENU_F_LOCK        },
	{"Tx 200", 		MENU_200TX         }, // was "200TX"
	{"Tx 350", 		MENU_350TX         }, // was "350TX"
	{"Tx 500", 		MENU_500TX         }, // was "500TX"
	{"350 En", 		MENU_350EN         }, // was "350EN"
	{"Scra En", 	MENU_SCREN         }, // was "SCREN"
#ifdef ENABLE_F_CAL_MENU
	{"Frq Cali", 	MENU_F_CALI        }, // reference xtal calibration
#endif
	{"Bat Cali", 	MENU_BATCAL        }, // battery voltage calibration
	{"Bat Type", 	MENU_BATTYP        }, // battery type 1600/2200mAh
	{"Reset",  		MENU_RESET         }, // might be better to move this to the hidden menu items ?

	{"",       0xff               }  // end of list - DO NOT delete or move this this
};


const uint8_t FIRST_HIDDEN_MENU_ITEM = MENU_F_LOCK;

const char gSubMenu_TXP[][5] =
{
	"LOW",
	"MID",
	"HIGH"
};

const char gSubMenu_W_N[][6] =
{
	"25.0K",
	"12.5K",
	"6.25K"
};

const char gSubMenu_TOT[][8] =
{
	"30 sec",
	"1 min",
	"2 min",
	"3 min",
	"4 min",
	"5 min",
	"6 min",
	"7 min",
	"8 min",
	"9 min",
	"15 min"
};

const char gSubMenu_BACKLIGHT[][7] =
{
	"OFF",
	"5 sec",
	"10 sec",
	"20 sec",
	"1 min",
	"2 min",
	"4 min",
	"ON"
};

const char gSubMenu_SFT_D[][4] =
{
	"OFF",
	"+",
	"-"
};

const char gSubMenu_OFF_ON[][4] =
{
	"OFF",
	"ON"
};

const char gSubMenu_SAVE[][4] =
{
	"OFF",
	"1:1",
	"1:2",
	"1:3",
	"1:4"
};

const char gSubMenu_RXMode[][11] =
{
	"MAIN RX TX", 		// TX and RX on main only
	"DUAL RX TX", 		// Watch both and respond
	"CROSS B.", 		// TX on main, RX on secondary
	"TX MAIN" 			// always TX on main, but RX on both
};

const char gSubMenu_SC_REV[][8] =
{
	"TIMEOUT",
	"CARRIER",
	"STOP"
};

/*const char gSubMenu_MDF[][16] =
{
	"FREQ",
	"CHANNEL\nNUMBER",
	"NAME",
	"NAME\n+\nFREQ"
};*/

#ifdef ENABLE_ALARM
	const char gSubMenu_AL_MOD[][5] =
	{
		"SITE",
		"TONE"
	};
#endif

#ifdef ENABLE_DTMF_CALLING
const char gSubMenu_D_RSP[][11] =
{
	"DO NOTHING",
	"RING",
	"REPLY",
	"BOTH"
};
#endif

const char gSubMenu_PTT_ID[][10] =
{
	"OFF",
	"UP CODE",
	"DOWN CODE",
	"U+D CODE",
	"APOLLO"
};

const char gSubMenu_PONMSG[][8] =
{
	"FULL",
	"MESSAGE",
	"VOLTAGE",
	"NONE"
};

const char gSubMenu_ROGER[][11] =
{
	"OFF",
	"DEFAULT",
	"MOTOTRBO",
    "MOTO TPT",
	"MOTO T40",
	"MOTO T80",
	"C.AM845",
	"POLIZIA"
	//"MDC"
};

const char gSubMenu_RESET[][4] =
{
	"VFO",
	"ALL"
};

const char gSubMenu_F_LOCK[][27] =
{
	"DEFAULT+\n137-174\n400-470",
	"FCC HAM\n144-148\n420-450",
	"CE HAM\n144-146\n430-440",
	"GB HAM\n144-148\n430-440",
	"137-174\n400-430",
	"137-174\n400-438",
	"DISABLE\nALL",
	"UNLOCK\nALL",
};

const char gSubMenu_RX_TX[][6] =
{
	"OFF",
	"TX",
	"RX",
	"TX/RX"
};

const char gSubMenu_BAT_TXT[][8] =
{
	"NONE",
	"VOLTAGE",
	"PERCENT"
};

const char gSubMenu_BATTYP[][9] =
{
	"1600mAh",
	"2200mAh"
};

const char gSubMenu_SCRAMBLER[][7] =
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

const t_sidefunction gSubMenu_SIDEFUNCTIONS[] =
{
	{"NONE",			ACTION_OPT_NONE},
#ifdef ENABLE_FLASHLIGHT
	{"FLASH\nLIGHT",	ACTION_OPT_FLASHLIGHT},
#endif
	{"POWER",			ACTION_OPT_POWER},
	{"MONITOR",			ACTION_OPT_MONITOR},
	{"SCAN",			ACTION_OPT_SCAN},
#ifdef ENABLE_VOX
	{"VOX",				ACTION_OPT_VOX},
#endif
#ifdef ENABLE_ALARM
	{"ALARM",			ACTION_OPT_ALARM},
#endif
#ifdef ENABLE_FMRADIO
	{"FM RADIO",		ACTION_OPT_FM},
#endif
#ifdef ENABLE_TX1750
	{"1750HZ",			ACTION_OPT_1750},
#endif
	{"LOCK\nKEYPAD",	ACTION_OPT_KEYLOCK},
	{"SWITCH\nVFO",		ACTION_OPT_A_B},
	{"VFO/MR",			ACTION_OPT_VFO_MR},
	{"SWITCH\nDEMODUL",	ACTION_OPT_SWITCH_DEMODUL},
#ifdef ENABLE_BLMIN_TMP_OFF
	{"BLMIN\nTMP OFF",  ACTION_OPT_BLMIN_TMP_OFF}, 		//BackLight Minimum Temporay OFF
#endif
};

const uint8_t gSubMenu_SIDEFUNCTIONS_size = ARRAY_SIZE(gSubMenu_SIDEFUNCTIONS);

//bool    gIsInSubMenu;
//uint8_t gMenuCursor;

uint8_t UI_MENU_GetMenuIdx(uint8_t id)
{
	for(uint8_t i = 0; i < ARRAY_SIZE(MenuList); i++)
		if(MenuList[i].menu_id == id)
			return i;
	return 0;
}

//int32_t gSubMenuSelection;

// edit box
char    edit_original[17]; // a copy of the text before editing so that we can easily test for changes/difference
char    edit[17];
int     edit_index;

#if 1

#define SUB_MENU_X 68
#define SETTINGS_TIMESHOW_SUB 500

uint8_t settingsCurrentMenu = 0;
uint8_t settingsCurrentSubMenu = 0;
uint16_t settingsSubmenuMin = 0;
uint16_t settingsSubmenuSize = 0;

uint16_t settingsSubMenuTime = 0;
bool settingsShowSubMenu = false;
bool settingsSubMenuActive = false;
bool needToSave = false;

int UI_MENU_GetCurrentMenuId() {
	if(settingsCurrentMenu < ARRAY_SIZE(MenuList))
		return MenuList[settingsCurrentMenu].menu_id;

	return MenuList[ARRAY_SIZE(MenuList)-1].menu_id;
}

void SettingsMenu_showList() {
    uint8_t yPos = 15;
    uint8_t menuSize = gMenuListCount - 2;
	const uint8_t offset = Clamp(settingsCurrentMenu - 2, 0, menuSize - 3);

    for (uint8_t i = 0; i < 5; i++) {
		bool isFill = settingsCurrentMenu == i + offset ? true : false;

        UI_printf(&font_small, TEXT_ALIGN_LEFT, 8, 0, yPos - 1, true, false, "%0.2i", (i + 1) + offset);

        UI_printf(&font_10, TEXT_ALIGN_LEFT, 18, 0, yPos, !isFill, isFill,
            "%s", MenuList[i + offset].name);
        yPos += 11;
    }

    yPos = 10;
    yPos += ((( (100 * settingsCurrentMenu) / (menuSize + 2) ) / 100.0) * 52);

    UI_drawFastVLine(1, yPos - 1, 3, true);
    UI_drawFastVLine(3, yPos - 1, 3, true);
}

/* ---------------------------------------------------------------------------------------------------- */
// Sub List

void SettingsMenu_showSubListCalc(uint8_t size, uint8_t *yPos, uint8_t *listCount, uint8_t *offset) {
    if (size <= 2 ) {
        if (size == 2 ) {
            *yPos = *yPos + 6;
        } else if (size == 1 ) {
            *yPos = *yPos + 11;
        }
        *listCount = size + 1;
    } else {
		*offset = Clamp(settingsCurrentSubMenu - 2, 0, size - 3);
    }
}

void SettingsMenu_showSubList(uint8_t size, uint8_t length, const char subList[size][length]) {
	uint8_t yPos = 21;
    uint8_t listCount = 4;
	uint8_t offset = 0;
	SettingsMenu_showSubListCalc(size - 1, &yPos, &listCount, &offset);
    for (uint8_t i = 0; i < listCount; i++) {
		bool isFill = settingsCurrentSubMenu == i + offset ? true : false;
		UI_printf(&font_10, TEXT_ALIGN_CENTER, SUB_MENU_X, 125, yPos, !isFill, isFill,
			"%s", subList[i + offset]);
        yPos += 11;
    }
}

void SettingsMenu_showSubValue(void) {
	const uint8_t yPos = 21;
	switch (UI_MENU_GetCurrentMenuId())
	{
		case MENU_OFFSET:
			if (!settingsSubMenuActive || gInputBoxIndex == 0) {
				UI_printf(&font_10, TEXT_ALIGN_CENTER, SUB_MENU_X, 125, yPos + 6, true, false, "%3d.%05u", (uint16_t)settingsCurrentSubMenu / 100000, (uint16_t)abs(settingsCurrentSubMenu) % 100000);
			} else {
				const char * ascii = INPUTBOX_GetAscii();
				UI_printf(&font_10, TEXT_ALIGN_CENTER, SUB_MENU_X, 125, yPos + 6, true, false, "%.3s.%.3s",ascii, ascii + 3);
			}

			UI_printf(&font_10, TEXT_ALIGN_CENTER, SUB_MENU_X, 125, yPos + 27, true, false, "Mhz");

			break;
		default:
		break;
	}
}

void SettingsMenu_showSubListValue(void) {
	uint8_t yPos = 21;
    uint8_t listCount = 4;
	uint8_t offset = 0;
	SettingsMenu_showSubListCalc(settingsSubmenuSize, &yPos, &listCount, &offset);
    for (uint8_t i = 0; i < listCount; i++) {
		bool isFill = settingsCurrentSubMenu == i + offset ? true : false;

		if ( (i + offset == 0) && (UI_MENU_GetCurrentMenuId() != MENU_STEP) ) {
			UI_printf(&font_10, TEXT_ALIGN_CENTER, SUB_MENU_X, 125, yPos, !isFill, isFill, "OFF" );
		} else {

			switch (UI_MENU_GetCurrentMenuId())
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
	SettingsMenu_showSubListCalc(settingsSubmenuSize, &yPos, &listCount, &offset);
    for (uint8_t i = 0; i < listCount; i++) {
		UI_printf(&font_10, TEXT_ALIGN_CENTER, SUB_MENU_X, 125, yPos,
			settingsCurrentSubMenu == i + offset ? false : true,
			settingsCurrentSubMenu == i + offset ? true : false,
			"%i", (i + offset));
        yPos += 11;
    }
}

void SettingsMenu_subList() {

	UI_fillRect(SUB_MENU_X - 2, 11, 61, 48, false);
    if ( settingsSubMenuActive ) {
    	UI_drawRoundRect(SUB_MENU_X - 2, 11, 61, 48, 4, true);
	}
    UI_drawRoundRect(SUB_MENU_X - 1, 12, 59, 46, 4, true);

	settingsSubmenuMin = 0;
	uint8_t arrayLength = 0;
	bool showNumber = false;
	bool showValue = false;

	switch (UI_MENU_GetCurrentMenuId())
    {

		case MENU_ABR:
			//gSubMenu_BACKLIGHT
			arrayLength = 7;
            SettingsMenu_showSubList(settingsSubmenuSize, arrayLength, gSubMenu_BACKLIGHT);
			break;

		case MENU_SCR:
			//gSubMenu_SCRAMBLER
			arrayLength = 7;
            SettingsMenu_showSubList(settingsSubmenuSize, arrayLength, gSubMenu_SCRAMBLER);
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
			arrayLength = 27;
            SettingsMenu_showSubList(settingsSubmenuSize, arrayLength, gSubMenu_F_LOCK);
			break;

		case MENU_PTT_ID:
			arrayLength = 10;
            SettingsMenu_showSubList(settingsSubmenuSize, arrayLength, gSubMenu_PTT_ID);
			break;

		case MENU_SFT_D:
			//gSubMenu_SFT_D
			arrayLength = 4;
            SettingsMenu_showSubList(settingsSubmenuSize, arrayLength, gSubMenu_SFT_D);
			break;

		case MENU_TDR:
			//gSubMenu_RXMode
			arrayLength = 11;
            SettingsMenu_showSubList(settingsSubmenuSize, arrayLength, gSubMenu_RXMode);
			break;

		case MENU_ROGER:
			//gSubMenu_ROGER
			arrayLength = 11;
            SettingsMenu_showSubList(settingsSubmenuSize, arrayLength, gSubMenu_ROGER);
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
			arrayLength = 4;
            SettingsMenu_showSubList(settingsSubmenuSize, arrayLength, gSubMenu_RESET);
			break;

		case MENU_COMPAND:
		case MENU_ABR_ON_TX_RX:
			//gSubMenu_RX_TX
			arrayLength = 6;
            SettingsMenu_showSubList(settingsSubmenuSize, arrayLength, gSubMenu_RX_TX);
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
			arrayLength = 4;
        	SettingsMenu_showSubList(settingsSubmenuSize, arrayLength, gSubMenu_OFF_ON);
			break;

		case MENU_TOT:
			//gSubMenu_TOT
			arrayLength = 8;
            SettingsMenu_showSubList(settingsSubmenuSize, arrayLength, gSubMenu_TOT);
			break;

		case MENU_SAVE:
			//gSubMenu_SAVE
			arrayLength = 4;
            SettingsMenu_showSubList(settingsSubmenuSize, arrayLength, gSubMenu_SAVE);
			break;

		case MENU_BAT_TXT:
			//gSubMenu_BAT_TXT
			arrayLength = 8;
            SettingsMenu_showSubList(settingsSubmenuSize, arrayLength, gSubMenu_BAT_TXT);
			break;

		case MENU_PONMSG:
			arrayLength = 8;
            SettingsMenu_showSubList(settingsSubmenuSize, arrayLength, gSubMenu_PONMSG);
			break;

		case MENU_SC_REV:
			arrayLength = 8;
            SettingsMenu_showSubList(settingsSubmenuSize, arrayLength, gSubMenu_SC_REV);
			break;


		case MENU_OFFSET:
			//TODO:
			SettingsMenu_showSubValue();
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
			arrayLength = 9;
            SettingsMenu_showSubList(settingsSubmenuSize, arrayLength, gSubMenu_BATTYP);
			break;


        /*default:
            break;*/
    }
	if (showValue) {
		SettingsMenu_showSubListValue();
	} else if (showNumber) {
		SettingsMenu_showSubListNumber();
	}
}

void UI_DisplayMenu(void) {
	UI_displayClear();
    UI_drawFastVLine(2, 9, 53, true);

    SettingsMenu_showList();

    if (settingsShowSubMenu) {
        SettingsMenu_subList();
    } else {
        if (xTaskGetTickCount() - settingsSubMenuTime > pdMS_TO_TICKS(SETTINGS_TIMESHOW_SUB)) {
			/*if( GUI_inputGetSize() == 1 ) {
				const uint8_t inputValue = GUI_inputGetNumberClear();
				if ( inputValue > 0) {
					settingsCurrentMenu = inputValue - 1;
					settingsShowSubMenu = false;
					settingsSubMenuTime =xTaskGetTickCount();
				}
			} else {*/
				gMenuCountdown = menu_timeout_long_500ms;
            	settingsShowSubMenu = true;
            	settingsCurrentSubMenu = 0;
				MENU_ShowCurrentSetting();
			//}
        }
    }

	UI_displayUpdate();
	//gUpdateDisplay = true;
}
#else

void UI_DisplayMenu(void)
{
	const unsigned int menu_list_width = 6; // max no. of characters on the menu list (left side)
	const unsigned int menu_item_x1    = (8 * menu_list_width) + 2;
	const unsigned int menu_item_x2    = LCD_WIDTH - 1;
	unsigned int       i;
	char               String[64];  // bigger cuz we can now do multi-line in one string (use '\n' char)

#ifdef ENABLE_DTMF_CALLING
	char               Contact[16];
#endif

	UI_displayClear();

#ifndef ENABLE_CUSTOM_MENU_LAYOUT
		// original menu layout
	for (i = 0; i < 3; i++)
		if (gMenuCursor > 0 || i > 0)
			if ((gMenuListCount - 1) != gMenuCursor || i != 2)
				UI_PrintString(MenuList[gMenuCursor + i - 1].name, 0, 0, i * 2, 8);

	// invert the current menu list item pixels
	for (i = 0; i < (8 * menu_list_width); i++)
	{
		gFrameBuffer[2][i] ^= 0xFF;
		gFrameBuffer[3][i] ^= 0xFF;
	}

	// draw vertical separating dotted line
	for (i = 0; i < 7; i++)
		gFrameBuffer[i][(8 * menu_list_width) + 1] = 0xAA;

	// draw the little sub-menu triangle marker
	/*if (gIsInSubMenu)
		memcpy(gFrameBuffer[0] + (8 * menu_list_width) + 1, BITMAP_CurrentIndicator, sizeof(BITMAP_CurrentIndicator));*/

	// draw the menu index number/count
	sprintf(String, "%2u.%u", 1 + gMenuCursor, gMenuListCount);

	UI_PrintStringSmallNormal(String, 2, 0, 6);

#else
	{	// new menu layout .. experimental & unfinished
		const int menu_index = gMenuCursor;  // current selected menu item
		i = 1;

		if (!gIsInSubMenu) {
			while (i < 2)
			{	// leading menu items - small text
				const int k = menu_index + i - 2;
				if (k < 0)
					UI_PrintStringSmallNormal(MenuList[gMenuListCount + k].name, 0, 0, i);  // wrap-a-round
				else if (k >= 0 && k < (int)gMenuListCount)
					UI_PrintStringSmallNormal(MenuList[k].name, 0, 0, i);
				i++;
			}

			// current menu item - keep big n fat
			if (menu_index >= 0 && menu_index < (int)gMenuListCount)
				UI_PrintString(MenuList[menu_index].name, 0, 0, 2, 8);
			i++;

			while (i < 4)
			{	// trailing menu item - small text
				const int k = menu_index + i - 2;
				if (k >= 0 && k < (int)gMenuListCount)
					UI_PrintStringSmallNormal(MenuList[k].name, 0, 0, 1 + i);
				else if (k >= (int)gMenuListCount)
					UI_PrintStringSmallNormal(MenuList[gMenuListCount - k].name, 0, 0, 1 + i);  // wrap-a-round
				i++;
			}

			// draw the menu index number/count
			sprintf(String, "%2u.%u", 1 + gMenuCursor, gMenuListCount);
			UI_PrintStringSmallNormal(String, 2, 0, 6);
		}
		else if (menu_index >= 0 && menu_index < (int)gMenuListCount)
		{	// current menu item
//			strcat(String, ":");
			UI_PrintString(MenuList[menu_index].name, 0, 0, 0, 8);
//			UI_PrintStringSmallNormal(String, 0, 0, 0);
		}
	}
#endif

	// **************

	memset(String, 0, sizeof(String));

	bool already_printed = false;

	/* Brightness is set to max in some entries of this menu. Return it to the configured brightness
	   level the "next" time we enter here.I.e., when we move from one menu to another.
	   It also has to be set back to max when pressing the Exit key. */

	BACKLIGHT_TurnOn();

	switch (UI_MENU_GetCurrentMenuId())
	{
		case MENU_SQL:
			sprintf(String, "%d", (uint16_t)gSubMenuSelection);
			break;

		case MENU_MIC:
			{	// display the mic gain in actual dB rather than just an index number
				const uint8_t mic = gMicGain_dB2[gSubMenuSelection];
				sprintf(String, "+%u.%01udB", mic / 2, mic % 2);
			}
			break;

		#ifdef ENABLE_AUDIO_BAR
			case MENU_MIC_BAR:
				strcpy(String, gSubMenu_OFF_ON[gSubMenuSelection]);
				break;
		#endif

		case MENU_STEP: {
			uint16_t step = gStepFrequencyTable[FREQUENCY_GetStepIdxFromSortedIdx(gSubMenuSelection)];
			sprintf(String, "%d.%02ukHz", step / 100, step % 100);
			break;
		}

		case MENU_TXP:
			strcpy(String, gSubMenu_TXP[gSubMenuSelection]);
			break;

		case MENU_R_DCS:
		case MENU_T_DCS:
			if (gSubMenuSelection == 0)
				strcpy(String, "OFF");
			else if (gSubMenuSelection < 105)
				sprintf(String, "D%03oN", DCS_Options[gSubMenuSelection -   1]);
			else
				sprintf(String, "D%03oI", DCS_Options[gSubMenuSelection - 105]);
			break;

		case MENU_R_CTCS:
		case MENU_T_CTCS:
		{
			if (gSubMenuSelection == 0)
				strcpy(String, "OFF");
			else
				sprintf(String, "%u.%uHz", CTCSS_Options[gSubMenuSelection - 1] / 10, CTCSS_Options[gSubMenuSelection - 1] % 10);
			break;
		}

		case MENU_SFT_D:
			strcpy(String, gSubMenu_SFT_D[gSubMenuSelection]);
			break;

		case MENU_OFFSET:
			if (!gIsInSubMenu || gInputBoxIndex == 0)
			{
				sprintf(String, "%3d.%05u", (uint16_t)gSubMenuSelection / 100000, (uint16_t)abs(gSubMenuSelection) % 100000);
				UI_PrintString(String, menu_item_x1, menu_item_x2, 1, 8);
			}
			else
			{
				const char * ascii = INPUTBOX_GetAscii();
				sprintf(String, "%.3s.%.3s  ",ascii, ascii + 3);
				UI_PrintString(String, menu_item_x1, menu_item_x2, 1, 8);
			}

			UI_PrintString("MHz",  menu_item_x1, menu_item_x2, 3, 8);

			already_printed = true;
			break;

		case MENU_W_N:
			strcpy(String, gSubMenu_W_N[gSubMenuSelection]);
			break;

		case MENU_SCR:
			strcpy(String, gSubMenu_SCRAMBLER[gSubMenuSelection]);
			#if 1
				if (gSubMenuSelection > 0 && gSetting_ScrambleEnable)
					BK4819_EnableScramble(gSubMenuSelection - 1);
				else
					BK4819_DisableScramble();
			#endif
			break;

		#ifdef ENABLE_VOX
			case MENU_VOX:
				if (gSubMenuSelection == 0)
					strcpy(String, "OFF");
				else
					sprintf(String, "%d", gSubMenuSelection);
				break;
		#endif

		case MENU_ABR:
			strcpy(String, gSubMenu_BACKLIGHT[gSubMenuSelection]);
			if(BACKLIGHT_GetBrightness() < 4)
				BACKLIGHT_SetBrightness(4);
			break;

		case MENU_ABR_MIN:
		case MENU_ABR_MAX:
			sprintf(String, "%d", (uint16_t)gSubMenuSelection);
			if(gIsInSubMenu)
				BACKLIGHT_SetBrightness(gSubMenuSelection);
			else if(BACKLIGHT_GetBrightness() < 4)
				BACKLIGHT_SetBrightness(4);
			break;

		case MENU_CONTRAST:
			sprintf(String, "%d", (uint16_t)gSubMenuSelection);
			ST7565_SetContrast(gSubMenuSelection);
			//else
			//	ST7565_SetContrast(0);
			//g_update_display = true;
			break;

		case MENU_AM:
			strcpy(String, gModulationStr[gSubMenuSelection]);
			break;

		case MENU_AUTOLK:
			strcpy(String, (gSubMenuSelection == 0) ? "OFF" : "AUTO");
			break;

		case MENU_COMPAND:
		case MENU_ABR_ON_TX_RX:
			strcpy(String, gSubMenu_RX_TX[gSubMenuSelection]);
			break;

		#ifdef ENABLE_AM_FIX
			case MENU_AM_FIX:
		#endif
		case MENU_BCL:
		case MENU_BEEP:
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
			strcpy(String, gSubMenu_OFF_ON[gSubMenuSelection]);
			break;

		case MENU_MEM_CH:
		case MENU_1_CALL:
		case MENU_DEL_CH:
		{
			const bool valid = RADIO_CheckValidChannel(gSubMenuSelection, false, 0);

			UI_GenerateChannelStringEx(String, valid, gSubMenuSelection);
			UI_PrintString(String, menu_item_x1, menu_item_x2, 0, 8);

			if (valid && !gAskForConfirmation)
			{	// show the frequency so that the user knows the channels frequency
				const uint32_t frequency = SETTINGS_FetchChannelFrequency(gSubMenuSelection);
				sprintf(String, "%u.%05u", (uint16_t)frequency / 100000, (uint16_t)frequency % 100000);
				UI_PrintString(String, menu_item_x1, menu_item_x2, 4, 8);
			}

			SETTINGS_FetchChannelName(String, gSubMenuSelection);
			UI_PrintString(String[0] ? String : "--", menu_item_x1, menu_item_x2, 2, 8);
			already_printed = true;
			break;
		}

		case MENU_MEM_NAME:
		{
			const bool valid = RADIO_CheckValidChannel(gSubMenuSelection, false, 0);

			UI_GenerateChannelStringEx(String, valid, gSubMenuSelection);
			UI_PrintString(String, menu_item_x1, menu_item_x2, 0, 8);

			if (valid)
			{
				const uint32_t frequency = SETTINGS_FetchChannelFrequency(gSubMenuSelection);

				if (!gIsInSubMenu || edit_index < 0)
				{	// show the channel name
					SETTINGS_FetchChannelName(String, gSubMenuSelection);
					char *pPrintStr = String[0] ? String : "--";
					UI_PrintString(pPrintStr, menu_item_x1, menu_item_x2, 2, 8);
				}
				else
				{	// show the channel name being edited
					UI_PrintString(edit, menu_item_x1, 0, 2, 8);
					if (edit_index < 10)
						UI_PrintString("^", menu_item_x1 + (8 * edit_index), 0, 4, 8);  // show the cursor
				}

				if (!gAskForConfirmation)
				{	// show the frequency so that the user knows the channels frequency
					sprintf(String, "%u.%05u", (uint16_t)frequency / 100000, (uint16_t)frequency % 100000);
					UI_PrintString(String, menu_item_x1, menu_item_x2, 4 + (gIsInSubMenu && edit_index >= 0), 8);
				}
			}

			already_printed = true;
			break;
		}

		case MENU_SAVE:
			strcpy(String, gSubMenu_SAVE[gSubMenuSelection]);
			break;

		case MENU_TDR:
			strcpy(String, gSubMenu_RXMode[gSubMenuSelection]);
			break;

		case MENU_TOT:
			strcpy(String, gSubMenu_TOT[gSubMenuSelection]);
			break;

		case MENU_SC_REV:
			strcpy(String, gSubMenu_SC_REV[gSubMenuSelection]);
			break;

		case MENU_MDF:
			strcpy(String, gSubMenu_MDF[gSubMenuSelection]);
			break;

		case MENU_RP_STE:
			if (gSubMenuSelection == 0)
				strcpy(String, "OFF");
			else
				sprintf(String, "%d*100ms", (uint16_t)gSubMenuSelection);
			break;

		case MENU_S_LIST:
			if (gSubMenuSelection < 2)
				sprintf(String, "LIST%u", (uint16_t)(1 + gSubMenuSelection));
			else
				strcpy(String, "ALL");
			break;

		#ifdef ENABLE_ALARM
			case MENU_AL_MOD:
				sprintf(String, gSubMenu_AL_MOD[gSubMenuSelection]);
				break;
		#endif

#ifdef ENABLE_DTMF_CALLING
		case MENU_ANI_ID:
			strcpy(String, gEeprom.ANI_DTMF_ID);
			break;
#endif
		case MENU_UPCODE:
			sprintf(String, "%.8s\n%.8s", gEeprom.DTMF_UP_CODE, gEeprom.DTMF_UP_CODE + 8);
			break;

		case MENU_DWCODE:
			sprintf(String, "%.8s\n%.8s", gEeprom.DTMF_DOWN_CODE, gEeprom.DTMF_DOWN_CODE + 8);
			break;

#ifdef ENABLE_DTMF_CALLING
		case MENU_D_RSP:
			strcpy(String, gSubMenu_D_RSP[gSubMenuSelection]);
			break;

		case MENU_D_HOLD:
			sprintf(String, "%ds", gSubMenuSelection);
			break;
#endif
		case MENU_D_PRE:
			sprintf(String, "%d*10ms", (uint16_t)gSubMenuSelection);
			break;

		case MENU_PTT_ID:
			strcpy(String, gSubMenu_PTT_ID[gSubMenuSelection]);
			break;

		case MENU_BAT_TXT:
			strcpy(String, gSubMenu_BAT_TXT[gSubMenuSelection]);
			break;

#ifdef ENABLE_DTMF_CALLING
		case MENU_D_LIST:
			gIsDtmfContactValid = DTMF_GetContact((int)gSubMenuSelection - 1, Contact);
			if (!gIsDtmfContactValid)
				strcpy(String, "NULL");
			else
				memcpy(String, Contact, 8);
			break;
#endif

		case MENU_PONMSG:
			strcpy(String, gSubMenu_PONMSG[gSubMenuSelection]);
			break;

		case MENU_ROGER:
			strcpy(String, gSubMenu_ROGER[gSubMenuSelection]);
			break;

		case MENU_VOL:
			sprintf(String, "%u.%02uV\n%u%%",
				gBatteryVoltageAverage / 100, gBatteryVoltageAverage % 100,
				BATTERY_VoltsToPercent(gBatteryVoltageAverage));
			break;

		case MENU_RESET:
			strcpy(String, gSubMenu_RESET[gSubMenuSelection]);
			break;

		case MENU_F_LOCK:
			if(!gIsInSubMenu && gUnlockAllTxConfCnt>0 && gUnlockAllTxConfCnt<10)
				strcpy(String, "READ\nMANUAL");
			else
				strcpy(String, gSubMenu_F_LOCK[gSubMenuSelection]);
			break;

		#ifdef ENABLE_F_CAL_MENU
			case MENU_F_CALI:
				{
					const uint32_t value   = 22656 + gSubMenuSelection;
					const uint32_t xtal_Hz = (0x4f0000u + value) * 5;

					writeXtalFreqCal(gSubMenuSelection, false);

					sprintf(String, "%d\n%u.%06u\nMHz",
						(uint16_t)gSubMenuSelection,
						(uint16_t)xtal_Hz / 1000000, (uint16_t)xtal_Hz % 1000000);
				}
				break;
		#endif

		case MENU_BATCAL:
		{
			const uint16_t vol = (uint32_t)gBatteryVoltageAverage * gBatteryCalibration[3] / gSubMenuSelection;
			sprintf(String, "%u.%02uV\n%u", (uint16_t)vol / 100, (uint16_t)vol % 100, (uint16_t)gSubMenuSelection);
			break;
		}

		case MENU_BATTYP:
			strcpy(String, gSubMenu_BATTYP[gSubMenuSelection]);
			break;

		case MENU_F1SHRT:
		case MENU_F1LONG:
		case MENU_F2SHRT:
		case MENU_F2LONG:
		case MENU_MLONG:
			strcpy(String, gSubMenu_SIDEFUNCTIONS[gSubMenuSelection].name);
			break;
	}

	if (!already_printed)
	{	// we now do multi-line text in a single string

		unsigned int y;
		unsigned int lines = 1;
		unsigned int len   = strlen(String);
		bool         small = false;

		if (len > 0)
		{
			// count number of lines
			for (i = 0; i < len; i++)
			{
				if (String[i] == '\n' && i < (len - 1))
				{	// found new line char
					lines++;
					String[i] = 0;  // null terminate the line
				}
			}

			if (lines > 3)
			{	// use small text
				small = true;
				if (lines > 7)
					lines = 7;
			}

			// center vertically'ish
			if (small)
				y = 3 - ((lines + 0) / 2);  // untested
			else
				y = 2 - ((lines + 0) / 2);

			// draw the text lines
			for (i = 0; i < len && lines > 0; lines--)
			{
				if (small)
					UI_PrintStringSmallNormal(String + i, menu_item_x1, menu_item_x2, y);
				else
					UI_PrintString(String + i, menu_item_x1, menu_item_x2, y, 8);

				// look for start of next line
				while (i < len && String[i] >= 32)
					i++;

				// hop over the null term char(s)
				while (i < len && String[i] < 32)
					i++;

				y += small ? 1 : 2;
			}
		}
	}

	if (UI_MENU_GetCurrentMenuId() == MENU_SLIST1 || UI_MENU_GetCurrentMenuId() == MENU_SLIST2)
	{
		i = (UI_MENU_GetCurrentMenuId() == MENU_SLIST1) ? 0 : 1;
		char *pPrintStr = String;

		if (gSubMenuSelection < 0) {
			pPrintStr = "NULL";
		} else {
			UI_GenerateChannelStringEx(String, true, gSubMenuSelection);
			pPrintStr = String;
		}

		// channel number
		UI_PrintString(pPrintStr, menu_item_x1, menu_item_x2, 0, 8);

		SETTINGS_FetchChannelName(String, gSubMenuSelection);
		pPrintStr = String[0] ? String : "--";

		// channel name and scan-list
		if (gSubMenuSelection < 0 || !gEeprom.SCAN_LIST_ENABLED[i]) {
			UI_PrintString(pPrintStr, menu_item_x1, menu_item_x2, 2, 8);
		} else {
			UI_PrintStringSmallNormal(pPrintStr, menu_item_x1, menu_item_x2, 2);

			if (IS_MR_CHANNEL(gEeprom.SCANLIST_PRIORITY_CH1[i])) {
				sprintf(String, "PRI%d:%u", 1, gEeprom.SCANLIST_PRIORITY_CH1[i] + 1);
				UI_PrintString(String, menu_item_x1, menu_item_x2, 3, 8);
			}

			if (IS_MR_CHANNEL(gEeprom.SCANLIST_PRIORITY_CH2[i])) {
				sprintf(String, "PRI%d:%u", 2, gEeprom.SCANLIST_PRIORITY_CH2[i] + 1);
				UI_PrintString(String, menu_item_x1, menu_item_x2, 5, 8);
			}
		}
	}

	if ((UI_MENU_GetCurrentMenuId() == MENU_R_CTCS || UI_MENU_GetCurrentMenuId() == MENU_R_DCS) && gCssBackgroundScan)
		UI_PrintString("SCAN", menu_item_x1, menu_item_x2, 4, 8);

#ifdef ENABLE_DTMF_CALLING
	if (UI_MENU_GetCurrentMenuId() == MENU_D_LIST && gIsDtmfContactValid) {
		Contact[11] = 0;
		memcpy(&gDTMF_ID, Contact + 8, 4);
		sprintf(String, "ID:%4s", gDTMF_ID);
		UI_PrintString(String, menu_item_x1, menu_item_x2, 4, 8);
	}
#endif

	if (UI_MENU_GetCurrentMenuId() == MENU_R_CTCS ||
	    UI_MENU_GetCurrentMenuId() == MENU_T_CTCS ||
	    UI_MENU_GetCurrentMenuId() == MENU_R_DCS  ||
	    UI_MENU_GetCurrentMenuId() == MENU_T_DCS
#ifdef ENABLE_DTMF_CALLING
	    || UI_MENU_GetCurrentMenuId() == MENU_D_LIST
#endif
	) {
		sprintf(String, "%2d", (uint16_t)gSubMenuSelection);
		UI_PrintStringSmallNormal(String, 105, 0, 0);
	}

	if ((UI_MENU_GetCurrentMenuId() == MENU_RESET    ||
	     UI_MENU_GetCurrentMenuId() == MENU_MEM_CH   ||
	     UI_MENU_GetCurrentMenuId() == MENU_MEM_NAME ||
	     UI_MENU_GetCurrentMenuId() == MENU_DEL_CH) && gAskForConfirmation)
	{	// display confirmation
		char *pPrintStr = (gAskForConfirmation == 1) ? "SURE?" : "WAIT!";
		UI_PrintString(pPrintStr, menu_item_x1, menu_item_x2, 5, 8);
	}

	UI_displayUpdate();
}
#endif

