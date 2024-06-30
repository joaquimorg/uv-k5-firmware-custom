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
#include <math.h>
#include "ui.h"
#include "printf.h"
#include "driver/st7565.h"

#ifdef ENABLE_REMOTE_CONTROL
#include "app/uart.h"
#endif

uint8_t UI_nextX = 0;

void UI_displayClear(void) {
	for (int i = 1; i < FRAME_LINES; i++) {
        memset(gFrameBuffer[i], 0, sizeof(gFrameBuffer[i]));
    }
}

void UI_displayUpdate(void) {
	ST7565_BlitFullScreen(false);
}

void UI_statusUpdate(void) {
	ST7565_BlitFullScreen(true);
}

/**************************************************************************/

void setPixel(int x, int y, bool isBlack) {
	if (x < LCD_WIDTH && y < LCD_HEIGHT) {
		const uint8_t pixel = (uint8_t)(1 << (y % 8));
		const uint8_t row = (uint8_t)(y / 8);
    	if (isBlack) {
        	gFrameBuffer[row][x] |= pixel;
    	} else {
        	gFrameBuffer[row][x] &= (uint8_t)~pixel;
    	}
	}
}

void UI_drawFastVLine(uint8_t x, uint8_t y, uint8_t h, bool isBlack) {
  	for (uint8_t i = y; i < y + h; i++) {
        setPixel(x, i, isBlack);
    }
}

void UI_drawFastHLine(uint8_t x, uint8_t y, uint8_t w, bool isBlack) {
  	for (uint8_t i = x; i < x + w; i++) {
		setPixel(i, y, isBlack);
	}
}

void UI_fillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool isBlack) {
	for (uint8_t i = y; i < y + h; i++) {
    	UI_drawFastHLine(x, i, w, isBlack);
  	}
}


static void sort(uint8_t *a, uint8_t *b) {
	if(*a > *b) {
		*a ^= *b;
		*b ^= *a;
		*a ^= *b;
	}
}


void UI_drawDottedLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, bool black, uint8_t dotSpacing) {
    if (x2 == x1) {
        sort(&y1, &y2);
        for (uint8_t i = y1; i <= y2; i += dotSpacing) {
            setPixel(x1, i, black);
        }
    } else {
        const int multipl = 1000;
        int a = (y2 - y1) * multipl / (x2 - x1);
        int b = y1 - a * x1 / multipl;

        sort(&x1, &x2);
        for (int i = x1; i <= x2; i += dotSpacing) {
            setPixel(i, i * a / multipl + b, black);
        }
    }
}


void UI_drawCircleHelper(uint8_t x0, uint8_t y0, uint8_t r, uint8_t cornername, bool isBlack) {
  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;

  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
    if (cornername & 0x4) {
      setPixel(x0 + x, y0 + y, isBlack);
      setPixel(x0 + y, y0 + x, isBlack);
    }
    if (cornername & 0x2) {
      setPixel(x0 + x, y0 - y, isBlack);
      setPixel(x0 + y, y0 - x, isBlack);
    }
    if (cornername & 0x8) {
      setPixel(x0 - y, y0 + x, isBlack);
      setPixel(x0 - x, y0 + y, isBlack);
    }
    if (cornername & 0x1) {
      setPixel(x0 - y, y0 - x, isBlack);
      setPixel(x0 - x, y0 - y, isBlack);
    }
  }
}

void UI_drawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool isBlack) {
  UI_drawFastHLine(x, y, w, isBlack);
  UI_drawFastHLine(x, (uint8_t)(y + h - 1), w, isBlack);
  UI_drawFastVLine(x, y, h, isBlack);
  UI_drawFastVLine((uint8_t)(x + w - 1), y, h, isBlack);
}

void UI_drawRoundRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t r, bool isBlack) {
	uint8_t max_radius = ((w < h) ? w : h) / 2; // 1/2 minor axis
	if (r > max_radius)
		r = max_radius;
	UI_drawFastHLine(x + r, y, (uint8_t)(w - 2 * r), isBlack);         // Top
	UI_drawFastHLine(x + r, (uint8_t)(y + h - 1), (uint8_t)(w - 2 * r), isBlack); // Bottom
	UI_drawFastVLine(x, y + r, (uint8_t)(h - 2 * r), isBlack);         // Left
	UI_drawFastVLine((uint8_t)(x + w - 1), y + r, (uint8_t)(h - 2 * r), isBlack); // Right
	// draw four corners
	UI_drawCircleHelper(x + r, y + r, r, 1, isBlack);
	UI_drawCircleHelper((uint8_t)(x + w - r - 1), y + r, r, 2, isBlack);
	UI_drawCircleHelper((uint8_t)(x + w - r - 1), (uint8_t)(y + h - r - 1), r, 4, isBlack);
	UI_drawCircleHelper(x + r, (uint8_t)(y + h - r - 1), r, 8, isBlack);
}


