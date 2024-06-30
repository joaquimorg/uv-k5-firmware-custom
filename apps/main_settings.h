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

enum MAIN_Settings_Option_e {
	MENU_TOT = 0,         
	MENU_SAVE,
	MENU_MIC,
	MENU_BAT_TXT,
	MENU_ABR,
	MENU_ABR_MAX,
	MENU_CONTRAST,
	MENU_ABR_ON_TX_RX,
	MENU_BEEP,
	MENU_ROGER,
	MENU_STE,
	MENU_RP_STE,
	MENU_VOL,
	MENU_BATCAL,
	MENU_BATTYP,
};

typedef enum MAIN_Settings_Option_e MAIN_Settings_Option_t;

typedef struct {
	const char      name[11];
	MAIN_Settings_Option_t   menu_id;
} main_settings_item_t;

const main_settings_item_t MainMenuList[] =
{
//   text,     		menu ID
	{"Tx TimeOut", 	MENU_TOT           }, // was "TOT"
	{"Bat Save", 	MENU_SAVE          }, // was "SAVE"
	{"Mic",    		MENU_MIC           },
	{"Bat Txt", 	MENU_BAT_TXT       },
	{"Back Lt", 	MENU_ABR           }, // was "ABR"
	{"BL Max",  	MENU_ABR_MAX       },
	{"LCD Ctr", 	MENU_CONTRAST      },
	{"Blt TRX", 	MENU_ABR_ON_TX_RX  },
	{"Beep",   		MENU_BEEP          },
	{"Roger",  		MENU_ROGER         },
	{"STE",    		MENU_STE           },
	{"RP STE", 		MENU_RP_STE        },
	{"Bat Vol", 	MENU_VOL           }, // was "VOL"
	{"Bat Cali", 	MENU_BATCAL        }, // battery voltage calibration
	{"Bat Type", 	MENU_BATTYP        }, // battery type 1600/2200mAh
};

#define MENU_SETTINGS_SIZE 15

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
	yPos += (uint8_t)((mainSettingsCurrentMenu * 54.0) / MENU_SETTINGS_SIZE);

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

			switch (getMainMenuID()) {
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
            MainSettingsMenu_showSubList(gSubMenu_BACKLIGHT);
			break;

		case MENU_ABR_MAX:
		case MENU_RP_STE:
		case MENU_CONTRAST:
		case MENU_BATCAL:
			showNumber = true;
			break;

		case MENU_ROGER:
            MainSettingsMenu_showSubList(gSubMenu_ROGER);
			break;

		case MENU_MIC:
			showValue = true;
			break;

		case MENU_ABR_ON_TX_RX:
            MainSettingsMenu_showSubList(gSubMenu_RX_TX);
			break;

		case MENU_BEEP:
		case MENU_STE:
        	MainSettingsMenu_showSubList(gSubMenu_OFF_ON);
			break;

		case MENU_TOT:
            MainSettingsMenu_showSubList(gSubMenu_TOT);
			break;

		case MENU_SAVE:
            MainSettingsMenu_showSubList(gSubMenu_SAVE);
			break;

		case MENU_BAT_TXT:
            MainSettingsMenu_showSubList(gSubMenu_BAT_TXT);
			break;

		case MENU_VOL: {
			const uint16_t gBatteryPercentage = (uint16_t)BATTERY_VoltsToPercent(gBatteryVoltageAverage);
			UI_printf(&font_10, TEXT_ALIGN_CENTER, SUB_MENU_X, 125, 26, true, false,	"%u.%02uV", gBatteryVoltageAverage / 100, gBatteryVoltageAverage % 100);
			UI_printf(&font_10, TEXT_ALIGN_CENTER, SUB_MENU_X, 125, 37, true, false,	"%3i%%", gBatteryPercentage);
			UI_printf(&font_10, TEXT_ALIGN_CENTER, SUB_MENU_X, 125, 48, true, false,	"%dmA", gBatteryCurrent);
			break;
		}

		case MENU_BATTYP:
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

		case MENU_ABR:
			mainSettingsCurrentSubMenu = gSettings.backlightTime;
			break;

		case MENU_ABR_MAX:
			mainSettingsCurrentSubMenu = gSettings.backlight;
			break;

		case MENU_CONTRAST:
			mainSettingsCurrentSubMenu = gSettings.contrast;
			break;		

		case MENU_ABR_ON_TX_RX:
			mainSettingsCurrentSubMenu = gSettings.backlightOnSquelch;
			break;

		case MENU_BEEP:
			mainSettingsCurrentSubMenu = gSettings.beep;
			break;

		case MENU_TOT:
			mainSettingsCurrentSubMenu = gSettings.txTime;
			break;

		case MENU_STE:
			mainSettingsCurrentSubMenu = gSettings.ste;
			break;

		case MENU_RP_STE:
			mainSettingsCurrentSubMenu = gSettings.repeaterSte;
			break;

		case MENU_MIC:
			mainSettingsCurrentSubMenu = gSettings.micGain;
			break;

		case MENU_BAT_TXT:
			mainSettingsCurrentSubMenu = gSettings.batteryStyle;
			break;

		case MENU_ROGER:
			mainSettingsCurrentSubMenu = gSettings.roger;
			break;

		case MENU_BATCAL:
			mainSettingsCurrentSubMenu = gSettings.batteryCalibration;
			break;

		case MENU_BATTYP:
			mainSettingsCurrentSubMenu = gSettings.batteryType;
			break;

		default:
			break;
	}
}


