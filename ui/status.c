/* Copyright 2023 Dual Tachyon
 * https://github.com/DualTachyon
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

#include <string.h>

#include "driver/keyboard.h"
#include "driver/st7565.h"
#include "functions.h"
#include "helper/battery.h"
#include "misc.h"
#include "settings.h"
#include "ui/status.h"
#include "gui/ui.h"
#include "gui/gui.h"


#define STATUS_SPACE 2
#define STATUS_YPOS 5

void UI_DisplayStatus()
{
	//gUpdateStatus = false;
	memset(gFrameBuffer[0], 0, sizeof(gFrameBuffer[0]));
	//UI_fillRect(0, 0, 128, 8, false);	

	//UI_fillRectWithChessboard(0, 8, 128, 1, false);

	GUI_drawBattery();

	switch (gSettings.batteryStyle) {
		default:
		case 0:
			UI_nextX = 18;
			break;

		case 1:	{	// voltage
			const uint16_t voltage = (gBatteryVoltageAverage <= 999) ? gBatteryVoltageAverage : 999; // limit to 9.99V
			UI_printf(&font_small, TEXT_ALIGN_LEFT, 18, 0, STATUS_YPOS, true, false, "%u.%02uV", voltage / 100, voltage % 100);
			break;
		}

		case 2: {	// percentage
			const uint8_t gBatteryPercentage = (uint8_t)BATTERY_VoltsToPercent(gBatteryVoltageAverage);
			UI_printf(&font_small, TEXT_ALIGN_LEFT, 18, 0, STATUS_YPOS, true, false, "% 3i%%", gBatteryPercentage);
			break;
		}
	}

	// USB-C charge indicator
	if (gChargingWithTypeC) {
		UI_printf(&font_small, TEXT_ALIGN_LEFT, UI_nextX + STATUS_SPACE, 0, STATUS_YPOS, true, false, "\\");
	}

	// POWER-SAVE indicator
	if (gCurrentFunction == FUNCTION_TRANSMIT) {
		UI_printf(&font_small, TEXT_ALIGN_LEFT, UI_nextX + STATUS_SPACE, 0, STATUS_YPOS, true, false, "\"");
	}
	else if (FUNCTION_IsRx()) {
		UI_printf(&font_small, TEXT_ALIGN_LEFT, UI_nextX + STATUS_SPACE, 0, STATUS_YPOS, true, false, "$");
	}
	else if (gCurrentFunction == FUNCTION_POWER_SAVE) {
		UI_printf(&font_small, TEXT_ALIGN_LEFT, UI_nextX + STATUS_SPACE, 0, STATUS_YPOS, true, false, "^");
	}

	// KEY-LOCK indicator
	if (gSettings.keylock) {
		UI_printf(&font_small, TEXT_ALIGN_LEFT, UI_nextX + STATUS_SPACE, 0, STATUS_YPOS, true, false, "]");		
	}
	else if (gWasFKeyPressed) {
		UI_printf(&font_small, TEXT_ALIGN_LEFT, UI_nextX + STATUS_SPACE, 0, STATUS_YPOS, true, false, "[");
	}

	if(gSettings.crossBand) { // XB - crossband
		UI_printf(&font_small, TEXT_ALIGN_LEFT, UI_nextX + STATUS_SPACE, 0, STATUS_YPOS, true, false, "XB");
	}

	if(gRequestSaveSettings || gRequestSaveVfoIndices || gRequestSaveChannel) { // Settings not saved indicator
		UI_printf(&font_small, TEXT_ALIGN_LEFT, UI_nextX + STATUS_SPACE, 0, STATUS_YPOS, true, false, "{");
	}

	// Show App Status / Name
	UI_printf(&font_small, TEXT_ALIGN_RIGHT, 64, 128, STATUS_YPOS, true, false, "%s", gMainAppStatus);
	// **************	

	UI_statusUpdate();
}
