// https://simple-circuit.com/arduino-gps-clock-local-time-neo-6m/

/*-----------POUZITÉ-KNIŽNICE------------*/
#include <TinyGPS++.h> 
#include <TimeLib.h>
#include <SoftwareSerial.h>

#include <DS3232RTC.h>
#include <Time.h> 
#include <FastLED.h> 
#include <OneWire.h> 
#include <DallasTemperature.h>

/*-----------DEFINÍCIA-PINOV------------*/
#define S_RX 12     // define software serial RX pin
#define S_TX 10   // define software serial TX pin
#define WINTER_OFFSET 3600  // define a clock offset of 3600 seconds (1 hour) ==> UTC + 1 ?? neviem preco
#define SUMMER_OFFSET 7200  // define a clock offset of 3600 seconds (1 hour) ==> UTC + 1 ?? neviem preco
#define SYNC_TIME 30000 // Time which willing to wait for syncing
#define NUM_LEDS 29 // počet led pásikov v hodinách (4*7 + 1..dvojbodka v strede)
#define COLOR_ORDER BRG  // poradie farieb
//#define MIN_PIN 4  // tlacidlo na nastavenie minút
//#define HUR_PIN 5  // tlačidlo na nastavenie hodín
#define DATA_PIN 6  // pin na prenos dat
#define BRI_PIN 0  // photorezistor na snimanie svetla
#define ONE_WIRE_BUS 3 // Data kabel na spojenie so snímačom teploty

#define SUMMER_TIME_BUTTON 2 // Pin na nastavenie posunutia casu +
#define STATIC_BRIGHTNESS 4  // tlacidlo na nastavovanie daylight-saving-time

/*-----------DEFINÍCIA-ZNAKOV------------*/
byte digits[13][7] = {{0,1,1,1,1,1,1},  // číslo 0
                     {0,1,0,0,0,0,1},   // číslo 1
                     {1,1,1,0,1,1,0},   // číslo 2
                     {1,1,1,0,0,1,1},   // číslo 3
                     {1,1,0,1,0,0,1},   // číslo 4
                     {1,0,1,1,0,1,1},   // číslo 5
                     {1,0,1,1,1,1,1},   // číslo 6
                     {0,1,1,0,0,0,1},   // číslo 7
                     {1,1,1,1,1,1,1},   // číslo 8
                     {1,1,1,1,0,1,1},   // číslo 9
                     {1,1,1,1,0,0,0},   // znak * (stupne)
                     {0,0,1,1,1,1,0},   // znak C
                     {0,0,0,0,0,0,0},}; // prazdny znak

/*-----KOMUNIKÁCIA-SO-SNÍMAČOM-TEPLOTY + Ine instancie------*/
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

DS3232RTC rtc;
TinyGPSPlus gps;
SoftwareSerial SoftSerial(S_RX, S_TX);

/*-----------DEFINÍCIA-FARIEB---------------*/
long ColorTable[3] = {
  CRGB::Red,
  CRGB::Green,
  CRGB::Blue
};

/*-----------VÝBER-FARBY--------------------*/
long ledColor = CRGB::Red;
long dotColor = CRGB::Red;


/*-----------DEFINÍCIA-LED-PÁSIKA-----------*/
CRGB leds[NUM_LEDS];


/*-----------DEFINÍCIA-PREMENNÝCH-----------*/
int time_offset = WINTER_OFFSET;
time_t prevDisplay = 0; 
byte Second, Minute, Hour, Day, Month;
int Year;

const long intervalColor = 30000; //menenie farieb internval
float prevColorMillis = 0;
int indexOfColor = 0;

const long intervalTemp = 90000; //Each 90 sec show temp
const long intervalShowTempFor = 10000; // show temp for 10 sec
float prevTempMillis = 0;

/*-----------FLAGS-----------*/
bool DST = true; //Šetrič (Daylight saving time)
bool Dot = true;  //Stav dvojbodky
bool ShowingTemp = false; // Stav Teploty

/*-----------FUNKCIA-SETUP-PREBEHNE-LEN-RAZ-*/
void setup(){ 
  SoftSerial.begin(9600);
  Serial.begin(115200); 

  LEDS.addLeds<WS2812, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS); //Nastavenie typu led pásiku
  LEDS.setBrightness(75); // Nastavenie Jasu

  sensors.begin();
  sensors.requestTemperatures(); // Začne sa žiadať teplota z RTC

//  Prepinanie hodin je vypnute kvoli gps
//  pinMode(DST_PIN, INPUT_PULLUP); // Tlačidlo na preínanie šetriacého režimu
//  pinMode(MIN_PIN, INPUT_PULLUP); // Tlačidlo na prepínanie minút
//  pinMode(HUR_PIN, INPUT_PULLUP); // Tlačidlo na prepínanie hodín

  pinMode(SUMMER_TIME_BUTTON, INPUT);
  pinMode(STATIC_BRIGHTNESS, INPUT);
  Serial.println("Lets begin!");
} 

