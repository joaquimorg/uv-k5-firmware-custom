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

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "debugging.h"

#include "settings.h"
#include "applications_task.h"
#include "task_main.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

#include "driver/keyboard.h"
#include "driver/backlight.h"

#ifdef ENABLE_UART
	#include "driver/uart.h"
#endif

#include "misc.h"
#include "audio.h"
#include "gui/ui.h"
#include "gui/gui.h"
#include "ui/status.h"
#include "apps/apps.h"
#include "apps/welcome.h"
#include "apps/main_vfo.h"
#include "apps/settings_vfo.h"
#include "apps/main_menu.h"
#include "apps/main_settings.h"
#include "apps/empty_app.h"

#define QUEUE_LENGTH    10
#define APP_ITEM_SIZE   sizeof( APP_Messages_t )

StackType_t app_task_stack[configMINIMAL_STACK_SIZE + 150];
StaticTask_t app_task_buffer;

static StaticQueue_t appTasksQueue;
QueueHandle_t appTasksMsgQueue;
uint8_t appQueueStorageArea[ QUEUE_LENGTH * APP_ITEM_SIZE ];

/*-----------------------------------------------------------*/

static app_t *currentApplication;
static APPS_t currentApp;

TimerHandle_t idleTimer;
StaticTimer_t idleTimerBuffer;

TimerHandle_t lightTimer;
StaticTimer_t lightTimerBuffer;

TimerHandle_t renderTimer;
StaticTimer_t renderTimerBuffer;
/*-----------------------------------------------------------*/

static APPS_Popup_t currentAppPopup;
static bool popupAutoClose = false;
static bool autoReturntoMain = true;

/*-----------------------------------------------------------*/

void light_timer_callback(TimerHandle_t xTimer) {
    (void)xTimer;
    main_push_message(MAIN_MSG_BKLIGHT_OFF);
    //LogUartf("MAIN_MSG_BKLIGHT_OFF\r\n");
}

void idle_timer_callback(TimerHandle_t xTimer) {

	(void)xTimer;	

    if (gWasFKeyPressed) {
        gWasFKeyPressed = false;
        main_push_message_value(MAIN_MSG_PLAY_BEEP, BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL);
    }

    if(GUI_inputNotEmpty()) {
        GUI_inputReset();
    }

    app_push_message(APP_MSG_TIMEOUT);
    //xTimerStart(xTimer, 0);
    //LogUartf("APP_MSG_TIMEOUT\r\n");
}

void render_timer_callback(TimerHandle_t xTimer) {

    UI_displayClear();
    
    if(GUI_inputNotEmpty()) {
        GUI_updateCursor();
    }

	if (currentApplication->render) {
        currentApplication->render();
    }

    if (currentApplication->showStatusLine) {
        UI_DisplayStatus();
    }

    if ( currentAppPopup != APP_POPUP_NONE ) {
        if (currentApplication->renderPopup) {
            currentApplication->renderPopup(currentAppPopup);
        }       
    }

    UI_statusUpdate();
    UI_displayUpdate();

    xTimerStart(xTimer, 0);

}

void keyboard_callback(KEY_Code_t key, KEY_State_t state) {
    xTimerReset(idleTimer, 0);
    
    if ( !BACKLIGHT_IsOn() ) {
        app_push_message(APP_MSG_WAKEUP);
    } else {
        xTimerReset(lightTimer, 0);
    }
    
    if ( key == KEY_PTT ) {
        if ( state == KEY_PRESSED ) {
            if (gCurrentFunction != FUNCTION_TRANSMIT) {
                main_push_message(RADIO_TX);
            }            
        } else if ( state == KEY_RELEASED ) {
            main_push_message(RADIO_RX); 
        }
        return;
        //LogUartf("PTT : %i\r\n", state);   

    }

    if ( currentAppPopup != APP_POPUP_NONE ) {
        if (currentApplication->keyHandlerPopup) {
            currentApplication->keyHandlerPopup(key, state, currentAppPopup);
        }
    } else {
        if (currentApplication->keyHandler) {
            currentApplication->keyHandler(key, state);
        }
    }

    if ( state != KEY_RELEASED && state != KEY_LONG_PRESSED_CONT ) {
        main_push_message_value(MAIN_MSG_PLAY_BEEP, BEEP_1KHZ_60MS_OPTIONAL);
    }
}

/*-----------------------------------------------------------*/

