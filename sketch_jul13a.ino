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