/**
 * @brief Printesrs for LEDs
 * Theese functions are suppose to prepare leds to be printed
 */
  
// .. Funkcia na vypisanie casu na ledkach
void ledPrintTime(int Now){
  //cursor je posledna led, v nášom prípade ich máme 29
  int cursor = 29;  

  // Blikanie dvojbodky
  if (Dot){
    leds[14]=dotColor;   
  } else  {
    leds[14]=0x000000;
  };

  //Zaciatok cyklu pre Led 
  for(int i=1;i<=4;i++){
    int digit = Now % 10; // Posledná číslica

    /*---4-segment---*/
    if (i==1){
      cursor = 22; 
   
    for(int k=0; k<=6;k++){ 
        if (digits[digit][k]== 1){
            leds[cursor]=ledColor;
            
            } else if (digits[digit][k]==0){
              leds[cursor]=0x000000;
              
              };
         cursor ++;
        };
      }
      
    /*---3-segment---*/ 
    else if (i==2){
      cursor -= 14;
      
      for(int k=0; k<=6;k++){ 
        if (digits[digit][k]== 1){
            leds[cursor]=ledColor;
            
            } else if (digits[digit][k]==0){
              leds[cursor]=0x000000;
              
              };
         cursor ++;
        };
      }
      
    /*---2-segment---*/ 
    else if (i==3){
      cursor = 7;
      
      for(int k=0; k<=6;k++){ 
          if (digits[digit][k]== 1){
              leds[cursor]=ledColor;
              } else if (digits[digit][k]==0){
                  leds[cursor]=0x000000;
                  
                  };
         cursor ++;
        };
      }
    /*---1-segment---*/ 
    else if (i==4){
      cursor = 0;
      
      for(int k=0; k<=6;k++){ 
        //zaisti vykreslovanie prazdneho znaku
        if(Now < 1){ digit = 12; }
          if (digits[digit][k]== 1){
              leds[cursor]=ledColor;
            
             }  else if (digits[digit][k]==0){
                leds[cursor]=0x000000;
                
                };
           cursor ++;
        };
      }
    Now /= 10;
  }; 
};


// .. Funkcia na ukazanie teploty
void ledPrintTemperature(){  
  sensors.requestTemperatures();
  int celsius = sensors.getTempCByIndex(0);
  
  // nastavenie poslednej led, plus zhasnutie dvojbodky
  int cursor = 29; 
  leds[14] = 0x000000;

  // definicia cisla teploty, z dovodu minusu ukaze najmensiu nulu
  int digit = celsius <= 0 ? 0 : celsius; 

   //Zaciatok cyklu pre Led 
  for(int i=1;i<=4;i++){
    
    /*---4-segment---*/
    if (i==1){
      cursor = 22; 
   
    for(int k=0; k<=6;k++){ 
        if (digits[11][k]== 1){
            leds[cursor]=ledColor;
            
            } else if (digits[11][k]==0){
              leds[cursor]=0x000000;
              
              };
         cursor ++;
        };
      }
      
    /*---3-segment---*/ 
    else if (i==2){
      cursor -= 14;
      
      for(int k=0; k<=6;k++){ 
        if (digits[10][k]== 1){
            leds[cursor]=ledColor;
            
            } else if (digits[10][k]==0){
              leds[cursor]=0x000000;
              
              };
         cursor ++;
        };
      }
      
    /*---2-segment---*/ 
    else if (i==3){
      cursor = 7;
      
      for(int k=0; k<=6;k++){ 
          if (digits[digit%10][k]== 1){
              leds[cursor]=ledColor;
              } else if (digits[digit%10][k]==0){
                  leds[cursor]=0x000000;
                  
                  };
         cursor ++;
        };
      }
    /*---1-segment---*/ 
    else if (i==4){
      cursor = 0;
      
      for(int k=0; k<=6;k++){ 
          if (digits[digit/10][k]== 1){
              leds[cursor]=ledColor;
            
             }  else if (digits[digit/10][k]==0){
                leds[cursor]=0x000000;
                
                };
           cursor ++;
           };
      }
  }
  
  // .. Afterr showing temperature will be reseted
};


