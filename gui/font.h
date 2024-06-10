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

#ifndef FONT_H
#define FONT_H

#include <stdint.h>


//https://tchapi.github.io/Adafruit-GFX-Font-Customiser/
//https://www.dafont.com/bitmap.php


// ./fontconvert font_n_20 superstar_memesbruh03.ttf 8 > /mnt/d/Work/UV-K5/uv-k5-firmware-custom-dev/gui/font_n_20.c

// From Adafruit-GFX-Library

/// Font data stored PER GLYPH

#define PROGMEM
typedef struct {
  uint16_t bitmapOffset; ///< Pointer into GFXfont->bitmap
  uint8_t width;         ///< Bitmap dimensions in pixels
  uint8_t height;        ///< Bitmap dimensions in pixels
  uint8_t xAdvance;      ///< Distance to advance cursor (x axis)
  int8_t xOffset;        ///< X dist from cursor pos to UL corner
  int8_t yOffset;        ///< Y dist from cursor pos to UL corner
} GFXglyph;

/// Data stored for FONT AS A WHOLE
typedef struct {
  uint8_t *bitmap;  ///< Glyph bitmaps, concatenated
  GFXglyph *glyph;  ///< Glyph array
  uint16_t first;   ///< ASCII extents (first char)
  uint16_t last;    ///< ASCII extents (last char)
  uint8_t yAdvance; ///< Newline distance (y axis)
} GFXfont;


// Structure to define a font
typedef struct font_struct {
    char minChar;           // Minimum character supported by the font
    char maxChar;           // Maximum character supported by the font
    uint8_t charWidth;      // Width of each character in pixels
    uint8_t charHeight;     // Height of each character in pixels
    uint8_t charSpace;      // Space to leave between characters in pixels
    const uint8_t fontTable[];  // Pointer to the font table
} font_t;

/* - - - - - - - - - - - - - - - - - - - - - - - - */

extern const GFXfont font_small;
extern const GFXfont font_10;
extern const GFXfont font_n_16;
extern const GFXfont font_n_20;


#endif

