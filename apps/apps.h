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

#ifndef APPS_MSG_H
#define APPS_MSG_H

#include "driver/keyboard.h"

enum APPS_e {
	APP_WELCOME = 0,
    APP_EMPTY,
    APP_MAIN_VFO,
    APP_MENU,
    APP_VFO_SETTINGS,
    APP_MAIN_SETTINGS,
};
typedef enum APPS_e APPS_t;


enum APPS_Popup_e {
    APP_POPUP_NONE = 0,
	APP_POPUP_TXP,
    APP_POPUP_W_N,
    APP_POPUP_AM,
    APP_POPUP_INFO,
};
typedef enum APPS_Popup_e APPS_Popup_t;


typedef void (*InitFunction)();
typedef void (*RenderFunction)();
typedef void (*TimeoutFunction)();
typedef void (*RenderFunctionPopup)(APPS_Popup_t popup);
typedef void (*KeyHandlerFunction)(KEY_Code_t key, KEY_State_t state);
typedef void (*KeyHandlerFunctionPopup)(KEY_Code_t key, KEY_State_t state, APPS_Popup_t popup);

typedef struct app_struct {
    bool showStatusLine;
    InitFunction init;
    RenderFunction render;
    TimeoutFunction timeoutHandler;

    KeyHandlerFunction keyHandler;

    RenderFunctionPopup renderPopup;
    KeyHandlerFunctionPopup keyHandlerPopup;
} app_t;


#endif