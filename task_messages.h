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

#ifndef TASKS_MSG_H
#define TASKS_MSG_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

enum MAIN_MSG_e
{
	MAIN_MSG_INIT = 0,
	MAIN_MSG_IDLE,
	MAIN_MSG_PLAY_BEEP,
	MAIN_MSG_BKLIGHT_ON,
	MAIN_MSG_BKLIGHT_OFF,

	RADIO_SQUELCH_LOST,
	RADIO_SQUELCH_FOUND,
	RADIO_CTCSS_LOST,
	RADIO_CTCSS_FOUND,
	RADIO_CDCSS_LOST,
	RADIO_CDCSS_FOUND,

	RADIO_VFO_UP,
	RADIO_VFO_DOWN,
	RADIO_VFO_SWITCH,
	RADIO_VFO_SWITCH_MODE,
	RADIO_VFO_CONFIGURE_RELOAD,
	RADIO_SAVE_VFO,
	RADIO_SAVE_CHANNEL,
	RADIO_VFO_CONFIGURE,
	RADIO_VFO_CONFIGURE_CHANNEL,
	RADIO_SAVE_SETTINGS,
	RADIO_RECONFIGURE_VFO,

	RADIO_SET_CHANNEL,
	RADIO_SET_FREQ,

	RADIO_TX,
	RADIO_RX,

	SET_VFO_STATE_NORMAL,
	
};

typedef enum MAIN_MSG_e MAIN_MSG_t;

typedef struct {
	MAIN_MSG_t message;
	uint32_t payload;
} MAIN_Messages_t;


enum APP_MSG_e
{
	APP_MSG_TIMEOUT = 0,
	APP_MSG_TIMEOUT_RETURN_MAIN,
	APP_MSG_TIMEOUT_NO_RETURN_MAIN,
	APP_MSG_IDLE,
	APP_MSG_BACKLIGHT,
	APP_MSG_WAKEUP,
	APP_MSG_RX,
	APP_MSG_TX,
};

typedef enum APP_MSG_e APP_MSG_t;

typedef struct {
	APP_MSG_t message;
	uint32_t payload;
} APP_Messages_t;

#endif