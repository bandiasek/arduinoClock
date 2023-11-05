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
#define time_offset   3600  // define a clock offset of 3600 seconds (1 hour) ==> UTC + 1 ?? neviem preco
#define NUM_LEDS 29 // počet led pásikov v hodinách (4*7 + 1..dvojbodka v strede)
#define COLOR_ORDER BRG  // poradie farieb
#define DST_PIN 2  // tlacidlo na nastavovanie daylight-saving-time
#define MIN_PIN 4  // tlacidlo na nastavenie minút
#define HUR_PIN 5  // tlačidlo na nastavenie hodín
#define DATA_PIN 6  // pin na prenos dat
#define BRI_PIN 0  // photorezistor na snimanie svetla
#define ONE_WIRE_BUS 3 // Data kabel na spojenie so snímačom teploty
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
byte prev_hour, temp_started_showing, Second, Minute, Hour, Day, Month;
int Year;

const long intervalColor = 30000; //menenie farieb internval
float prevColorMillis = 0;
int indexOfColor = 0;

const long intervalTemp = 90000; //Each 90 sec show temp
const long intervalShowTempFor = 10000; // show temp for 10 sec
float prevTempMillis = 0;

/*-----------FLAGS-----------*/
bool DST = true; //Šetrič (Daylight saving time)
bool TempShow = false; //Ukazovanie teploty 
bool Dot = true;  //Stav dvojbodky
bool TimeSynced = false; // GPS synced time

/*-----------FUNKCIA-SETUP-PREBEHNE-LEN-RAZ-*/
void setup(){ 
  SoftSerial.begin(9600);
  Serial.begin(115200); 

  TempShow = false; // Nastavenie ukazovania teploty na log 0, aby sa prvé ukázal čas
  LEDS.addLeds<WS2812, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS); //Nastavenie typu led pásiku
  LEDS.setBrightness(75); // Nastavenie Jasu

  sensors.begin();
  sensors.requestTemperatures(); // Začne sa žiadať teplota z RTC

//  Prepinanie hodin je vypnute kvoli gps
//  pinMode(DST_PIN, INPUT_PULLUP); // Tlačidlo na preínanie šetriacého režimu
//  pinMode(MIN_PIN, INPUT_PULLUP); // Tlačidlo na prepínanie minút
//  pinMode(HUR_PIN, INPUT_PULLUP); // Tlačidlo na prepínanie hodín
  Serial.println("Lets begin!");
} 


/*-----------FUNKCIA-NA-ZÍSKANIE-SVETLA-V-OKOLÍ--------------------------*/
void BrightnessCheck(){
  
  //definícia pinu
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


/*-----------FUNKCIA-NA-MENENIE-FARIEB-------------------------*/
void ColorChange(){
  //definícia millis
  long currMillis = millis();

  if (currMillis - prevColorMillis >= intervalColor ){
  
  //prepisanie millis
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

void showTempCheck(){
  //definícia millis
  long currMillis = millis();

  if (currMillis - prevTempMillis >= intervalTemp ){
    //prepisanie millis
    prevTempMillis = currMillis;
    TempShow = true;
  }
 };
  
/*-----------FUNKCIA-NA-PREMENU-ČASU-NA-POLE----------------------*/
void FormatTime(){
  
  //Získanie času z hornej funkcie
  int Now = (hour()*100) + minute();
  //cursor je posledna led, v nášom prípade ich máme 29
  int cursor = 29;  

  // Blikanie dvojbodky
  if (Dot){
    leds[14]=dotColor;   
  }
    else  {
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


/*-----------FUNKCIA-NA-UKAZANIE-A-FORMATOVANIE-TEPLOTY-------------------*/
void TempToArray(){  
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

/*-----------FUNKCIA-NA-SNÍMANIE-DST-TLACIDLA----------------------*/
//void DSTcheck(){
////   int buttonDST = digitalRead(2);
//   int buttonDST = HIGH; // Temporary disabling of dst
//   if (buttonDST == LOW){
//      DST = !DST;
//      dotColor = DST ? CRGB::Gold : CRGB::Red;
//      Serial.println(dotColor);
//      delay(3000);   
//   };
//  }


/*-----------FUNKCIA-NA-SNÍMANIE-UPRAVOVACÍCH-TLAČIDIEL----------------------*/
// void TimeAdjust(){
//  int buttonH = digitalRead(HUR_PIN);
//  int buttonM = digitalRead(MIN_PIN);
//  if (buttonH == LOW || buttonM == LOW){
//    delay(500);
//    tmElements_t Now;
//    RTC.read(Now);
//    int hour=Now.Hour;
//    int minutes=Now.Minute;
//    int second =Now.Second;
//      if (buttonH == LOW){
//        if (Now.Hour== 23){Now.Hour=0;}
//          else {Now.Hour += 1;};
//        }else {
//          if (Now.Minute== 59){Now.Minute=0;}
//          else {Now.Minute += 1;};
//          };
//    RTC.write(Now); 
//    }
//  }


void syncGPSTime() {
  while (!TimeSynced && SoftSerial.available() > 0){
      // Precitanie cosu faunoveho
      if (gps.encode(SoftSerial.read())){
        
        // .. Get Time from gps module
        if (gps.time.isValid()){
          Minute = gps.time.minute();
          Second = gps.time.second();
          Hour   = gps.time.hour();
        }

        // .. Get date drom GPS module
        if (gps.date.isValid()){
           Day   = gps.date.day();
           Month = gps.date.month();
           Year  = gps.date.year();
        }

        // ..
        if(Day && Second){  
          prev_hour = Hour;
          
          // set current UTC time
          setTime(Hour, Minute, Second, Day, Month, Year);
          adjustTime(time_offset);
          
          Serial.println("Time sucessfully sinced");
          TimeSynced = true;
        }
       }
    }  
}

  // Vypis cas kazdu minutu
  byte prev_minute = 0;
  void serialPrintTime(){
    if(prev_minute != minute()){
        prev_minute = minute();
      
        Serial.print(hour());
        Serial.print(":");
        Serial.print(minute());
        Serial.print(":");
        Serial.println(second());
    }
  }

  void serialPrintTemperature(){
     Serial.print(sensors.getTempCByIndex(0));
     Serial.println(" Stupnov");
  }


  void loop(){
     // .. Ensure dot is flashing
     if(second() % 2 == 0){
        Dot = false;
     } else {
        Dot = true;
     }

     // .. Decide to sync every hour
     if(prev_hour != (hour() - time_offset / 3600)){
      TimeSynced = false; 
     }

     // .. Main loop
     if(TimeSynced){
        BrightnessCheck();
        ColorChange();
        showTempCheck();

        // ..
        if(TempShow == true){
          long currMillis = millis();
          if(currMillis - prevTempMillis >= intervalShowTempFor ) TempShow = false;
          
          TempToArray();
          //serialPrintTemperature();

        } else {
          FormatTime();
          serialPrintTime();
        } 
      
        LEDS.show();   
     } else {
      // .. Sync the time through GPS
      syncGPSTime();
     }
   }