/**
 * @brief Serial Printers
 * 
 */

  long prevSerialSecond = 0;
  void serialPrintTime(){
    long currMillis = millis();

    if(currMillis - prevSerialSecond  > 1000 ) {
      prevSerialSecond = currMillis;
      Serial.print(hour());
      Serial.print(":");
      Serial.print(minute());
      Serial.print(":");
      Serial.print(second());
      Serial.print(":offset:");
      Serial.println(time_offset);
    }
  }


  void serialPrintTemperature(){
    long currMillis = millis();

    if(currMillis - prevSerialSecond  > 1000){
      prevSerialSecond = currMillis;

      Serial.print(sensors.getTempCByIndex(0));
      Serial.println(" Stupnov");
    }
  }

  /**
   * @brief Function for checking and vlidating gps
   * @return if gps was successfully read
   * 
   * @author https://forum.arduino.cc/t/tinygps-time-intervals/1116809/10
   */

  bool readGps() {
    long start = millis();
    bool gpsEncodeComplete = false;

    do { // .. Loop until gps data was successfully read and encoded from GPS module
      if (!SoftSerial.available()) {
        // .. No new data available. Lets continue
        continue;
      }

      gpsEncodeComplete = gps.encode(SoftSerial.read());

      if (!gpsEncodeComplete) {
        // Data is incomplete. Lets continue 
        continue;
      }
    } while (!gpsEncodeComplete && millis() - start <= SYNC_TIME);  

    return gpsEncodeComplete;
  }

  void updateGpsTimeAndDate() {
    // .. Get Time from gps module
    if (gps.time.isValid()){
      Minute = gps.time.minute();
      Second = gps.time.second();
      Hour = gps.time.hour();
    }

    // .. Get date drom GPS module
    if (gps.date.isValid()){
      Day = gps.date.day();
      Month = gps.date.month();
      Year = gps.date.year();
    }

    if(Minute && Second && Hour && Day && Month && Year){  
      setTime(Hour, Minute, Second, Day, Month, Year);
      adjustTime(time_offset -1);

      Minute = Second = Hour = Day = Month = Year = -1;
    }
  }

  /**
   * @brief Handlers used to create procedure
   * 
   */

  // .. Ensure dot is flashing
  void handleDotChange() {
     if(second() % 2 == 0){
        Dot = false;
     } else {
        Dot = true;
     }
  }

  // .. Handle offset of a clocks
  void handleTimeOffsetChange() {
    if(digitalRead(SUMMER_TIME_BUTTON)== LOW && time_offset != SUMMER_OFFSET){
      time_offset = SUMMER_OFFSET;
    }

    if(digitalRead(SUMMER_TIME_BUTTON)== HIGH && time_offset != WINTER_OFFSET){
      time_offset = WINTER_OFFSET;
    }
  }

  // .. Handle DST (DynamicBrightness)
  void handleDSTChange() {
    if(digitalRead(STATIC_BRIGHTNESS)== LOW && !DST){
      DST = true;
    }

    if(digitalRead(STATIC_BRIGHTNESS)== HIGH && DST){
      DST = false;
    }
  }

  // .. Handle brightness check
  void handleBrightnessChange(){
    const byte sensorPin = BRI_PIN; // light sensor pin

    //načítanie jasu + formovanie do potrebnej hodnoty
    int sensorValue = analogRead(sensorPin); 
    sensorValue = map(sensorValue, 10, 900, 255, 40);
    
    //nastavenie jasu ak je DST povolene
    if(DST){
      LEDS.setBrightness(sensorValue);
    } else {
      LEDS.setBrightness(180);
    }
  };

  void handleColorChange() {
    long currMillis = millis();

    if (currMillis - prevColorMillis >= intervalColor ){
      prevColorMillis = currMillis;

      if(indexOfColor > 2){
        indexOfColor = 0;
      }

    //prepisanie farby
    ledColor = ColorTable[indexOfColor];
    dotColor = ColorTable[indexOfColor];

    //zaistenie zmeny farby podla indexu
    indexOfColor++;
    }
  };

  // .. Handle if temp should be changed
  bool shouldShowTemp() {
    long currMillis = millis();

    // .. Wait 90 seconds for set showing to true + store when it started
    if(!ShowingTemp && currMillis - prevTempMillis >= intervalTemp) {
      prevTempMillis = currMillis;
      ShowingTemp = true;
    } 
    
    // .. Hide temp when it has been showing for 10 sec
    if(ShowingTemp && currMillis - prevTempMillis >= intervalShowTempFor) {
      ShowingTemp = false;
    }
  
    return ShowingTemp;
 };

  /**
   * @brief Main loop function
   * 
   */
  void loop(){
    handleTimeOffsetChange();
    handleDSTChange();

    handleBrightnessChange();
    handleColorChange();

    bool showTemp = shouldShowTemp();
    if(showTemp) {
      ledPrintTemperature();
      serialPrintTemperature();
    } else {
      bool gpsRead = readGps();
      if(gpsRead) {
        updateGpsTimeAndDate();
      }

      if(timeStatus()!= timeNotSet && now() != prevDisplay) {
        prevDisplay = now();
        handleDotChange();

        serialPrintTime();
        ledPrintTime((hour()*100) + minute());
      }
    }

    LEDS.show();
   }