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


#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "settings.h"
#include "driver/st7565.h"
#include "driver/keyboard.h"
#include "gui.h"
#include "ui.h"

#ifdef ENABLE_UART
	#include "driver/uart.h"
#endif

char inputKeys[13];
int8_t inputKeysIndex = -1;
bool showCursor = false;
uint8_t showCursorCount = 0;

void GUI_drawBattery(void) {

	const uint8_t gBatteryPercentage = BATTERY_VoltsToPercent(gBatteryVoltageAverage);
	const uint8_t maxBattBars = 10;
	const uint8_t battBars = (gBatteryPercentage / 100.0) * maxBattBars;

	gFrameBuffer[0][2] = 0x7E;
	gFrameBuffer[0][maxBattBars + 3] = 0x3C;
	gFrameBuffer[0][maxBattBars + 4] = 0x3C;
	for (uint8_t i = 0; i < maxBattBars; i++) {
		if ( i <= battBars ) {
			gFrameBuffer[0][3 + i] = 0x7E;
		} else {
			gFrameBuffer[0][3 + i] = 0x42;
		}
	}
}

void GUI_updateCursor() {	
	if ( ++showCursorCount > 5 ) {
		showCursorCount = 0;
		showCursor = !showCursor;
	}
}

void GUI_inputAppendKey(const KEY_Code_t key, uint8_t length, bool decimal) {
	(void)decimal;
	if (inputKeysIndex == -1 && key != KEY_STAR && key != KEY_F) {
		memset(inputKeys, 0x00, sizeof(inputKeys));
		inputKeysIndex = 0;
	}

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
			if (inputKeysIndex < length) {
				inputKeys[inputKeysIndex++] = '0' + key;
			}
			break;
		case KEY_F:
			if (inputKeysIndex > 0) {
				inputKeys[--inputKeysIndex] = 0x00;
				gWasFKeyPressed = false;
			}			
			break;
		default:
			break;
	}	

}

uint8_t GUI_inputGetSize() {
	if (inputKeysIndex >= 0) {
		return inputKeysIndex;
	} else {
		return 0;
	}
}


bool GUI_inputNotEmpty() {
	if (inputKeysIndex == -1) {
		return false;
	} else {
		return true;
	}
}

void GUI_inputReset() {
	inputKeysIndex = -1;
}

uint32_t string_to_uint32(const char *str) {
    uint32_t result = 0;
    const uint8_t len = strlen(str);

    for (uint8_t i = 0; i < len; i++) {
        if (str[i] != '.') {
            result = result * 10 + (str[i] - '0');
        }
    }

    return result;
}

uint32_t GUI_inputGetNumber() {
	return (uint32_t)string_to_uint32(inputKeys);	
}

uint32_t GUI_inputGetNumberClear() {
	inputKeysIndex = -1;
	return (uint32_t)string_to_uint32(inputKeys);	
}


void GUI_inputShowFreq(const char *title) {
	const uint8_t popupW = 80;
	const uint8_t popupH = 30;	
    uint8_t startX;
    uint8_t startY;
	uint32_t inputValue = (uint32_t)string_to_uint32(inputKeys);
    GUI_showPopup(popupW, popupH, &startX, &startY);
    UI_printf(&font_small, TEXT_ALIGN_CENTER, startX, startX + popupW - 2, startY, true, false, title);
	if ( inputValue >= _1GHz_in_KHz ) {
		UI_printf(&font_10, TEXT_ALIGN_CENTER, startX, startX + popupW - 2, startY + 14, true, false, "%1u.%03u.%03u.%02u", (inputValue / 100000000), (inputValue / 100000) % 1000, (inputValue % 100000) / 100, (inputValue % 100));	
	} else {
		UI_printf(&font_10, TEXT_ALIGN_CENTER, startX, startX + popupW - 2, startY + 14, true, false, "%03u.%03u.%02u", (inputValue / 100000) % 1000, (inputValue % 100000) / 100, (inputValue % 100));	
	}
	if ( showCursor ) {
		UI_drawString(&font_10, TEXT_ALIGN_CENTER, UI_nextX, 0, startY + 14, "`", true, false);
	}
}


