#include <FastLED.h>

#define ID 0x082F
#define BTN_ON 0x05
#define BTN_OFF 0x04
#define BTN_SAT_UP 0x02
#define BTN_SAT_DN 0x01
#define BTN_BRI_UP 0x06
#define BTN_BRI_DN 0x07


volatile unsigned long t_high = 0;
volatile unsigned long t_rising = 0;
volatile unsigned long t_sum = 0;
volatile unsigned int rcvd = 0;
volatile byte bit_count = 0;
volatile bool id_found = false;
CHSV leds_hsv[1];
CRGB leds[1];

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<WS2812B, 11, GRB>(leds, 1);
  leds_hsv[0] = CHSV(0, 128, 128);
  attachInterrupt(0, rising, RISING); // Pin 2
}
 
void loop() {
}
 
void rising() {
  detachInterrupt(0);
  t_sum = micros() - t_rising;
  t_rising = micros();
  if (t_sum >= 1000 && t_sum <= 1200)
  {
    if  (t_high >= 700 && t_high <= 800)
    {
      rcvd = (rcvd << 1) | 1;
      if (id_found)
      {
        bit_count++;
      }
    }
    else if (t_high >= 250 && t_high <= 350)
    {
      rcvd = rcvd << 1;
      if (id_found)
      {
        bit_count++;
      }
    }
  }
  if (!id_found && rcvd == ID)
  {
    rcvd = 0;
    bit_count = 0;
    id_found = true;
  }
  else if (id_found && bit_count == 16)
  {
    id_found = 0;
    bit_count = 0;
    if (rcvd >> 15 == 1) // 1xx = Buttons
    {
      switch ((rcvd << 5) >> 13)
      {
        case BTN_ON:
          FastLED.showColor(CHSV(leds_hsv[0].hue, leds_hsv[0].sat, leds_hsv[0].val));
          break;
        case BTN_OFF:
          FastLED.showColor(CHSV(0, 0, 0));
          break;
        case BTN_SAT_UP:
          leds_hsv[0].sat = min(leds_hsv[0].sat + 10, 255);
          FastLED.showColor(CHSV(leds_hsv[0].hue, leds_hsv[0].sat, leds_hsv[0].val));
          break;
        case BTN_SAT_DN:
          leds_hsv[0].sat = max(leds_hsv[0].sat - 10, 0);
          FastLED.showColor(CHSV(leds_hsv[0].hue, leds_hsv[0].sat, leds_hsv[0].val));
          break;
        case BTN_BRI_UP:
          leds_hsv[0].val = min(leds_hsv[0].val + 10, 255);
          FastLED.showColor(CHSV(leds_hsv[0].hue, leds_hsv[0].sat, leds_hsv[0].val));
          break;
        case BTN_BRI_DN:
          leds_hsv[0].val = max(leds_hsv[0].val - 10, 0);
          FastLED.showColor(CHSV(leds_hsv[0].hue, leds_hsv[0].sat, leds_hsv[0].val));
          break;
        default:
          break;
      }
    }
    else // 0xxxxxxx = Color
    {
      byte color = rcvd >> 8;
      color = abs(map(color, 1, 125, 0, 255) - 255) - 10;
      if (color < 0)
      {
        color = 255 - color;
      }
      leds_hsv[0].hue = color;
      FastLED.showColor(CHSV(leds_hsv[0].hue, leds_hsv[0].sat, leds_hsv[0].val));
    }
    rcvd = 0;
  }
  attachInterrupt(0, falling, FALLING);
}
 
void falling() {
  attachInterrupt(0, rising, RISING);
  t_high = micros() - t_rising;
}