int MainSettingsMenu_GetLimits(uint8_t menu_id, uint16_t *pMin, uint16_t *pMax)
{
	*pMin = 0;
	switch (menu_id)
	{
		case MENU_ABR:
			*pMax = ARRAY_SIZE(gSubMenu_BACKLIGHT) - 1;
			break;

		case MENU_ABR_MAX:
			*pMin = 1;
			*pMax = 10;
			break;

		case MENU_CONTRAST:
			*pMin = 1;
			*pMax = 63;
			break;	

		case MENU_ROGER:
			*pMax = ARRAY_SIZE(gSubMenu_ROGER) - 1;
			break;

		case MENU_ABR_ON_TX_RX:
			*pMax = ARRAY_SIZE(gSubMenu_RX_TX) - 1;
			break;

		case MENU_BEEP:
		case MENU_STE:
			*pMax = ARRAY_SIZE(gSubMenu_OFF_ON) - 1;
			break;

		case MENU_TOT:
			*pMax = ARRAY_SIZE(gSubMenu_TOT) - 1;
			break;

		case MENU_RP_STE:
			*pMax = 10;
			break;

		case MENU_SAVE:
			*pMax = ARRAY_SIZE(gSubMenu_SAVE) - 1;
			break;

		case MENU_MIC:
			*pMax = 6;
			break;

		case MENU_BAT_TXT:
			*pMax = ARRAY_SIZE(gSubMenu_BAT_TXT) - 1;
			break;

		case MENU_BATCAL:
			*pMin = 1600;
			*pMax = 2200;
			break;

		case MENU_BATTYP:
			*pMax = 1;
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
	
	switch (getMainMenuID())
	{

		case MENU_SAVE:
			gSettings.batSave = (uint16_t)(mainSettingsCurrentSubMenu & 0x0F);
			break;

		case MENU_ABR:
			gSettings.backlightTime = (uint16_t)(mainSettingsCurrentSubMenu & 0x0F);
			app_push_message(APP_MSG_BACKLIGHT);
			break;

		case MENU_ABR_MAX:
			gSettings.backlight = (uint16_t)(mainSettingsCurrentSubMenu & 0x0F);
			main_push_message(MAIN_MSG_BKLIGHT_ON);
			break;			

		case MENU_CONTRAST:
			gSettings.contrast = (uint16_t)(mainSettingsCurrentSubMenu & 0x0F);
			//ST7565_SetContrast(gSettings.contrast);
			break;

		case MENU_ABR_ON_TX_RX:
			gSettings.backlightOnSquelch = (uint16_t)(mainSettingsCurrentSubMenu & 0x03);
			break;

		case MENU_BEEP:
			gSettings.beep = (uint16_t)(mainSettingsCurrentSubMenu & 0x01);
			break;

		case MENU_TOT:
			gSettings.txTime = (uint16_t)(mainSettingsCurrentSubMenu & 0x0F);
			break;

		case MENU_STE:
			gSettings.ste = (uint16_t)(mainSettingsCurrentSubMenu & 0x01);
			break;

		case MENU_RP_STE:
			gSettings.repeaterSte = (uint16_t)(mainSettingsCurrentSubMenu & 0x01);
			break;

		case MENU_MIC:
			gSettings.micGain = (uint16_t)(mainSettingsCurrentSubMenu & 0x0F);
			//SETTINGS_LoadCalibration();
			//gFlagReconfigureVfos = true;
			break;

		case MENU_BAT_TXT:
			gSettings.batteryStyle = (uint16_t)(mainSettingsCurrentSubMenu & 0x03);
			break;

		case MENU_ROGER:
			gSettings.roger = (uint16_t)(mainSettingsCurrentSubMenu & 0x03);
			break;

		case MENU_BATCAL: 
			// voltages are averages between discharge curves of 1600 and 2200 mAh
			gSettings.batteryCalibration = (uint16_t)(mainSettingsCurrentSubMenu & 0xFFF);
			break;

		case MENU_BATTYP:
			gSettings.batteryType = (uint16_t)(mainSettingsCurrentSubMenu & 0x03);
			break;

		default:
			break;
	}

	gRequestSaveSettings = true;
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

void MainSettings_exitFunction() {
	main_push_message(RADIO_SAVE_SETTINGS);
}

app_t APPMainSettings = {
    .showStatusLine = true,
    .init = MainSettings_initFunction,
    .render = MainSettings_renderFunction,
    .keyHandler = MainSettings_keyHandlerFunction,
	.exit = MainSettings_exitFunction
};
