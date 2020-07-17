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
#define DST_PIN 2  // tlacidlo na nastavovanie daylight-saving-time
#define MIN_PIN 4  // tlacidlo na nastavenie minút
#define HUR_PIN 5  // tlačidlo na nastavenie hodín
#define DATA_PIN 6  // pin na prenos dat
#define BRI_PIN 0  // photorezistor na snimanie svetla
#define ONE_WIRE_BUS 3 // Data kabel na spojenie so snímačom teploty

/*-----------DEFINÍCIA-ZNAKOV------------*/
byte digits[12][7] = {{0,1,1,1,1,1,1},  // číslo 0
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
                     {0,0,1,1,1,1,0},}; // znak C

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
long dotColor = CRGB::Red;


/*-----------DEFINÍCIA-LED-PÁSIKA-----------*/
CRGB leds[NUM_LEDS];


/*-----------DEFINÍCIA-PREMENNÝCH-----------*/
bool Dot = true;  //Stav dvojbodky
bool DST = true; //Šetrič (Daylight saving time)
bool TempShow = false; //Ukazovanie teploty 



/*-----------FUNKCIA-SETUP-PREBEHNE-LEN-RAZ-*/
void setup(){ 
  Serial.begin(9600); 
  
  LEDS.addLeds<WS2812, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS); //Nastavenie typu led pásiku
  LEDS.setBrightness(75); // Nastavenie Jasu
  
  pinMode(DST_PIN, INPUT_PULLUP); // Tlačidlo na preínanie šetriacého režimu
  pinMode(MIN_PIN, INPUT_PULLUP); // Tlačidlo na prepínanie minút
  pinMode(HUR_PIN, INPUT_PULLUP); // Tlačidlo na prepínanie hodín
  
  TempShow = false; // Nastavenie ukazovania teploty na log 0, aby sa prvé ukázal čas
  
  sensors.begin();
  sensors.requestTemperatures(); // Začne sa žiadať teplota z RTC
} 

/*-----------FUNKCIA-NA-ZÍSKANIE-ČASU-A-BLIKANIE-DVOJBODKY-V-STREDE------*/
/*-----------PREMENA-ČASOVÉEHO-FORMÁTU-Z-0155-NA-155---------------------*/
int GetTime(){
  tmElements_t Now;
  RTC.read(Now);
  
  int hour = Now.Hour;
  int minutes = Now.Minute;
  int second = Now.Second;
  
  if (second % 2==0) {
      Dot = false;
      } else {
        Dot = true;
        BrightnessCheck(); // Zistenie svetla + nastavenie ak je DST true
       };
       
  return (hour*100+minutes);
  };
  

/*-----------FUNKCIA-NA-ZÍSKANIE-SVETLA-V-OKOLÍ--------------------------*/
void BrightnessCheck(){
  
  //definícia pinu
  const byte sensorPin = BRI_PIN; // light sensor pin

  //načítanie jasu + formovanie do potrebnej hodnoty
  int sensorValue = analogRead(sensorPin); 
  sensorValue = map(sensorValue, 10, 900, 255, 40);
  
  //výpis na konzolu
  Serial.print("Jas bude: ");
  Serial.println(sensorValue);
  
  //nastavenie jasu ak je DST povolene
  if(DST){
    LEDS.setBrightness(sensorValue);
    } else {
        LEDS.setBrightness(180);
      }
  
  };
  
  
/*-----------FUNKCIA-NA-PREMENU-ČASU-NA-POLE----------------------*/
void FormatTime(){
  
  //Získanie času z hornej funkcie
  int Now = GetTime();
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
        if(Now >= 1){
          if (digits[digit][k]== 1){
              leds[cursor]=ledColor;
            
             }  else if (digits[digit][k]==0){
                leds[cursor]=0x000000;
                
                };
           cursor ++;
           };
        };
      }
    Now /= 10;
  }; 
};


/*-----------FUNKCIA-NA-UKAZANIE-A-FORMATOVANIE-TEPLOTY-------------------*/
void TempToArray(){
  tmElements_t tm;

// Získanie času na prepočet času
  RTC.read(tm);

// Samotný prepočet času a riešenie koľko sa bude ukazovať teplota
  if (tm.Second != 5) { 
          
      return;                    
    }
    TempShow = true;
    
  // Poslanie príkazu na čítanie teploty
  sensors.requestTemperatures(); 
  
  int celsius = sensors.getTempCByIndex(0);

  // Výpis na konzolu
  Serial.print("Temp is: ");
  Serial.println(celsius);

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
};

/*-----------FUNKCIA-NA-SNÍMANIE-DST-TLACIDLA----------------------*/
void DSTcheck(){
   int buttonDST = digitalRead(2);
   if (buttonDST == LOW){
    if (DST){
      dotColor = CRGB::Gold;
      DST=false;
      }
      else if (!DST){
        dotColor = CRGB::Red;
        DST=true;
      };
   delay(500);   
   };
  }


/*-----------FUNKCIA-NA-SNÍMANIE-UPRAVOVACÍCH-TLAČIDIEL----------------------*/
 void TimeAdjust(){
  int buttonH = digitalRead(HUR_PIN);
  int buttonM = digitalRead(MIN_PIN);
  if (buttonH == LOW || buttonM == LOW){
    delay(500);
    tmElements_t Now;
    RTC.read(Now);
    int hour=Now.Hour;
    int minutes=Now.Minute;
    int second =Now.Second;
      if (buttonH == LOW){
        if (Now.Hour== 23){Now.Hour=0;}
          else {Now.Hour += 1;};
        }else {
          if (Now.Minute== 59){Now.Minute=0;}
          else {Now.Minute += 1;};
          };
    RTC.write(Now); 
    }
  }


  void loop()
  { 
    DSTcheck(); // Zistenie tlačidla DST
    TimeAdjust(); // Zistenie či sa nezmenil čas
    FormatTime(); // Formatovanie casu a nastavenie led
    TempToArray(); // Spracovavanie teploty
    FastLED.show(); // Ukázanie / zasvietenie led
    // cislo 5000 znamena ako dlho bude ukazana teplota
    if (TempShow == true) delay (7500);
    TempShow = false;
   }