void GUI_inputShow(const char *title, const char *prefix) {
	const uint8_t popupW = 80;
	const uint8_t popupH = 30;
    uint8_t startX;
    uint8_t startY;
    GUI_showPopup(popupW, popupH, &startX, &startY);
    UI_printf(&font_small, TEXT_ALIGN_CENTER, startX, startX + popupW - 2, startY, true, false, title);
	UI_printf(&font_10, TEXT_ALIGN_CENTER, startX, startX + popupW - 2, startY + 14, true, false, "%s%s", prefix, inputKeys);
	if ( showCursor ) {
		UI_drawString(&font_10, TEXT_ALIGN_CENTER, UI_nextX, 0, startY + 14, "`", true, false);
	}
}

void GUI_showPopup(uint8_t popupW, uint8_t popupH, uint8_t *startX, uint8_t *startY) {
	//const uint8_t popupW = 68;
	//const uint8_t popupH = 42;

	*startX = (LCD_WIDTH / 2) - (popupW / 2);
	*startY = (LCD_HEIGHT / 2) - (popupH / 2);
	
	UI_fillRect(*startX, *startY, popupW, popupH, false);

	//UI_fillRectWithChessboard(*startX + 3, *startY + popupH - 1, popupW - 2, 3, true);
	//UI_fillRectWithChessboard(*startX + 4, *startY + popupH + 2, popupW - 3, 1, true);
	
	//UI_fillRectWithChessboard(*startX + popupW - 1, *startY + 3, 3, popupH - 2, true);
	//UI_fillRectWithChessboard(*startX + popupW + 2, *startY + 5, 1, popupH - 4, true);
	
	UI_drawRoundRect(*startX - 1, *startY - 1, popupW + 3, popupH + 3, 5, false);
	//UI_drawRoundRect(*startX - 2, *startY - 2, popupW + 4, popupH + 4, 5, false);
	//UI_drawRoundRect(*startX - 3, *startY - 3, popupW + 4, popupH + 4, 5, false);

	UI_drawRoundRect(*startX, *startY, popupW, popupH, 5, true);
	UI_drawRoundRect(*startX, *startY, popupW + 1, popupH + 1, 5, true);	

	*startX = *startX + 1;
	*startY = *startY + 7;
	
}

int Clamp(int v, int min, int max) {
  return v <= min ? min : (v >= max ? max : v);
}

void popupListCalc(uint8_t listSelected, uint8_t listSize, uint8_t *yPos, uint8_t *listCount, uint8_t *offset) {
    if (listSize <= 2 ) {
        if (listSize == 2 ) {
            *yPos = *yPos + 2;
        } else if (listSize == 1 ) {
            *yPos = *yPos + 8;
        }
        *listCount = listSize + 1;
    } else {
		*offset = Clamp(listSelected - 2, 0, listSize - 3);
    }
}

/*
void popupShowList(uint8_t listSelected, uint8_t listSize, uint8_t x, uint8_t y, uint8_t w, size_t num_cols, const char subList[][num_cols]) {
	uint8_t yPos = y;
    uint8_t listCount = 3;
	uint8_t offset = 0;
	popupListCalc(listSelected, listSize, &yPos, &listCount, &offset);
    for (uint8_t i = 0; i < listCount; i++) {
		bool isFill = listSelected == i + offset ? true : false;
		UI_printf(&font_10, TEXT_ALIGN_CENTER, x, w, yPos, !isFill, isFill,
			"%s", subList[i + offset]);
        yPos += 11;
    }
}
*/

void popupShowList(uint8_t listSelected, uint8_t listSize, uint8_t x, uint8_t y, uint8_t w, const char* const subList[]) {
	uint8_t yPos = y;
    uint8_t listCount = 3;
	uint8_t offset = 0;
	popupListCalc(listSelected, listSize, &yPos, &listCount, &offset);
    for (uint8_t i = 0; i < listCount; i++) {
		bool isFill = listSelected == i + offset ? true : false;
		UI_printf(&font_10, TEXT_ALIGN_CENTER, x, w, yPos, !isFill, isFill,
			"%s", subList[i + offset]);
        yPos += 11;
    }
}
