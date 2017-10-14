#include "Arduino.h"
#include "FastLED.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN 13 
#endif

#define NEOPIN  2
#define NUM_LEDS  12
#define MICPIN 0

enum {
	RING,
	FULL,
	SOUND
}		modeEnum;

CRGB		leds      [NUM_LEDS];
uint8_t		pixelIndex = 0, h = 0;
uint64_t	lastAffich = 0;
char		readBuffer[64];
char		lastRead  [64];
uint8_t		readIndex = 0;
bool		dataToRead = false;
uint8_t		mode = FULL;

void
setup()
{
	FastLED.addLeds < NEOPIXEL, NEOPIN > (leds, NUM_LEDS);
	pinMode(LED_BUILTIN, OUTPUT);
	Serial.begin(115200);
}

void
atCom()
{
	while (Serial.available() > 0) {
		readBuffer[readIndex] = Serial.read();
		if (readBuffer[readIndex] == '\n') {
			memcpy(lastRead, readBuffer, readIndex);
			lastRead[readIndex] = 0;
			dataToRead = true;
			readIndex = 0;
		} else {
			readIndex++;
		}
	}
}

void
affich()
{
	uint8_t		i;
	int16_t		micRead;

	switch (mode) {
	case RING:
		for (i = 0; i < 12; i++) {
			if (i == pixelIndex) {
				leds[i] = CHSV(h++, 128, 64);
			} else {
		leds[i] = CRGB::Black;
			}
		}
		pixelIndex++;
		if (pixelIndex == 12) {
			pixelIndex = 0;
		}
		break;
	case FULL:
		for (i = 0; i < 12; i++) {
			leds[i] = CHSV(h, 128, 64);
		}
		h++;
		break;
	case SOUND:
		micRead = analogRead(MICPIN);
		micRead = abs(micRead - 256);
		micRead = map(micRead, 0, 210, 0, NUM_LEDS);
		for (i = 0; i < 12; i++) {
			if (i < micRead) {
				leds[i] = CHSV(h, 128, 64);
			} else {
		leds[i] = CRGB::Black;
			}
		}
		h++;
		break;
	}
	FastLED.show();
}

void
loop()
{
	uint8_t		tdelay = 25;

	if ((millis() - lastAffich) > tdelay) {
		lastAffich = millis();
		affich();
	}
	atCom();
	if (dataToRead) {
		dataToRead = false;
		if (strcmp(lastRead, "RING") == 0) {
			Serial.print("RING mode ON\n");
			mode = RING;
		} else if (strcmp(lastRead, "FULL") == 0) {
			Serial.print("FULL mode ON\n");
			mode = FULL;
		} else if (strcmp(lastRead, "SOUND") == 0) {
			Serial.print("SOUND mode ON\n");
			mode = SOUND;
		}
		if (mode == SOUND) {
			tdelay = 10;
		} else {
			tdelay = 25;
		}
	}
}
