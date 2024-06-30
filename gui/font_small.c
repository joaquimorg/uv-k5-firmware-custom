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


#ifndef FONT_SMALL
#define FONT_SMALL

#include "font.h"

//https://www.dafont.com/wendy.font?sort=date&psize=xs

// 0x22 '"' TX
// 0x23 '#' MSG
// 0x24 '$' RX
// 0x26 '&' RSSI Empty
// 0x3F '?' RSSI Sep
// 0x40 '@' RSSI Box
// 0x5B '[' F key
// 0x5D ']' Lock
// 0x5C '\' Charging
// 0x5E '^' Mini Batt
// 0x27 ''' Antenna
// 0x60 '`' Radio
// 0x7B '{' Save

const uint8_t font_smallBitmaps[] PROGMEM = {
  0x00, 0xE8, 0xEA, 0x95, 0x12, 0x54, 0xA0, 0xFF, 0x8E, 0xAC, 0x9F, 0xE0, 
  0xEB, 0x57, 0x15, 0x5A, 0xA0, 0x92, 0x49, 0x1C, 0x00, 0xAB, 0x88, 0x42, 
  0x00, 0x6A, 0x40, 0x95, 0x80, 0x6D, 0xFD, 0xF1, 0xC1, 0x00, 0x5D, 0x00, 
  0xC0, 0xE0, 0x80, 0x25, 0x48, 0xF6, 0xDE, 0x59, 0x2E, 0x00, 0xE7, 0xCE, 
  0xE7, 0x9E, 0x96, 0xF2, 0xF3, 0x9E, 0xF3, 0xDE, 0xE4, 0x92, 0xF7, 0xDE, 
  0xF7, 0x9E, 0xA0, 0xB0, 0x2A, 0x22, 0xCC, 0x88, 0xA8, 0x00, 0x0A, 0x00, 
  0xFF, 0x80, 0xF7, 0xDA, 0xF7, 0x5E, 0xF2, 0x4E, 0xD6, 0xDC, 0xF3, 0xCE, 
  0xF3, 0x48, 0xF2, 0xDE, 0xB7, 0xDA, 0xF8, 0x24, 0x9E, 0xB7, 0x5A, 0x92, 
  0x4E, 0xDD, 0x6B, 0x58, 0x80, 0xF6, 0xDA, 0xF6, 0xDE, 0xF7, 0xC8, 0xF7, 
  0x92, 0xF7, 0x5A, 0xF3, 0x9E, 0xE9, 0x24, 0xB6, 0xDE, 0xB6, 0xD4, 0x8C, 
  0x6B, 0x5D, 0x80, 0xB5, 0x5A, 0xB7, 0x9E, 0xE5, 0x4E, 0x1E, 0x68, 0x38, 
  0x00, 0x57, 0xE2, 0xE2, 0x00, 0xE0, 0xBF, 0xE5, 0x6F, 0xFF, 0xF0, 0xE0, 
  0x93, 0xFE, 0x00, 0xF7, 0xDA, 0xF7, 0x5E, 0xF2, 0x4E, 0xD6, 0xDC, 0xF3, 
  0xCE, 0xF3, 0x48, 0xF2, 0xDE, 0xB7, 0xDA, 0xF8, 0x24, 0x9E, 0xB7, 0x5A, 
  0x92, 0x4E, 0xDD, 0x6B, 0x58, 0x80, 0xF6, 0xDA, 0xF6, 0xDE, 0xF7, 0xC8, 
  0xF7, 0x92, 0xF7, 0x5A, 0xF3, 0x9E, 0xE9, 0x24, 0xB6, 0xDE, 0xB6, 0xD4, 
  0x8C, 0x6B, 0x5D, 0x80, 0xB5, 0x5A, 0xB7, 0x9E, 0xE5, 0x4E, 0x55, 0xFE, 
  0x0F, 0xF5, 0x40, 0xFE
};

