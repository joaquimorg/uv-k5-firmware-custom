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

#ifndef G_UI_H
#define G_UI_H

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "font.h"

extern uint8_t UI_nextX;

enum TEXT_Align_e {
	TEXT_ALIGN_LEFT = 0,
    TEXT_ALIGN_CENTER,
    TEXT_ALIGN_RIGHT,
};
typedef enum TEXT_Align_e TEXT_Align_t;

void UI_displayClear(void);

void UI_displayUpdate(void);
void UI_statusUpdate(void);

void UI_drawString(const GFXfont * font, TEXT_Align_t tAlign, uint8_t xstart, uint8_t xend, uint8_t y, const char *str, bool isBlack, bool isFill);
void UI_drawStringBox(const GFXfont * font, TEXT_Align_t tAlign, uint8_t xstart, uint8_t xend, uint8_t y, const char *str, bool isBlack);
void UI_printf(const GFXfont * font, TEXT_Align_t tAlign, uint8_t xstart, uint8_t xend, uint8_t y, bool isBlack, bool isFill, const char *str, ...);

void getStringFontSize(const char *str, uint8_t * size, const GFXfont * font);

void UI_drawFastVLine(uint8_t x, uint8_t y, uint8_t h, bool isBlack);
void UI_drawFastHLine(uint8_t x, uint8_t y, uint8_t w, bool isBlack);
void UI_drawDottedLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, bool black, uint8_t dotSpacing);
void UI_fillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool isBlack);

void UI_drawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool isBlack);
void UI_drawRoundRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t r, bool isBlack);
void UI_fillRectWithChessboard(uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool isBlack);

void UI_showLogo(void);

#endif