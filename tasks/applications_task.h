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

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "task_messages.h"
#include "apps.h"

void applications_task_init(void);

void app_push_message_value(APP_MSG_t msg, uint32_t value);
void app_push_message_key(KEY_Code_t key, KEY_State_t state);
void app_push_message(APP_MSG_t msg);

void load_application(APPS_t application);
void application_showPopup(APPS_Popup_t popup, bool autoClose);
void application_closePopup(void);
APPS_Popup_t application_getPopup(void);