void UI_fillRectWithChessboard(uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool isBlack) {
	// Fill the rectangular area with the chessboard pattern
    for (uint8_t i = y; i < y + h; i++) {
        for (uint8_t j = x; j < x + w; j++) {
            if ((i + j) % 2 == 0) {  // Chessboard pattern
                setPixel(j, i, isBlack);
            } else {
                setPixel(j, i, !isBlack);
            }
        }
    }
}

// Draw a character
/**************************************************************************/
uint8_t drawChar(uint8_t x, uint8_t y, unsigned char c, bool isBlack, const GFXfont * font) {

    // Character is assumed previously filtered by write() to eliminate
    // newlines, returns, non-printable characters, etc.  Calling
    // drawChar() directly with 'bad' characters of font may cause mayhem!

	uint8_t first = (uint8_t)font->first,
		last = (uint8_t)font->last;
	if ((c >= first) && (c <= last)) { // Char present in this font?

		c -= first;
		GFXglyph *glyph = font->glyph + c;

		uint8_t w = glyph->width,
				h = glyph->height;

		if ((w > 0) && (h > 0)) { // Is there an associated bitmap?
			uint8_t *bitmap = font->bitmap;
			uint16_t bo = glyph->bitmapOffset;
			uint8_t w = glyph->width, h = glyph->height;
			int8_t xo = glyph->xOffset,
				yo = glyph->yOffset;
			uint8_t xx, yy, bits = 0, bit = 0;

			for (yy = 0; yy < h; yy++) {
				for (xx = 0; xx < w; xx++) {
					if (!(bit++ & 7)) {
						bits = bitmap[bo++];
					}
					if (bits & 0x80) {
						setPixel(x + xo + xx, y + yo + yy, isBlack);
					}
					bits <<= 1;
				}
			}
		}
		return (uint8_t)glyph->xAdvance;
	}
	return 0;
}

void getStringFontSize(const char *str, uint8_t * size, const GFXfont * font) {
	*size = 0;
	char c;
	GFXglyph *glyph;
	while (*str != '\0') {
		c = *str;
		c -= (uint8_t)font->first;
    	glyph = font->glyph + c;
		*size += (uint8_t)glyph->xAdvance;
		str++;
    }
}

void charBounds(unsigned char c, int16_t *x, int16_t *y,
                              int16_t *minx, int16_t *miny, int16_t *maxx,
                              int16_t *maxy, const GFXfont * font) {

	uint8_t first = (uint8_t)font->first,
		last = (uint8_t)font->last;
	if ((c >= first) && (c <= last)) { // Char present in this font?
		GFXglyph* glyph = font->glyph + (c - first);
		uint8_t gw = (uint8_t)glyph->width,
			gh = (uint8_t)glyph->height,
			xa = (uint8_t)glyph->xAdvance;
		int8_t xo = glyph->xOffset,
			yo = glyph->yOffset;
		int16_t tsx = 1, tsy = 1,
                x1 = (int16_t)(*x + xo * tsx), y1 = (int16_t)(*y + yo * tsy), x2 = (int16_t)(x1 + gw * tsx - 1),
                y2 = (int16_t)(y1 + gh * tsy - 1);
		if (x1 < *minx)
			*minx = x1;
		if (y1 < *miny)
			*miny = y1;
		if (x2 > *maxx)
			*maxx = x2;
		if (y2 > *maxy)
			*maxy = y2;
		*x += (int16_t)(xa * tsx);
	}
}

