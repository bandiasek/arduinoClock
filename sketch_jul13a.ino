/*-----------POUITÉ-KNIŽNICE------------*/
#include <DS3232RTC.h>
#include <RTClib.h> //
#include <Time.h> 
#include <TimeLib.h>
#include <FastLED.h> //
#include <OneWire.h> 
#include <DallasTemperature.h>

RTC_DS3231 rtc;

/*-----------DEFINÍCIA-PINOV------------*/
#define NUM_LEDS 29 // počet led pásikov v hodinách (4*7 + 1..dvojbodka v strede)
#define COLOR_ORDER BRG  // poradie farieb
#define DATA_PIN 6  // pin na prenos dat
#define DST_PIN 2  // tlacidlo na nastavovanie daylight-saving-time
#define MIN_PIN 4  // tlacidlo na nastavenie minút
#define HUR_PIN 5  // tlačidlo na nastavenie hodín
#define BRI_PIN 3  // photorezistor na snimanie svetla
#define ONE_WIRE_BUS 9 // Data kabel na spojenie so snímačom teploty

/*-----------DEFINÍCIA-ZNAKOV------------*/
byte digits[10][7] = {{0,1,1,1,1,1,1},  // číslo 0
                     {0,1,0,0,0,0,1},   // číslo 1
                     {1,1,1,0,1,1,0},   // číslo 2
                     {1,1,1,0,0,1,1},   // číslo 3
                     {1,1,0,1,0,0,1},   // číslo 4
                     {1,0,1,1,0,1,1},   // číslo 5
                     {1,0,1,1,1,1,1},   // číslo 6
                     {0,1,1,0,0,0,1},   // číslo 7
                     {1,1,1,1,1,1,1},   // číslo 8
                     {1,1,1,1,0,1,1},   // číslo 9
                     {1,1,1,1,1,1,1},   // znak * (stupne)
                     {1,1,1,1,1,1,1},}; // znak C

/*-----KOMUNIKÁCIA-SO-SNÍMAČOM-TEPLOTY------*/
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);


/*-----------DEFINÍCIA-FARIEB---------------*/
long ColorTable[16] = {
  CRGB::Amethyst,
  CRGB::Aqua,
  CRGB::Blue,
  CRGB::Chartreuse,
  CRGB::DarkGreen,
  CRGB::DarkMagenta,
  CRGB::DarkOrange,
  CRGB::DeepPink,
  CRGB::Fuchsia,
  CRGB::Gold,
  CRGB::GreenYellow,
  CRGB::LightCoral,
  CRGB::Tomato,
  CRGB::Salmon,
  CRGB::Red,
  CRGB::Orchid
};

/*-----------VÝBER-FARBY--------------------*/
long ledColor = CRGB::Red;
