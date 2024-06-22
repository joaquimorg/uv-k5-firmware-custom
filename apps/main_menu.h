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

uint8_t mainMenuCurrentMenu = 0;

typedef struct {
	const char      name[16];
} main_menu_item_t;

const main_menu_item_t mainMenuList[] =
{
    { "Main Settings" },
    { "VFO Settings" },
    { "Spectrum" },
    { "Messenger" },
    { "FM Radio" },
    { "About" },
};

#define MAIN_MENU_SIZE 6

void MainMenu_showList() {
    uint8_t yPos = 15;
	const uint8_t offset = (uint8_t)Clamp(mainMenuCurrentMenu - 2, 0, MAIN_MENU_SIZE - 5);

    for (uint8_t i = 0; i < 5; i++) {
		if ( (i + offset) < MAIN_MENU_SIZE ) {
			bool isFill = mainMenuCurrentMenu == i + offset ? true : false;
		
			UI_printf(&font_small, TEXT_ALIGN_LEFT, 8, 0, yPos - 1, true, false, "%0.2i", (i + 1) + offset);
		
			UI_printf(&font_10, TEXT_ALIGN_LEFT, 18, 0, yPos, !isFill, isFill,
				"%s", mainMenuList[i + offset].name);
			yPos += 11;
		}
    }

    yPos = 10;
    yPos += (uint8_t)((( (100 * mainMenuCurrentMenu) / MAIN_MENU_SIZE ) / 100.0) * 57);

    UI_drawFastVLine(1, yPos - 1, 3, true);
    UI_drawFastVLine(3, yPos - 1, 3, true);
}

void MenuAPP_initFunction() {
    mainMenuCurrentMenu = 0;
}

void MenuAPP_renderFunction() {
    UI_drawFastVLine(2, 9, 54, true);
    MainMenu_showList();
}

void MenuAPP_LoadSelection() {
    switch ( mainMenuCurrentMenu ) {
        case 0:
            load_application(APP_MAIN_SETTINGS);
            break;
        case 1:
            load_application(APP_VFO_SETTINGS);
            break;
        case 2:
            load_application(APP_EMPTY);
            break;
        case 3:
            load_application(APP_EMPTY);
            break;
        case 4:
            load_application(APP_EMPTY);
            break;
        case 5:
            load_application(APP_WELCOME);
            break;
    }
}

void MenuAPP_keyHandlerFunction(KEY_Code_t key, KEY_State_t state) {
    
    if ( state == KEY_PRESSED ) {
        switch (key)
		{
            case KEY_UP:
                if ( mainMenuCurrentMenu > 0 ) {
                    mainMenuCurrentMenu--;
                } else {
                    mainMenuCurrentMenu = MAIN_MENU_SIZE - 1;
                }
                break;

            case KEY_DOWN:
                if ( mainMenuCurrentMenu < MAIN_MENU_SIZE - 1 ) {
                    mainMenuCurrentMenu++;
                } else {
                    mainMenuCurrentMenu = 0;
                }
                break;
            case KEY_MENU:
                MenuAPP_LoadSelection();
                break;

            case KEY_EXIT:
				load_application(APP_MAIN_VFO);				
				break;
            default:
                break;
        }
    }
}

app_t APPMenuAPP = {
    .showStatusLine = true,
    .init = MenuAPP_initFunction,
    .render = MenuAPP_renderFunction,
    .keyHandler = MenuAPP_keyHandlerFunction
};

