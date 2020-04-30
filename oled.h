/*
  ==============================================================================
  ==============================================================================
  oled.h

  oled control via i2c
  ==============================================================================
  ==============================================================================
*/

#ifndef _OLED_H_
#define _OLED_H_

enum oled_font {
	OLED_FONT_INVALID = -1,
	OLED_FONT_SMALL,
	OLED_FONT_MEDIUM,
	OLED_FONT_LARGE
};

__attribute__ ((unused)) static const char *
oled_font_names(enum oled_font font)
{
	switch (font) {
	case OLED_FONT_INVALID:
		return "OLED_FONT_INVALID"; break;
	case OLED_FONT_SMALL:
		return "OLED_FONT_SMALL"; break;
	case OLED_FONT_MEDIUM:
		return "OLED_FONT_MEDIUM"; break;
	case OLED_FONT_LARGE:
		return "OLED_FONT_LARGE"; break;
	default: break;
	}

	return "BAD STATE";
}

int oled_initialize(int handle, bool flip, bool invert);

int oled_finalize(int handle);

int oled_contrast(int handle, char contrast);

/*
  Coordinates are as expected, x is 0...127 and y is 0...63.
*/

int oled_pixel(int handle, unsigned x, unsigned y, bool on);

/*
  This fills by "characters" which are 8x8 blocks.  It is possible to
  fill by pixels, but this is much slower!

  So, (x0, y0) is the top left corner and (x1, y1) is the bottom right.
*/

int oled_fill(int handle, bool on,
	      unsigned x0, unsigned y0, unsigned x1, unsigned y1);

int oled_clear(int handle);

/*
  Coordinates are NOT as expected.

  For OLED_FONT_SMALL (6x8)...
          x can be 0...20 (128 / 6 = 21.3, so 21) and
	  y can be 0...7 (64 / 8 = 8)
  For OLED_FONT_MEDIUM (8x8)...
          x can be 0...16 (128 / 8 = 16) and
	  y can be 0...7 (64 / 8 = 8)
  For OLED_FONT_LARGE (16x24)...
          x can be 0...7 (128 / 16 = 8) and
	  y can be 0...5 (64 / 24 = 2.6, so 2)
	  (but add 3 to avoid overlap)
*/

int oled_print(int handle,
	       unsigned x, unsigned y, enum oled_font font, char *string);

#endif	/* _OLED_H_ */
