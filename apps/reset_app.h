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


bool resetALL = true;
uint8_t resetStatus = 0;

void ResetAPP_initFunction() {
    app_push_message(APP_MSG_TIMEOUT_NO_RETURN_MAIN);
}

void ResetAPP_renderFunction() {

	UI_drawString(&font_10, TEXT_ALIGN_CENTER, 0, 128, 18, "! ! ! RESET Settings ! ! !", false, true);

    if ( resetStatus == 0 ) {
        UI_drawString(&font_small, TEXT_ALIGN_CENTER, 0, 128, 28, "This option is going to change", true, false);
        UI_drawString(&font_small, TEXT_ALIGN_CENTER, 0, 128, 36, "the Settings and VFO in eeprom !", true, false);
        UI_drawString(&font_small, TEXT_ALIGN_CENTER, 0, 128, 44, "Make a backup before continuing !", true, false);
    
        UI_drawString(&font_10, TEXT_ALIGN_CENTER, 14, 0, 56, " RESET ALL ", !resetALL, resetALL);
        UI_drawString(&font_10, TEXT_ALIGN_CENTER, UI_nextX + 18, 0, 56, " VFO ", resetALL, !resetALL);
    } else if ( resetStatus == 1 ) {
       if (resetALL) {
            UI_drawString(&font_10, TEXT_ALIGN_CENTER, 0, 128, 38, "Resetting ALL", true, false);
       } else {
            UI_drawString(&font_10, TEXT_ALIGN_CENTER, 0, 128, 38, "Resetting VFO", true, false);
       }
    } else {
        UI_drawString(&font_10, TEXT_ALIGN_CENTER, 0, 128, 38, "RESET DONE !", false, true);
        UI_drawString(&font_small, TEXT_ALIGN_CENTER, 0, 128, 52, "Power cycle the radio.", true, false);
    }
    
}

void ResetAPP_keyHandlerFunction(KEY_Code_t key, KEY_State_t state) {
    
    if ( state == KEY_PRESSED ) {
        switch (key)
		{
            case KEY_EXIT:
                // dont exit if is working...
                if ( resetStatus == 0 || resetStatus == 2 ) {
                    if ( gSettings.settingsVersion == SETTINGS_VERSION ) {
                        load_application(APP_MAIN_VFO);
                    } else {
                        main_push_message_value(MAIN_MSG_PLAY_BEEP, BEEP_880HZ_60MS_TRIPLE_BEEP);
                        resetStatus = 0;
                    }
                }
				break;
            case KEY_UP:
            case KEY_DOWN:
                if ( resetStatus == 0 ) {
                    resetALL = !resetALL;
                }
                break;
            case KEY_MENU:
                if ( resetStatus == 0 ) {
                    resetStatus = 1;
                    // call reset function
                    SETTINGS_FactoryReset(resetALL);
                    resetStatus = 2;
                }
                break;
            default:
                break;
        }
    }
}

app_t APPReset = {
    .showStatusLine = false,
    .init = ResetAPP_initFunction,
    .render = ResetAPP_renderFunction,
    .keyHandler = ResetAPP_keyHandlerFunction
};

