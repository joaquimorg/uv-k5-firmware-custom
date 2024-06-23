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

enum VFO_Settings_Option_e {
	MENU_SQL = 0,
	MENU_STEP,
	MENU_R_DCS,
	MENU_R_CTCS,
	MENU_T_DCS,
	MENU_T_CTCS,
	MENU_SFT_D,
	MENU_OFFSET,
	MENU_SCR,
	MENU_BCL,
	/*MENU_MEM_CH,
	MENU_DEL_CH,
	MENU_MEM_NAME,
	MENU_SAVE,*/
	MENU_COMPAND,
	MENU_PTT_ID,
};

typedef enum VFO_Settings_Option_e VFO_Settings_Option_t;

typedef struct {
	const char      name[11];
	VFO_Settings_Option_t   menu_id;
} vfo_settings_item_t;

const vfo_settings_item_t MenuList[] =
{
//   text,     		menu ID
	{"SQL",    		MENU_SQL           },
	{"Step",   		MENU_STEP          },
	{"Rx DCS",  	MENU_R_DCS         }, // was "R_DCS"
	{"Rx CTCS", 	MENU_R_CTCS        }, // was "R_CTCS"
	{"Tx DCS",  	MENU_T_DCS         }, // was "T_DCS"
	{"Tx CTCS", 	MENU_T_CTCS        }, // was "T_CTCS"
	{"Tx ODir", 	MENU_SFT_D         }, // was "SFT_D"
	{"Tx Offset", 	MENU_OFFSET        }, // was "OFFSET"
	{"Scrambler", 	MENU_SCR           }, // was "SCR"
	{"Busy CL", 	MENU_BCL           }, // was "BCL"
	{"Compand", 	MENU_COMPAND       },
	{"PTT ID", 		MENU_PTT_ID        },
};

#define MENU_VFO_SIZE 12

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

uint32_t settingsCurrentSubMenu = 0;
uint16_t settingsSubmenuMin = 0;
uint32_t settingsSubmenuSize = 0;

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
	const uint8_t offset = (uint8_t)Clamp((int)(settingsCurrentMenu - 2), 0, MENU_VFO_SIZE - 5);

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
	yPos += (uint8_t)((settingsCurrentMenu * 54.0) / MENU_VFO_SIZE);

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
        *listCount = (uint8_t)(settingsSubmenuSize + 1);
    } else {
		*offset = (uint8_t)Clamp((int)(settingsCurrentSubMenu - 2), 0, (int)(settingsSubmenuSize - 3));
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

void SettingsMenu_showInputValue(void) {
	uint8_t yPos = 30;

	if ( settingsCurrentSubMenu >= _1GHz_in_KHz ) {
		UI_printf(&font_10, TEXT_ALIGN_CENTER, SUB_MENU_X, 125, yPos, true, false, "%1u.%03u.%03u.%02u", (settingsCurrentSubMenu / 100000000), (settingsCurrentSubMenu / 100000) % 1000, (settingsCurrentSubMenu % 100000) / 100, (settingsCurrentSubMenu % 100));
	} else {
		UI_printf(&font_10, TEXT_ALIGN_CENTER, SUB_MENU_X, 125, yPos, true, false, "%03u.%03u.%02u", (settingsCurrentSubMenu / 100000) % 1000, (settingsCurrentSubMenu % 100000) / 100, (settingsCurrentSubMenu % 100));
	}

	UI_printf(&font_10, TEXT_ALIGN_CENTER, SUB_MENU_X, 125, yPos + 16, true, false, "MHz");
}

int SettingsMenu_GetLimits(uint8_t menu_id, uint16_t *pMin, uint32_t *pMax);

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

		case MENU_SCR:
            SettingsMenu_showSubList(gSubMenu_SCRAMBLER);
			break;

		case MENU_SQL:
			showNumber = true;
			break;

		case MENU_PTT_ID:
            SettingsMenu_showSubList(gSubMenu_PTT_ID);
			break;

		case MENU_SFT_D:
            SettingsMenu_showSubList(gSubMenu_SFT_D);
			break;

		case MENU_STEP:
		case MENU_R_DCS:
		case MENU_T_DCS:
		case MENU_R_CTCS:
		case MENU_T_CTCS:
			showValue = true;
			break;

		case MENU_COMPAND:
            SettingsMenu_showSubList(gSubMenu_RX_TX);
			break;

		case MENU_BCL:
        	SettingsMenu_showSubList(gSubMenu_OFF_ON);
			break;

		/*case MENU_SAVE:
            SettingsMenu_showSubList(gSubMenu_SAVE);
			break;*/


		case MENU_OFFSET:
			SettingsMenu_showInputValue();
			break;

		/*case MENU_MEM_CH:
		case MENU_DEL_CH:
			//TODO:
			break;

		case MENU_MEM_NAME:
			//TODO:
			break;*/

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
			settingsCurrentSubMenu = gSettings.squelch;
			break;

		case MENU_STEP:
			settingsCurrentSubMenu = FREQUENCY_GetSortedIdxFromStepIdx(gTxVfo->STEP_SETTING);
			break;

		case MENU_R_DCS:
		case MENU_R_CTCS:
		{
			DCS_CodeType_t type = gTxVfo->freq_config_RX.CodeType;
			uint8_t code = gTxVfo->freq_config_RX.Code;

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

		case MENU_SCR:
			settingsCurrentSubMenu = gTxVfo->SCRAMBLING_TYPE;
			break;

		case MENU_BCL:
			settingsCurrentSubMenu = gTxVfo->BUSY_CHANNEL_LOCK;
			break;

		case MENU_COMPAND:
			settingsCurrentSubMenu = gTxVfo->Compander;
			return;

		case MENU_PTT_ID:
			settingsCurrentSubMenu = gTxVfo->DTMF_PTT_ID_TX_MODE;
			break;

		default:
			return;
	}
}


