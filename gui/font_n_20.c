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

#ifndef FONT_N_20
#define FONT_N_20

#include "font.h"

// https://www.dafont.com/pt/16x8pxl.font?fpp=50&l[]=10&l[]=1

const uint8_t font_n_20Bitmaps[] PROGMEM = {
  0x18, 0xDB, 0xDB, 0xFF, 0x7E, 0x3C, 0x3C, 0x7E, 0xFF, 0xDB, 0xDB, 0x18, 
  0x18, 0x18, 0x18, 0xFF, 0xFF, 0x18, 0x18, 0x18, 0xF6, 0xFF, 0xFF, 0xF0, 
  0x03, 0x03, 0x07, 0x06, 0x0E, 0x0C, 0x1C, 0x18, 0x18, 0x38, 0x30, 0x70, 
  0x60, 0xE0, 0xC0, 0xC0, 0x7E, 0xFF, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 
  0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xFF, 0x7E, 0x30, 0xCF, 0x3C, 0x30, 
  0xC3, 0x0C, 0x30, 0xC3, 0x0C, 0x30, 0xCF, 0xFF, 0x7E, 0xFF, 0xC3, 0xC3, 
  0x03, 0x03, 0x07, 0x0E, 0x1C, 0x38, 0x70, 0xE0, 0xC0, 0xC0, 0xFF, 0xFF, 
  0x7E, 0xFF, 0xC3, 0xC3, 0x03, 0x03, 0x07, 0x1E, 0x1E, 0x07, 0x03, 0x03, 
  0xC3, 0xC3, 0xFF, 0x7E, 0x0E, 0x0E, 0x1E, 0x3E, 0x36, 0x76, 0x66, 0xE6, 
  0xC6, 0xC6, 0xFF, 0xFF, 0x06, 0x06, 0x06, 0x06, 0xFF, 0xFF, 0xC0, 0xC0, 
  0xC0, 0xC0, 0xC0, 0xFE, 0xFF, 0x03, 0x03, 0x03, 0xC3, 0xC3, 0xFF, 0x7E, 
  0x7E, 0xFF, 0xC3, 0xC3, 0xC0, 0xC0, 0xC0, 0xFE, 0xFF, 0xC3, 0xC3, 0xC3, 
  0xC3, 0xC3, 0xFF, 0x7E, 0xFF, 0xFF, 0xC3, 0xC3, 0x07, 0x06, 0x0E, 0x0C, 
  0x1C, 0x18, 0x38, 0x30, 0x30, 0x30, 0x30, 0x30, 0x7E, 0xFF, 0xC3, 0xC3, 
  0xC3, 0xC3, 0xE7, 0x7E, 0x7E, 0xE7, 0xC3, 0xC3, 0xC3, 0xC3, 0xFF, 0x7E, 
  0x7E, 0xFF, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xFF, 0x7F, 0x03, 0x03, 0x03, 
  0xC3, 0xC3, 0xFF, 0x7E, 0xF0, 0x00, 0xF0
};

const GFXglyph font_n_20Glyphs[] PROGMEM = {
  {     0,   8,  12,   9,    0,  -13 },   // 0x2A '*'
  {    12,   8,   8,   9,    0,  -11 },   // 0x2B '+'
  {    20,   2,   4,   9,    3,   -3 },   // 0x2C ','
  {    21,   8,   2,   9,    0,   -8 },   // 0x2D '-'
  {    23,   2,   2,   9,    3,   -1 },   // 0x2E '.'
  {    24,   8,  16,   9,    0,  -15 },   // 0x2F '/'
  {    40,   8,  16,   9,    0,  -15 },   // 0x30 '0'
  {    56,   6,  16,   9,    1,  -15 },   // 0x31 '1'
  {    68,   8,  16,   9,    0,  -15 },   // 0x32 '2'
  {    84,   8,  16,   9,    0,  -15 },   // 0x33 '3'
  {   100,   8,  16,   9,    0,  -15 },   // 0x34 '4'
  {   116,   8,  16,   9,    0,  -15 },   // 0x35 '5'
  {   132,   8,  16,   9,    0,  -15 },   // 0x36 '6'
  {   148,   8,  16,   9,    0,  -15 },   // 0x37 '7'
  {   164,   8,  16,   9,    0,  -15 },   // 0x38 '8'
  {   180,   8,  16,   9,    0,  -15 },   // 0x39 '9'
  {   196,   2,  10,   9,    3,  -12 }    // 0x3A ':'
};

const GFXfont font_n_20 PROGMEM = {
  (uint8_t  *)font_n_20Bitmaps, 
  (GFXglyph *)font_n_20Glyphs, 0x2A, 0x3A,  21 };

#endif

// Approx. 1833 bytes
