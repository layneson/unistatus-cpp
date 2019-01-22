#include <string.h>
#include <stdint.h>

#include <ws2811.h>

#include "display.hpp"

namespace display {

int pixel_map[] = {
	56, 55, 40, 39, 24, 23, 8,  7,
	57, 54, 41, 38, 25, 22, 9,  6,
	58, 53, 42, 37, 26, 21, 10, 5,
	59, 52, 43, 36, 27, 20, 11, 4,
	60, 51, 44, 35, 28, 19, 12, 3,
	61, 50, 45, 34, 29, 18, 13, 2,
	62, 49, 46, 33, 30, 17, 14, 1,
	63, 48, 47, 32, 31, 16, 15, 0 
};

ws2811_t ledstring = { 0 };

void init() {
	ledstring.freq = WS2811_TARGET_FREQ;
	ledstring.dmanum = 10;
	ledstring.channel[0].gpionum = 18;
	ledstring.channel[0].count = 64;
	ledstring.channel[0].invert = 0;
	ledstring.channel[0].brightness = 128;
	ledstring.channel[0].strip_type = WS2811_STRIP_BGR;

	ws2811_init(&ledstring);
}

void quit() {
	memset(ledstring.channel[0].leds, 0, 64 * 4);

	ws2811_render(&ledstring);
}

void update() {
	ws2811_render(&ledstring);
}

void set_pixel(int x, int y, unsigned char r, unsigned char g, unsigned char b) {
	ledstring.channel[0].leds[pixel_map[y * 8 + x]] = ((int)b << 16) | ((int)g << 8) | (int)r;
}

void set_brightness(unsigned char v) {
	ledstring.channel[0].brightness = v;
}

}
