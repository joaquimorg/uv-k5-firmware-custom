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

#ifndef G_GUI_H
#define G_GUI_H

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "driver/keyboard.h"

void        GUI_drawBattery(void);

void        GUI_inputAppendKey(const KEY_Code_t key, uint8_t length, bool decimal);
uint8_t     GUI_inputGetSize();
bool        GUI_inputNotEmpty();
void        GUI_inputReset();
void        GUI_inputShowFreq(const char *title);
void        GUI_inputShow(const char *title, const char *prefix);
uint32_t    GUI_inputGetNumberClear();
uint32_t    GUI_inputGetNumber();

void        GUI_showPopup(uint8_t popupW, uint8_t popupH, uint8_t *startX, uint8_t *startY);

void        GUI_updateCursor();

int Clamp(int v, int min, int max);

void popupListCalc(uint8_t listSelected, uint8_t listSize, uint8_t *yPos, uint8_t *listCount, uint8_t *offset);
//void popupShowList(uint8_t listSelected, uint8_t listSize, uint8_t x, uint8_t y, uint8_t w, size_t num_cols, const char subList[][num_cols]);
void popupShowList(uint8_t listSelected, uint8_t listSize, uint8_t x, uint8_t y, uint8_t w, const char* const subList[]);


#endif