void getTextBounds(const char *str, int16_t x, int16_t y,
                                 int16_t *x1, int16_t *y1, uint16_t *w,
                                 uint16_t *h, const GFXfont * font) {

	uint8_t c; // Current character
	int16_t minx = 0x7FFF, miny = 0x7FFF, maxx = -1, maxy = -1; // Bound rect
	// Bound rect is intentionally initialized inverted, so 1st char sets it

	*x1 = x; // Initial position is value passed in
	*y1 = y;
	*w = *h = 0; // Initial size is zero

	while ((c = *str++)) {
		// charBounds() modifies x/y to advance for each character,
		// and min/max x/y are updated to incrementally build bounding rect.
		charBounds(c, &x, &y, &minx, &miny, &maxx, &maxy, font);
	}

	if (maxx >= minx) {     // If legit string bounds were found...
		*x1 = minx - 1;           // Update x1 to least X coord,
		*w = (uint16_t)(maxx - minx + 3); // And w to bound rect width
	}
	if (maxy >= miny) { // Same for height
		*y1 = miny - 1;
		*h = (uint16_t)(maxy - miny + 3);
	}
}


// Function to draw a string in the buffer with specified x-range and pixel color
void UI_drawStringInt(const GFXfont * font, TEXT_Align_t tAlign, uint8_t xstart, uint8_t xend, uint8_t y, const char *str, bool isBlack, bool isFill, bool isBox) {

	uint8_t startX = xstart;
	uint8_t stringWidth = 0;
	int16_t x1;
	int16_t y1;
	uint16_t w;
	uint16_t h;

	getStringFontSize(str, &stringWidth, font);

	if ( xend > xstart ) {
		if ( tAlign == TEXT_ALIGN_CENTER ) {
			if (stringWidth < xend) {
				startX = (uint8_t)(((xstart + xend) / 2) - (stringWidth / 2));
			}
		} else if ( tAlign == TEXT_ALIGN_RIGHT ) {
			startX = xend - stringWidth;
		}
	}

	if ( isFill ) {
		getTextBounds(str, startX, y, &x1, &y1, &w, &h, font);
		if ( x1 < 0 ) x1 = 0;
		if ( y1 < 0 ) y1 = 0;
		if ( xend > xstart ) {
			UI_fillRect(xstart, (uint8_t)y1, xend - xstart, (uint8_t)h, !isBlack);
		} else {
			UI_fillRect((uint8_t)x1, (uint8_t)y1, (uint8_t)w, (uint8_t)h, !isBlack);
		}
		UI_nextX = (uint8_t)(x1 + w);
	} else if ( isBox ) {
		getTextBounds(str, startX, y, &x1, &y1, &w, &h, font);
		if ( x1 < 0 ) x1 = 0;
		if ( y1 < 0 ) y1 = 0;
		if ( xend > xstart ) {
			UI_drawRect(xstart - 1, (uint8_t)(y1 - 1), (uint8_t)((xend - xstart) + 2), (uint8_t)(h + 2), isBlack);
		} else {
			UI_drawRect((uint8_t)(x1 - 1), (uint8_t)(y1 - 1), (uint8_t)(w + 2), (uint8_t)(h + 2), isBlack);
		}
		UI_nextX = (uint8_t)(x1 + w + 1);
	} else {
		UI_nextX = startX + stringWidth;
	}

    while (*str != '\0') {
		startX += drawChar(startX, y, *str, isBlack, font);
        str++;
    }

}


void UI_drawString(const GFXfont * font, TEXT_Align_t tAlign, uint8_t xstart, uint8_t xend, uint8_t y, const char *str, bool isBlack, bool isFill) {
	UI_drawStringInt(font, tAlign, xstart, xend, y, str, isBlack, isFill, false);
}

void UI_drawStringBox(const GFXfont * font, TEXT_Align_t tAlign, uint8_t xstart, uint8_t xend, uint8_t y, const char *str, bool isBlack) {
	UI_drawStringInt(font, tAlign, xstart, xend, y, str, isBlack, false, true);
}

void UI_printf(const GFXfont * font, TEXT_Align_t tAlign, uint8_t xstart, uint8_t xend, uint8_t y, bool isBlack, bool isFill, const char *str, ...) {
	char text[52];

	va_list va;
	va_start(va, str);
		vsnprintf(text, sizeof(text), str, va);
	va_end(va);

	UI_drawStringInt(font, tAlign, xstart, xend, y, text, isBlack, isFill, false);
}

extern const uint8_t boot_logo[];

void UI_showLogo(void) {
	uint16_t logo = 0;
    for (uint8_t i = 0; i < 8; i++) {
        for (uint8_t j = 0; j < 64; j++) {
            gFrameBuffer[i][j] = boot_logo[logo++];
        }
    }
}