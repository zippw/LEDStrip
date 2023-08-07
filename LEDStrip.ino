/*
 * Arduino interface for the use of WS2812 strip LEDs
 * Uses Adalight protocol and is compatible with Boblight, Prismatik etc...
 * "Magic Word" for synchronisation is 'Ada' followed by LED High, Low and Checksum
 * @author: Wifsimster <wifsimster@gmail.com> 
 * @library: FastLED v3.001
 * @date: 11/22/2015
 */
#include "FastLED.h"

// Baudrate, higher rate allows faster refresh rate and more LEDs (defined in /etc/boblight.conf)
#define serialRate 115200

// #define TOTAL_LEDS 96

#define S1_PIN 4
#define S2_PIN 5

#define S1_LEDS 30
#define S2_LEDS 18

#define TOTAL_LEDS S1_LEDS + S2_LEDS

// Adalight sends a "Magic Word" (defined in /etc/boblight.conf) before sending the pixel data
uint8_t prefix[] = { 'A', 'd', 'a' },
        hi, lo, chk, i;

// Initialise LED-array
CRGB leds[TOTAL_LEDS];

void setup() {
  // Use NEOPIXEL to keep true colors / WS2801
  FastLED.addLeds<NEOPIXEL, S1_PIN>(leds, 0, S1_LEDS);
  FastLED.addLeds<NEOPIXEL, S2_PIN>(leds, S1_LEDS - 1, S2_LEDS);

  // Initial RGB flash
  LEDS.showColor(CRGB(255, 113, 206));
  delay(200);
  LEDS.showColor(CRGB(185, 103, 255));
  delay(200);
  LEDS.showColor(CRGB(1, 205, 254));
  delay(200);
  LEDS.showColor(CRGB(5, 255, 161));
  delay(200);
  LEDS.showColor(CRGB(255, 251, 150));
  delay(200);
  LEDS.showColor(CRGB(0, 0, 0));

  Serial.begin(serialRate);
  // Send "Magic Word" string to host
  Serial.print("Ada\n");
}

void loop() {
  // Wait for first byte of Magic Word
  for (i = 0; i < sizeof prefix; ++i) {
waitLoop:
    while (!Serial.available())
      ;
    ;
    // Check next byte in Magic Word
    if (prefix[i] == Serial.read()) continue;
    // otherwise, start over
    i = 0;
    goto waitLoop;
  }

  // Hi, Lo, Checksum
  while (!Serial.available())
    ;
  ;
  hi = Serial.read();
  while (!Serial.available())
    ;
  ;
  lo = Serial.read();
  while (!Serial.available())
    ;
  ;
  chk = Serial.read();

  // If checksum does not match go back to wait
  if (chk != (hi ^ lo ^ 0x55)) {
    i = 0;
    goto waitLoop;
  }

  memset(leds, 0, TOTAL_LEDS * sizeof(struct CRGB));

  for (uint8_t i = 0; i < TOTAL_LEDS; i++) {
    byte r, g, b;
    while (!Serial.available())
      ;
    r = Serial.read();
    while (!Serial.available())
      ;
    g = Serial.read();
    while (!Serial.available())
      ;
    b = Serial.read();
    leds[i].r = r;
    leds[i].g = g;
    leds[i].b = b;
  };
  FastLED.show();
}