int SettingsMenu_GetLimits(uint8_t menu_id, uint16_t *pMin, uint32_t *pMax)
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

		case MENU_SFT_D:
			*pMax = ARRAY_SIZE(gSubMenu_SFT_D) - 1;
			break;

		case MENU_R_DCS:
		case MENU_T_DCS:
			*pMax = 208;
			break;

		case MENU_R_CTCS:
		case MENU_T_CTCS:
			*pMax = ARRAY_SIZE(CTCSS_Options) - 1;
			break;

		case MENU_COMPAND:
			*pMax = ARRAY_SIZE(gSubMenu_RX_TX) - 1;
			break;

		case MENU_BCL:
			*pMax = ARRAY_SIZE(gSubMenu_OFF_ON) - 1;
			break;

		case MENU_SCR:
			*pMax = ARRAY_SIZE(gSubMenu_SCRAMBLER) - 1;
			break;

		/*case MENU_MEM_CH:
		case MENU_DEL_CH:
		case MENU_MEM_NAME:
			*pMax = MR_CHANNEL_LAST;
			break;*/

		/*case MENU_SAVE:
			*pMax = ARRAY_SIZE(gSubMenu_SAVE) - 1;
			break;*/

		case MENU_PTT_ID:
			*pMax = ARRAY_SIZE(gSubMenu_PTT_ID) - 1;
			break;

		case MENU_OFFSET:
			*pMax = 1000000;
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
			gSettings.squelch = (uint16_t)(settingsCurrentSubMenu & 0x0F);
			main_push_message(RADIO_VFO_CONFIGURE);
			break;

		case MENU_STEP:
			gTxVfo->STEP_SETTING = FREQUENCY_GetStepIdxFromSortedIdx((uint8_t)settingsCurrentSubMenu);
			if (IS_FREQ_CHANNEL(gTxVfo->CHANNEL_SAVE)) {
				main_push_message(RADIO_SAVE_CHANNEL);
				return;
			}
			return;

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
				pConfig->Code = (uint8_t)(settingsCurrentSubMenu - 1);
			}
			else {
				pConfig->CodeType = CODE_TYPE_REVERSE_DIGITAL;
				pConfig->Code = (uint8_t)(settingsCurrentSubMenu - 105);
			}

			//gRequestSaveChannel = 1;
			main_push_message(RADIO_SAVE_CHANNEL);
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
				pConfig->Code     = (uint8_t)(settingsCurrentSubMenu - 1);
				pConfig->CodeType = CODE_TYPE_CONTINUOUS_TONE;
			}

			//gRequestSaveChannel = 1;
			main_push_message(RADIO_SAVE_CHANNEL);
			return;
		}
		case MENU_SFT_D:
			gTxVfo->TX_OFFSET_FREQUENCY_DIRECTION = (uint8_t)settingsCurrentSubMenu;
			//gRequestSaveChannel                   = 1;
			main_push_message(RADIO_SAVE_CHANNEL);
			return;

		case MENU_OFFSET:
			gTxVfo->TX_OFFSET_FREQUENCY = settingsCurrentSubMenu;
			//gRequestSaveChannel         = 1;
			main_push_message(RADIO_SAVE_CHANNEL);
			return;

		case MENU_SCR:
			gTxVfo->SCRAMBLING_TYPE = (uint8_t)settingsCurrentSubMenu;
			//gRequestSaveChannel     = 1;
			main_push_message(RADIO_SAVE_CHANNEL);
			return;

		case MENU_BCL:
			gTxVfo->BUSY_CHANNEL_LOCK = (uint8_t)settingsCurrentSubMenu;
			//gRequestSaveChannel       = 1;
			main_push_message(RADIO_SAVE_CHANNEL);
			return;

		/*case MENU_MEM_CH:
			gTxVfo->CHANNEL_SAVE = (uint8_t)settingsCurrentSubMenu;
			#if 0
				gMrChannel[0] = (uint8_t)settingsCurrentSubMenu;
			#else
				gMrChannel[gSettings.activeVFO] = (uint8_t)settingsCurrentSubMenu;
			#endif
			//gRequestSaveChannel = 2;
			//gVfoConfigureMode   = VFO_CONFIGURE_RELOAD;
			//gFlagResetVfos      = true;
			return;*/

		//case MENU_MEM_NAME:
			/*for (int i = 9; i >= 0; i--) {
				if (edit[i] != ' ' && edit[i] != '_' && edit[i] != 0x00 && edit[i] != 0xff)
					break;
				edit[i] = ' ';
			}*/

			//SETTINGS_SaveChannelName(settingsCurrentSubMenu, edit);
			//return;


		case MENU_COMPAND:
			gTxVfo->Compander = (uint8_t)settingsCurrentSubMenu;
			//SETTINGS_UpdateChannel(gTxVfo->CHANNEL_SAVE, gTxVfo, true);
			//gVfoConfigureMode = VFO_CONFIGURE;
			//gFlagResetVfos    = true;
