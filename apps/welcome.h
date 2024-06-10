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

void Welcome_renderFunction() {    
    
    const uint8_t batteryPercentage = BATTERY_VoltsToPercent(gBatteryVoltageAverage);

	UI_drawString(&font_10, TEXT_ALIGN_CENTER, 0, 128, 18, "Open Radio FW", true, false);    
    UI_drawString(&font_10, TEXT_ALIGN_CENTER, 0, 128, 32, "version "VERSION_STRING, false, true);
    UI_drawString(&font_small, TEXT_ALIGN_CENTER, 0, 128, 42, __DATE__, true, false);
	
    UI_printf(&font_small, TEXT_ALIGN_CENTER, 0, 128, 50, true, false,	"^ %u.%02uV - %3i%%", gBatteryVoltageAverage / 100, gBatteryVoltageAverage % 100, batteryPercentage);
	
    UI_drawString(&font_10, TEXT_ALIGN_CENTER, 0, 128, 60, "2024 - joaquim.org", true, false);

}

void Welcome_keyHandlerFunction(KEY_Code_t key, KEY_State_t state) {
    
    if ( state == KEY_PRESSED ) {
        switch (key)
		{
            case KEY_EXIT:
				load_application(APP_MAIN_VFO);				
				break;
            default:
                break;
        }
    }
}

app_t APPWelcome = {
    .showStatusLine = false,
    .render = Welcome_renderFunction,
    .keyHandler = Welcome_keyHandlerFunction
};