const GFXglyph font_smallGlyphs[] PROGMEM = {
  {     0,   1,   1,   2,    0,    0 },   // 0x20 ' '
  {     1,   1,   5,   2,    0,   -4 },   // 0x21 '!'
  {     2,   7,   5,   8,    0,   -4 },   // 0x22 '"'
  {     7,   7,   5,   8,    0,   -4 },   // 0x23 '#'
  {    12,   7,   5,   8,    0,   -4 },   // 0x24 '$'
  {    17,   4,   4,   5,    0,   -3 },   // 0x25 '%'
  {    19,   3,   3,   4,    0,   -4 },   // 0x26 '&'
  {    21,   5,   5,   6,    0,   -4 },   // 0x27 '''
  {    25,   2,   5,   3,    0,   -4 },   // 0x28 '('
  {    27,   2,   5,   3,    0,   -4 },   // 0x29 ')'
  {    29,   7,   5,   8,    0,   -4 },   // 0x2A '*'
  {    34,   3,   3,   4,    0,   -3 },   // 0x2B '+'
  {    36,   1,   2,   2,    0,    0 },   // 0x2C ','
  {    37,   3,   1,   4,    0,   -2 },   // 0x2D '-'
  {    38,   1,   1,   2,    0,    0 },   // 0x2E '.'
  {    39,   3,   5,   6,    0,   -4 },   // 0x2F '/'
  {    41,   3,   5,   4,    0,   -4 },   // 0x30 '0'
  {    43,   3,   5,   4,    0,   -4 },   // 0x31 '1'
  {    46,   3,   5,   4,    0,   -4 },   // 0x32 '2'
  {    48,   3,   5,   4,    0,   -4 },   // 0x33 '3'
  {    50,   3,   5,   4,    0,   -4 },   // 0x34 '4'
  {    52,   3,   5,   4,    0,   -4 },   // 0x35 '5'
  {    54,   3,   5,   4,    0,   -4 },   // 0x36 '6'
  {    56,   3,   5,   4,    0,   -4 },   // 0x37 '7'
  {    58,   3,   5,   4,    0,   -4 },   // 0x38 '8'
  {    60,   3,   5,   4,    0,   -4 },   // 0x39 '9'
  {    62,   1,   3,   2,    0,   -2 },   // 0x3A ':'
  {    63,   1,   4,   2,    0,   -2 },   // 0x3B ';'
  {    64,   3,   5,   4,    0,   -4 },   // 0x3C '<'
  {    66,   2,   3,   3,    0,   -3 },   // 0x3D '='
  {    67,   3,   5,   4,    0,   -4 },   // 0x3E '>'
  {    69,   3,   5,   4,    0,   -4 },   // 0x3F '?'
  {    72,   3,   3,   4,    0,   -4 },   // 0x40 '@'
  {    74,   3,   5,   4,    0,   -4 },   // 0x41 'A'
  {    76,   3,   5,   4,    0,   -4 },   // 0x42 'B'
  {    78,   3,   5,   4,    0,   -4 },   // 0x43 'C'
  {    80,   3,   5,   4,    0,   -4 },   // 0x44 'D'
  {    82,   3,   5,   4,    0,   -4 },   // 0x45 'E'
  {    84,   3,   5,   4,    0,   -4 },   // 0x46 'F'
  {    86,   3,   5,   4,    0,   -4 },   // 0x47 'G'
  {    88,   3,   5,   4,    0,   -4 },   // 0x48 'H'
  {    90,   1,   5,   2,    0,   -4 },   // 0x49 'I'
  {    91,   3,   5,   4,    0,   -4 },   // 0x4A 'J'
  {    93,   3,   5,   4,    0,   -4 },   // 0x4B 'K'
  {    95,   3,   5,   4,    0,   -4 },   // 0x4C 'L'
  {    97,   5,   5,   6,    0,   -4 },   // 0x4D 'M'
  {   101,   3,   5,   4,    0,   -4 },   // 0x4E 'N'
  {   103,   3,   5,   4,    0,   -4 },   // 0x4F 'O'
  {   105,   3,   5,   4,    0,   -4 },   // 0x50 'P'
  {   107,   3,   5,   4,    0,   -4 },   // 0x51 'Q'
  {   109,   3,   5,   4,    0,   -4 },   // 0x52 'R'
  {   111,   3,   5,   4,    0,   -4 },   // 0x53 'S'
  {   113,   3,   5,   4,    0,   -4 },   // 0x54 'T'
  {   115,   3,   5,   4,    0,   -4 },   // 0x55 'U'
  {   117,   3,   5,   4,    0,   -4 },   // 0x56 'V'
  {   119,   5,   5,   6,    0,   -4 },   // 0x57 'W'
  {   123,   3,   5,   4,    0,   -4 },   // 0x58 'X'
  {   125,   3,   5,   4,    0,   -4 },   // 0x59 'Y'
  {   127,   3,   5,   4,    0,   -4 },   // 0x5A 'Z'
  {   129,   3,   7,   4,    0,   -5 },   // 0x5B '['
  {   133,   5,   5,   6,    0,   -4 },   // 0x5C '\'
  {   137,   8,   3,   9,    0,   -3 },   // 0x5D ']'
  {   140,   4,   5,   5,    0,   -4 },   // 0x5E '^'
  {   143,   3,   1,   4,    0,    0 },   // 0x5F '_'
  {   144,   3,   5,   4,    0,   -4 },   // 0x60 '`'
  {   147,   3,   5,   4,    0,   -4 },   // 0x61 'a'
  {   149,   3,   5,   4,    0,   -4 },   // 0x62 'b'
  {   151,   3,   5,   4,    0,   -4 },   // 0x63 'c'
  {   153,   3,   5,   4,    0,   -4 },   // 0x64 'd'
  {   155,   3,   5,   4,    0,   -4 },   // 0x65 'e'
  {   157,   3,   5,   4,    0,   -4 },   // 0x66 'f'
  {   159,   3,   5,   4,    0,   -4 },   // 0x67 'g'
  {   161,   3,   5,   4,    0,   -4 },   // 0x68 'h'
  {   163,   1,   5,   2,    0,   -4 },   // 0x69 'i'
  {   164,   3,   5,   4,    0,   -4 },   // 0x6A 'j'
  {   166,   3,   5,   4,    0,   -4 },   // 0x6B 'k'
  {   168,   3,   5,   4,    0,   -4 },   // 0x6C 'l'
  {   170,   5,   5,   6,    0,   -4 },   // 0x6D 'm'
  {   174,   3,   5,   4,    0,   -4 },   // 0x6E 'n'
  {   176,   3,   5,   4,    0,   -4 },   // 0x6F 'o'
  {   178,   3,   5,   4,    0,   -4 },   // 0x70 'p'
  {   180,   3,   5,   4,    0,   -4 },   // 0x71 'q'
  {   182,   3,   5,   4,    0,   -4 },   // 0x72 'r'
  {   184,   3,   5,   4,    0,   -4 },   // 0x73 's'
  {   186,   3,   5,   4,    0,   -4 },   // 0x74 't'
  {   188,   3,   5,   4,    0,   -4 },   // 0x75 'u'
  {   190,   3,   5,   4,    0,   -4 },   // 0x76 'v'
  {   192,   5,   5,   6,    0,   -4 },   // 0x77 'w'
  {   196,   3,   5,   4,    0,   -4 },   // 0x78 'x'
  {   198,   3,   5,   4,    0,   -4 },   // 0x79 'y'
  {   200,   3,   5,   4,    0,   -4 },   // 0x7A 'z'
  {   202,   7,   5,   8,    0,   -4 },   // 0x7B '{'
  {   207,   1,   7,   2,    0,   -5 }    // 0x7C '|'
};

const GFXfont font_small PROGMEM = {
  (uint8_t  *)font_smallBitmaps,       
  (GFXglyph *)font_smallGlyphs, 0x20, 0x7C,        9 };

// Approx. 859 bytes

#endif