//			gRequestSaveChannel = 1;
			return;

		case MENU_PTT_ID:
			//gTxVfo->DTMF_PTT_ID_TX_MODE = settingsCurrentSubMenu;
			//gRequestSaveChannel         = 1;
			main_push_message(RADIO_SAVE_CHANNEL);
			return;

		//case MENU_DEL_CH:
			/*SETTINGS_UpdateChannel(settingsCurrentSubMenu, NULL, false);
			gVfoConfigureMode = VFO_CONFIGURE_RELOAD;
			gFlagResetVfos    = true;*/
			//return;

	}

	//gRequestSaveSettings = true;
}

void MenuVFO_initFunction() {
    settingsSubMenuTime = xTaskGetTickCount();
    settingsShowSubMenu = false;
    settingsSubMenuActive = false;
}

void MenuVFO_renderFunction() {

    UI_drawFastVLine(2, 9, 54, true);

    SettingsMenu_showList();

    if (settingsShowSubMenu) {
        SettingsMenu_loadSubList();
    } else {
        if (xTaskGetTickCount() - settingsSubMenuTime > pdMS_TO_TICKS(SETTINGS_TIMESHOW_SUB)) {
			if( GUI_inputGetSize() == 1 ) {
				const uint8_t inputValue = (uint8_t)GUI_inputGetNumberClear();
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
						const uint8_t inputValue = (uint8_t)GUI_inputGetNumberClear();
						if ( inputValue > 0 && inputValue < MENU_VFO_SIZE ) {
							settingsCurrentMenu = inputValue - 1;
						}
					}
					settingsSubMenuTime = xTaskGetTickCount();
					settingsShowSubMenu = false;
				} else {
					if (getMenuID() == MENU_OFFSET) {
						GUI_inputAppendKey(key, 9, true);
						if (GUI_inputGetSize() > 0) {
							settingsCurrentSubMenu = GUI_inputGetNumber();
						}
					}
				}
			}
			break;
		case KEY_UP:
			if ( state == KEY_PRESSED || state == KEY_LONG_PRESSED_CONT ) {
				if ( settingsSubMenuActive ) {
					if ( settingsCurrentSubMenu > settingsSubmenuMin ) {
						if (getMenuID() == MENU_OFFSET) {
							settingsCurrentSubMenu -= gTxVfo->STEP_SETTING;
						} else {
							settingsCurrentSubMenu--;
						}
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
						if (getMenuID() == MENU_OFFSET) {
							settingsCurrentSubMenu += gTxVfo->STEP_SETTING;
						} else {
							settingsCurrentSubMenu++;
						}
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