void app_task(void* arg) {
	(void)arg;

    currentAppPopup = APP_POPUP_NONE;

    appTasksMsgQueue = xQueueCreateStatic(QUEUE_LENGTH, APP_ITEM_SIZE, appQueueStorageArea, &appTasksQueue);    

    renderTimer = xTimerCreateStatic("render", pdMS_TO_TICKS(50), pdFALSE, NULL, render_timer_callback, &renderTimerBuffer);
    idleTimer = xTimerCreateStatic("idle", pdMS_TO_TICKS(5000), pdTRUE, NULL, idle_timer_callback, &idleTimerBuffer);
    lightTimer = xTimerCreateStatic("light", pdMS_TO_TICKS(BACKLIGHT_getTime()), pdFALSE, NULL, light_timer_callback, &lightTimerBuffer);    
    
    xTimerStart(idleTimer, 0);
    xTimerStart(renderTimer, 0);
    xTimerStart(lightTimer, 0);

    load_application(APP_WELCOME);
    
    //LogUartf("Task APPs Ready\r\n");

    keyboard_init();
    
    for (;;) {

        APP_Messages_t msg;

    	if (xQueueReceive(appTasksMsgQueue, &msg, 20)) {

			switch(msg.message) {
                case APP_MSG_KEY:
                    keyboard_callback(msg.key_code, msg.key_state);
                    break;

                case APP_MSG_TIMEOUT:
                    if ( currentApp != APP_MAIN_VFO && autoReturntoMain ) {
                        load_application(APP_MAIN_VFO);
                    } else {
                        if( currentAppPopup != APP_POPUP_NONE && popupAutoClose) {
                            if ( currentAppPopup == APP_POPUP_INFO ) {
                                main_push_message(SET_VFO_STATE_NORMAL);
                            }
                            popupAutoClose = false;
                            currentAppPopup = APP_POPUP_NONE;
                            xTimerReset( idleTimer, 0 );
                        } else if( currentAppPopup == APP_POPUP_NONE) {
                             if (currentApplication->timeoutHandler) {
                                currentApplication->timeoutHandler();
                            }
                        }
                    }
                    break;
                case APP_MSG_TIMEOUT_RETURN_MAIN:
                    autoReturntoMain = true;
                    break;

	            case APP_MSG_TIMEOUT_NO_RETURN_MAIN:
                    autoReturntoMain = false;
                    break;

				case APP_MSG_WAKEUP:
                    xTimerReset(lightTimer, 0);
                    if ( !BACKLIGHT_IsOn() ) {
					    main_push_message(MAIN_MSG_BKLIGHT_ON);                    
                    }
					break;

                case APP_MSG_BACKLIGHT:
                    xTimerChangePeriod( lightTimer, pdMS_TO_TICKS(BACKLIGHT_getTime()), 0 );
                    break;

                case APP_MSG_IDLE:
					//global_status.isRX = false;
					break;

                case APP_MSG_RX:
                    //global_status.isRX = true;
                    app_push_message(APP_MSG_WAKEUP);
                    xTimerReset( idleTimer, 0 );					
                    break;

                case APP_MSG_TX:
                    break;
                default:
                    break;
            }

        }

    }
}


void app_push_message_value(APP_MSG_t msg, uint32_t value) {
    APP_Messages_t appMSG = { msg, value, 0, 0 };
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendToBackFromISR(appTasksMsgQueue, (void *)&appMSG, &xHigherPriorityTaskWoken);
}

void app_push_message_key(KEY_Code_t key, KEY_State_t state) {
    APP_Messages_t appMSG = { APP_MSG_KEY, 0, key, state };
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendToBackFromISR(appTasksMsgQueue, (void *)&appMSG, &xHigherPriorityTaskWoken);
}

void app_push_message(APP_MSG_t msg) {
    app_push_message_value(msg, 0);
}

/*-----------------------------------------------------------*/

void change_application(app_t *application) {
    if ( currentApplication != application ) {
        xTimerStop(renderTimer, 0);
        currentApplication = application;        
        if (currentApplication->init) {
            currentApplication->init();
        }
        xTimerStart(renderTimer, 0);
    }    
}

void load_application(APPS_t application) {
    clearMainAppStatus();
    switch (application) {
        case APP_WELCOME:
            change_application(&APPWelcome);
            break;

        case APP_MAIN_VFO:            
            change_application(&APPMainVFO);
            break;

        case APP_EMPTY:
            setMainAppStatus("Demo APP...");
            change_application(&APPEmptyAPP);
            break;

        case APP_MENU:
            setMainAppStatus("Main Menu");
            change_application(&APPMenuAPP);
            break;            

        case APP_VFO_SETTINGS:
            setMainAppStatus("VFO Settings");
            change_application(&APPVFOSettings);
            break;

        case APP_MAIN_SETTINGS:
            setMainAppStatus("Main Settings");
            change_application(&APPMainSettings);
            break;            

        default:
            break;
    }
    currentApp = application;
}

/*-----------------------------------------------------------*/

void application_showPopup(APPS_Popup_t popup, bool autoClose) {
    currentAppPopup = popup;
    popupAutoClose = autoClose;
    xTimerReset( idleTimer, 0 );
}

void application_closePopup(void) {
    if ( currentAppPopup != APP_POPUP_NONE ) {
        currentAppPopup = APP_POPUP_NONE;
        popupAutoClose = false;
        xTimerReset( idleTimer, 0 );
    }
}

APPS_Popup_t application_getPopup(void) {
    return currentAppPopup;
}

/*-----------------------------------------------------------*/

void applications_task_init(void) {

     xTaskCreateStatic(
		app_task,
		"APP",
		ARRAY_SIZE(app_task_stack),
		NULL,
		3 + tskIDLE_PRIORITY,
		app_task_stack,
		&app_task_buffer
	);

}