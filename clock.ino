/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* You should have received a copy of the GNU General Public License */
/* along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#include <FastLED.h>
#include <RTClib.h>

#define LED_PIN     5
#define NUM_LEDS    16
//#define BRIGHTNESS  8
#define BRIGHTNESS  16 // this is good for the wall clock size
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

#define UPDATES_PER_SECOND 100

// clock chip DS3231
// D -> A4
// C -> A5
//
// leds
// DIN -> D5


void printTime();

RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

CRGB::HTMLColorCode digits[8][8];
uint8_t intensities[3];
int radix;

void random_radix() {
  radix = random(0,8);
}

//CRGB::HTMLColorCode orange = CRGB::OrangeRed; // this one is good for the pre-soldered panels
CRGB::HTMLColorCode orange = CRGB::DarkOrange; // this one is good for the individual leds
void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(0));
  intensities[0] = 7;
  intensities[1] = 32;
  intensities[2] = 100;

  // brightness color 1
  digits[0][0] = CRGB::Red;
  digits[0][1] = CRGB::Red;
  digits[0][2] = CRGB::Red;

  // brightness encoding color 2
  digits[1][0] = CRGB::Green;
  digits[1][1] = CRGB::Green;
  digits[1][2] = CRGB::Green;

  // brightness encoding color 3
  digits[2][0] = CRGB::Blue;
  digits[2][1] = CRGB::Blue;
  digits[2][2] = CRGB::Blue;

  // base 3
  digits[3][0] = CRGB::Red;
  digits[3][1] = CRGB::Green;
  digits[3][2] = CRGB::Blue;

  // base 4
  digits[4][0] = CRGB::Red;
  digits[4][1] = orange;
  digits[4][2] = CRGB::Yellow;
  digits[4][3] = CRGB::Green;

  // base 5
  digits[5][0] = CRGB::Red;
  digits[5][1] = orange;
  digits[5][2] = CRGB::Yellow;
  digits[5][3] = CRGB::Green;
  digits[5][4] = CRGB::Blue;

  // base 6
  digits[6][0] = CRGB::Red;
  digits[6][1] = orange;
  digits[6][2] = CRGB::Yellow;
  digits[6][3] = CRGB::Green;
  digits[6][4] = CRGB::Blue;
  digits[6][5] = CRGB::Indigo;

  // base 7
  digits[7][0] = CRGB::Red;
  digits[7][1] = orange;
  digits[7][2] = CRGB::Yellow;
  digits[7][3] = CRGB::Green;
  digits[7][4] = CRGB::Blue;
  digits[7][5] = CRGB::Indigo;
  digits[7][6] = CRGB::Violet;

  delay( 1000 ); // power-up safety delay
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  //rtc.adjust(DateTime(2019, 8, 20, 2, 53, 0));
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    // following line sets the RTC to the date & time this sketch was compiled
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    //rtc.adjust(DateTime(2019, 8, 3, 14, 54, 0));
  }

  random_radix();
}

void intensity_encode(int led, int value, int radix) {
  FastLED.setBrightness(110);
  leds[led] = digits[radix][value];
  leds[led].fadeLightBy(255-intensities[value]);
}
void set_row_forward(int start, int time, int radix) {
  if ( radix < 3) {
    intensity_encode(start+3, (time)%3, radix);
    intensity_encode(start+2, (time/3)%3, radix);
    intensity_encode(start+1, (time/(3*3))%3, radix);
    intensity_encode(start+0, (time/(3*3*3))%3, radix);
  } else {
    leds[start+3] = digits[radix][(time)%radix];
    leds[start+2] = digits[radix][(time/radix)%radix];
    leds[start+1] = digits[radix][(time/(radix*radix))%radix];
    leds[start+0] = digits[radix][(time/(radix*radix*radix))%radix];
  }
}

void set_row_backward(int start, int time, int radix) {
  if ( radix < 3) {
    intensity_encode(start+0, (time)%3, radix);
    intensity_encode(start+1, (time/3)%3, radix);
    intensity_encode(start+2, (time/(3*3))%3, radix);
    intensity_encode(start+3, (time/(3*3*3))%3, radix);
  } else {
    leds[start+0] = digits[radix][(time)%radix];
    leds[start+1] = digits[radix][(time/radix)%radix];
    leds[start+2] = digits[radix][(time/(radix*radix))%radix];
    leds[start+3] = digits[radix][(time/(radix*radix*radix))%radix];
  }
}

void loop()
{
  //ChangePalettePeriodically();
  static uint8_t startIndex = 0;
  startIndex = startIndex + 1; /* motion speed */
  printTime();
  //FillLEDsFromPaletteColors( startIndex);
  //int radix = 7;
  if (random(0,65535) == 1) {
    random_radix();
  }

  DateTime now = rtc.now();
  if ((now.dayOfTheWeek() == 5) &&
      (now.hour() == 15) &&
      ((now.minute() == 58) || (now.minute() == 59))) {
    radix = 3;
    if ((now.second() % 2) == 0) {
      FastLED.setBrightness( 110 );
    } else {
      FastLED.setBrightness(  BRIGHTNESS );
    }
  } else {
    FastLED.setBrightness(  BRIGHTNESS );
  }

  set_row_backward(0, now.second(), radix);
  set_row_forward(4, now.minute(), radix);
  set_row_backward(8, now.hour(), radix);
  set_row_forward(12, now.day(), radix);
  FastLED.show();

  delay(100);
}

void printTime()
{
  DateTime now = rtc.now();

  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" (");
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
}
