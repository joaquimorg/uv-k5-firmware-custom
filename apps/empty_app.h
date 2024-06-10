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

void EmptyAPP_initFunction() {    
    
}

void EmptyAPP_renderFunction() {

	UI_drawString(&font_10, TEXT_ALIGN_CENTER, 0, 128, 22, "Open Radio FW", true, false);    
    UI_drawString(&font_10, TEXT_ALIGN_CENTER, 0, 128, 42, "Empty APP", true, false);
}

void EmptyAPP_keyHandlerFunction(KEY_Code_t key, KEY_State_t state) {
    
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

app_t APPEmptyAPP = {
    .showStatusLine = true,
    .init = EmptyAPP_initFunction,
    .render = EmptyAPP_renderFunction,
    .keyHandler = EmptyAPP_keyHandlerFunction
};

