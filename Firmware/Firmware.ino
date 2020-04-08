
// Include Libraries
#include "CircularBuffer.h"
#include "Arduino.h"
#include "DHT.h"
#include "LiquidCrystal.h"
#include "Button.h"
#include "Wire.h"
#include "RTClib.h"
#include "Servo.h"


// Pin Definitions
#define DHT_1_PIN_DATA	2
#define DHT_2_PIN_DATA	3
#define LCD_PIN_RS	9
#define LCD_PIN_E	8
#define LCD_PIN_DB4	4
#define LCD_PIN_DB5	5
#define LCD_PIN_DB6	6
#define LCD_PIN_DB7	7
#define PUSHBUTTON_1_PIN_2	10
#define PUSHBUTTON_2_PIN_2	11
// #define RELAYMODULE4CH_PIN_IN1	12
// #define RELAYMODULE4CH_PIN_IN2	13
// #define RELAYMODULE4CH_PIN_IN4	A1
// #define RELAYMODULE4CH_PIN_IN3	A3
#define RELAYMODULE4CH_PIN_IN1	A1
#define RELAYMODULE4CH_PIN_IN2	A3
#define RELAYMODULE4CH_PIN_IN4	12
#define RELAYMODULE4CH_PIN_IN3	13
#define SERVO9G1_1_PIN_SIG	A0
#define SERVO9G2_2_PIN_SIG	A2



//define an array for the 4ch relay module pins
int RelayModule4chPins[] = { RELAYMODULE4CH_PIN_IN1, RELAYMODULE4CH_PIN_IN2, RELAYMODULE4CH_PIN_IN3, RELAYMODULE4CH_PIN_IN4 };
const int servo9g1_1RestPosition   = 0;  //Starting position
const int servo9g1_1TargetPosition = 50; //Position when event is detected
const int servo9g2_2RestPosition   = 0;  //Starting position
const int servo9g2_2TargetPosition = 25; //Position when event is detected
// object initialization
DHT dht_1(DHT_1_PIN_DATA);
DHT dht_2(DHT_2_PIN_DATA);
LiquidCrystal lcd(LCD_PIN_RS,LCD_PIN_E,LCD_PIN_DB4,LCD_PIN_DB5,LCD_PIN_DB6,LCD_PIN_DB7);
Button pushButton_1(PUSHBUTTON_1_PIN_2);
Button pushButton_2(PUSHBUTTON_2_PIN_2);
RTC_DS3231 rtcDS;
Servo servo9g1_1;
Servo servo9g2_2;

byte EmChar[] = {
  B11111,
  B10001,
  B10001,
  B10001,
  B10001,
  B10001,
  B10001,
  B11111
};

byte FuChar[] = {
  B11111,
  B10001,
  B10101,
  B11111,
  B11111,
  B10101,
  B10001,
  B11111
};

byte OpenRele[] = {
  B00010,
  B00100,
  B01000,
  B10000,
  B00010,
  B00010,
  B00010,
  B00010
};

byte CloseRele[] = {
  B00100,
  B00100,
  B00100,
  B00100,
  B00100,
  B00100,
  B00100,
  B00100
};

// define vars for testing menu
const int timeout = 10000;       //define timeout of 10 sec
char menuOption = 0;
long time0;

static char T1[4];
static char U1[4];
static char T2[4];
static char U2[4];

bool b1 = false;
bool b2 = false;

bool Lamp1 = false;
bool Lamp2 = false;

bool Hole1O = false;
bool Hole2O = false;

bool TappOn = false;
bool UmidOn = false;

CircularBuffer<float, 30> T1buff;
CircularBuffer<float, 30> T2buff;
CircularBuffer<float, 30> U1buff;
CircularBuffer<float, 30> U2buff;

float T1filt = 0.0;
float T2filt = 0.0;
float U1filt = 0.0;
float U2filt = 0.0;

float TLTH1  = 24.0;
float TUTH1  = 28.0;

float ULTH1  = 68.0;
float UUTH1  = 85.0;

float TUTH2  = 26.0;

float ULTH2  = 35.0;
float UUTH2  = 55.0;

void setup()
{
    // Setup Serial which is useful for debugging
    // Use the Serial Monitor to view printed messages
    // Serial.begin(9600);
    // while (!Serial) ; // wait for serial port to connect. Needed for native USB
    // Serial.println("start");

    dht_1.begin();
    dht_2.begin();

    lcd.begin(16, 2);

    lcd.setCursor(0, 0);
    lcd.createChar(0, EmChar);
    lcd.createChar(1, FuChar);
    lcd.createChar(2, OpenRele);
    lcd.createChar(3, CloseRele);

    lcd.setCursor(15, 0);
    lcd.write(byte(0));
    lcd.setCursor(15, 1);
    lcd.write(byte(0));


    lcd.setCursor(14, 0);
    lcd.write(byte(2));
    lcd.setCursor(14, 1);
    lcd.write(byte(2));

    pushButton_1.init();
    pushButton_2.init();

    rtcDS.begin();
    if (rtcDS.lostPower())
    {
        Serial.println("RTC lost power, lets set the time!");
        rtcDS.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
    rtcDS.adjust(DateTime(F(__DATE__), F(__TIME__)));

    pinMode(RELAYMODULE4CH_PIN_IN1, OUTPUT);
    pinMode(RELAYMODULE4CH_PIN_IN2, OUTPUT);
    pinMode(RELAYMODULE4CH_PIN_IN3, OUTPUT);
    pinMode(RELAYMODULE4CH_PIN_IN4, OUTPUT);

    servo9g1_1.attach(SERVO9G1_1_PIN_SIG);
    servo9g1_1.write(servo9g1_1RestPosition);
    delay(100);
    servo9g1_1.detach();

    servo9g2_2.attach(SERVO9G2_2_PIN_SIG);
    servo9g2_2.write(servo9g2_2RestPosition);
    delay(100);
    servo9g2_2.detach();

    delay(1000);
    digitalWrite(RelayModule4chPins[0],HIGH);
    delay(200);
    digitalWrite(RelayModule4chPins[1],HIGH);
    delay(200);
    digitalWrite(RelayModule4chPins[2],HIGH);
    delay(200);
    digitalWrite(RelayModule4chPins[3],HIGH);

    delay(4000);

}


void loop()
{

  T1filt = 0.0;
  T2filt = 0.0;
  U1filt = 0.0;
  U2filt = 0.0;

  float dht_1Humidity1 = dht_1.readHumidity();
  float tempT1         = dht_1.readTempC();

  float dht_2Humidity2 = dht_2.readHumidity();
  float tempT2         = dht_2.readTempC();

  T1buff.push(tempT1);
  T2buff.push(tempT2);

  U1buff.push(dht_1Humidity1);
  U2buff.push(dht_2Humidity2);

 	using index_t = decltype(T1buff)::index_t;
  for (index_t i = 0; i < T1buff.size(); i++)
  {
    T1filt += T1buff[i] / T1buff.size();
    T2filt += T2buff[i] / T1buff.size();
    U1filt += U1buff[i] / T1buff.size();
    U2filt += U2buff[i] / T1buff.size();
  }

  T1filt = T1filt - 1;
  T2filt = T2filt - 1;

  int T1p = (int)(T1filt * 10);
  int T2p = (int)(T2filt * 10);

  lcd.setCursor(0, 0);
  lcd.print((int)T1p);
  lcd.setCursor(4, 0);
  lcd.print((int)U1filt);

  lcd.setCursor(0, 1);
  lcd.print((int)T2p);
  lcd.setCursor(4, 1);
  lcd.print((int)U2filt);

  DateTime now = rtcDS.now();
  int h = now.hour();
  int m = now.minute();
  float totsec = (float)h + (float)(m)/60.0;
  int timeint = (int)(totsec * 100);
  lcd.setCursor(7, 0);
  // if (h<10){
  //  lcd.print(0);
  // }
  // lcd.print(h);
  // lcd.setCursor(9, 0);
  // lcd.print(":");
  // lcd.setCursor(10, 0);
  // if (m<10){
  //   lcd.print(0);
  // }
  // lcd.print(m);
  lcd.print(timeint);

  while(false)
  {
    digitalWrite(RelayModule4chPins[0],LOW);
    digitalWrite(RelayModule4chPins[1],LOW);
    digitalWrite(RelayModule4chPins[2],LOW);
    digitalWrite(RelayModule4chPins[3],LOW);
    delay(2000);
    digitalWrite(RelayModule4chPins[0],LOW);
    digitalWrite(RelayModule4chPins[1],LOW);
    digitalWrite(RelayModule4chPins[2],LOW);
    digitalWrite(RelayModule4chPins[3],HIGH);
    delay(2000);
    digitalWrite(RelayModule4chPins[0],LOW);
    digitalWrite(RelayModule4chPins[1],LOW);
    digitalWrite(RelayModule4chPins[2],HIGH);
    digitalWrite(RelayModule4chPins[3],HIGH);
    delay(2000);
    digitalWrite(RelayModule4chPins[0],LOW);
    digitalWrite(RelayModule4chPins[1],HIGH);
    digitalWrite(RelayModule4chPins[2],HIGH);
    digitalWrite(RelayModule4chPins[3],HIGH);
    delay(2000);
    digitalWrite(RelayModule4chPins[0],HIGH);
    digitalWrite(RelayModule4chPins[1],HIGH);
    digitalWrite(RelayModule4chPins[2],HIGH);
    digitalWrite(RelayModule4chPins[3],HIGH);
    delay(2000);
    digitalWrite(RelayModule4chPins[0],HIGH);
    digitalWrite(RelayModule4chPins[1],HIGH);
    digitalWrite(RelayModule4chPins[2],HIGH);
    digitalWrite(RelayModule4chPins[3],LOW);
    delay(2000);
    digitalWrite(RelayModule4chPins[0],HIGH);
    digitalWrite(RelayModule4chPins[1],HIGH);
    digitalWrite(RelayModule4chPins[2],LOW);
    digitalWrite(RelayModule4chPins[3],LOW);
    delay(2000);
    digitalWrite(RelayModule4chPins[0],HIGH);
    digitalWrite(RelayModule4chPins[1],LOW);
    digitalWrite(RelayModule4chPins[2],LOW);
    digitalWrite(RelayModule4chPins[3],LOW);
    delay(2000);
    digitalWrite(RelayModule4chPins[0],LOW);
    digitalWrite(RelayModule4chPins[1],LOW);
    digitalWrite(RelayModule4chPins[2],LOW);
    digitalWrite(RelayModule4chPins[3],LOW);
    delay(2000);
    digitalWrite(RelayModule4chPins[0],LOW);
    digitalWrite(RelayModule4chPins[1],LOW);
    digitalWrite(RelayModule4chPins[2],HIGH);
    digitalWrite(RelayModule4chPins[3],HIGH);
    delay(2000);
    digitalWrite(RelayModule4chPins[0],HIGH);
    digitalWrite(RelayModule4chPins[1],LOW);
    digitalWrite(RelayModule4chPins[2],HIGH);
    digitalWrite(RelayModule4chPins[3],HIGH);
    delay(2000);
    digitalWrite(RelayModule4chPins[0],LOW);
    digitalWrite(RelayModule4chPins[1],HIGH);
    digitalWrite(RelayModule4chPins[2],HIGH);
    digitalWrite(RelayModule4chPins[3],HIGH);
    delay(2000);
  }

  if(false)
  {
    unsigned long startedWaiting = millis();
    while( millis() - startedWaiting <= 1000)
    {
      b1 = pushButton_1.onPress();
      b2 = pushButton_2.onPress();

      if(false)
      {
        servo9g1_1.attach(SERVO9G1_1_PIN_SIG);
        if(Hole1O)
        {
          servo9g1_1.write(servo9g1_1RestPosition);
          delay(1500);
          Hole1O = false;
          lcd.setCursor(15, 0);
          lcd.write(byte(0));
        }
        else
        {
          servo9g1_1.write(servo9g1_1TargetPosition);
          delay(1500);
          Hole1O = true;
          lcd.setCursor(15, 0);
          lcd.write(byte(1));
        }
        servo9g1_1.detach();
      }

      if(false)
      {
        servo9g2_2.attach(SERVO9G2_2_PIN_SIG);
        if(Hole2O)
        {
          servo9g2_2.write(servo9g2_2RestPosition);
          delay(1500);
          Hole2O = false;
          lcd.setCursor(15, 1);
          lcd.write(byte(0));
        }
        else
        {
          servo9g2_2.write(servo9g2_2TargetPosition);
          delay(1500);
          Hole2O = true;
          lcd.setCursor(15, 1);
          lcd.write(byte(1));
        }
        servo9g2_2.detach();
      }
      if(b1)
      {digitalWrite(RelayModule4chPins[1],HIGH);}
      if(b2)
      {digitalWrite(RelayModule4chPins[1],LOW);}
      delay(50);
    }
  }

  bool istime1 = (totsec > 19.00 || totsec < 9.0 ) ? true:false;

  bool istime2 = (totsec > 19.00 || totsec < 7.0 ) ? true:false;

  if(istime1 && !Lamp1)
  {
    digitalWrite(RelayModule4chPins[0],LOW);
    delay(500);
    lcd.setCursor(8, 1);
    lcd.write("si");
    Lamp1 = true;
  }

  if(!istime1 && Lamp1)
  {
    digitalWrite(RelayModule4chPins[0],HIGH);
    delay(500);
    lcd.setCursor(8, 1);
    lcd.write("no");
    Lamp1 = false;
  }

  if(istime2 && !Lamp2)
  {
    digitalWrite(RelayModule4chPins[1],LOW);
    delay(500);
    lcd.setCursor(10, 1);
    lcd.write("si");
    Lamp2 = true;
  }

  if(!istime2 && Lamp2)
  {
    digitalWrite(RelayModule4chPins[1],HIGH);
    delay(500);
    lcd.setCursor(10, 1);
    lcd.write("no");
    Lamp2 = false;
  }



  if((T1filt > TUTH1) && !Hole1O)
  {
    TUTH1 = TUTH1 - 0.3;
    servo9g1_1.attach(SERVO9G1_1_PIN_SIG);
    servo9g1_1.write(servo9g1_1TargetPosition);
    delay(1500);
    Hole1O = true;
    lcd.setCursor(15, 0);
    lcd.write(byte(1));
    servo9g1_1.detach();
  }

  if((T1filt < TUTH1) && Hole1O)
  {
    TUTH1 = TUTH1 + 0.3;
    servo9g1_1.attach(SERVO9G1_1_PIN_SIG);
    servo9g1_1.write(servo9g1_1RestPosition);
    delay(1500);
    Hole1O = false;
    lcd.setCursor(15, 0);
    lcd.write(byte(0));
    servo9g1_1.detach();
  }

  if((T1filt < TLTH1) && !TappOn)
  {
    TLTH1 = TLTH1 + 0.3;
    digitalWrite(RelayModule4chPins[3],LOW);
    delay(500);
    TappOn = true;
    lcd.setCursor(14, 0);
    lcd.write(byte(3));
  }

  if((T1filt > TLTH1) && TappOn)
  {
    TLTH1 = TLTH1 - 0.3;
    digitalWrite(RelayModule4chPins[3],HIGH);
    delay(500);
    TappOn = false;
    lcd.setCursor(14, 0);
    lcd.write(byte(2));
  }

  if((U1filt > UUTH1) && !Hole1O)
  {
    UUTH1 = UUTH1 - 0.5;
    servo9g1_1.attach(SERVO9G1_1_PIN_SIG);
    servo9g1_1.write(servo9g1_1TargetPosition);
    delay(1500);
    Hole1O = true;
    lcd.setCursor(15, 0);
    lcd.write(byte(1));
    servo9g1_1.detach();
  }

  if((U1filt < UUTH1) && Hole1O)
  {
    UUTH1 = UUTH1 + 0.5;
    servo9g1_1.attach(SERVO9G1_1_PIN_SIG);
    servo9g1_1.write(servo9g1_1RestPosition);
    delay(1500);
    Hole1O = false;
    lcd.setCursor(15, 0);
    lcd.write(byte(0));
    servo9g1_1.detach();
  }

  if((U1filt < ULTH1) && !UmidOn)
  {
    ULTH1 = ULTH1 + 0.3;
    digitalWrite(RelayModule4chPins[2],LOW);
    delay(500);
    UmidOn = true;
    lcd.setCursor(14, 1);
    lcd.write(byte(3));
  }

  if((U1filt > ULTH1) && UmidOn)
  {
    ULTH1 = ULTH1 - 0.3;
    digitalWrite(RelayModule4chPins[2],HIGH);
    delay(500);
    UmidOn = false;
    lcd.setCursor(14, 1);
    lcd.write(byte(2));
  }

  if((T2filt > TUTH2) && !Hole2O)
  {
    // TUTH2 = TUTH2 - 0.3;
    servo9g2_2.attach(SERVO9G2_2_PIN_SIG);
    servo9g2_2.write(servo9g2_2TargetPosition);
    delay(1500);
    Hole2O = true;
    lcd.setCursor(15, 1);
    lcd.write(byte(1));
    servo9g2_2.detach();
  }

  // if((T2filt < TUTH2) && Hole2O)
  // {
  //   TUTH2 = TUTH2 + 0.3;
  //   servo9g2_2.attach(SERVO9G2_2_PIN_SIG);
  //   servo9g2_2.write(servo9g2_2RestPosition);
  //   delay(1500);
  //   Hole2O = false;
  //   lcd.setCursor(15, 1);
  //   lcd.write(byte(0));
  //   servo9g2_2.detach();
  // }

  if((U2filt > UUTH2) && !Hole2O)
  {
    UUTH2 = UUTH2 - 0.5;
    servo9g2_2.attach(SERVO9G2_2_PIN_SIG);
    servo9g2_2.write(servo9g2_2RestPosition);
    delay(1500);
    Hole2O = true;
    lcd.setCursor(15, 1);
    lcd.write(byte(1));
    servo9g2_2.detach();
  }

  if((U2filt < ULTH2) && Hole2O)
  {
    UUTH2 = UUTH2 + 0.5;
    servo9g2_2.attach(SERVO9G2_2_PIN_SIG);
    servo9g2_2.write(servo9g2_2TargetPosition);
    delay(1500);
    Hole2O = false;
    lcd.setCursor(15, 1);
    lcd.write(byte(0));
    servo9g2_2.detach();
  }



  delay(4000);

}

// Main logic of your circuit. It defines the interaction between the components you selected. After setup, it runs over and over again, in an eternal loop.
// void loop()
// {


//     if(menuOption == '1') {
//     // DHT22/11 Humidity and Temperature Sensor #1 - Test Code
//     // Reading humidity in %
//     float dht_1Humidity = dht_1.readHumidity();
//     // Read temperature in Celsius, for Fahrenheit use .readTempF()
//     float dht_1TempC = dht_1.readTempC();
//     Serial.print(F("Humidity: ")); Serial.print(dht_1Humidity); Serial.print(F(" [%]\t"));
//     Serial.print(F("Temp: ")); Serial.print(dht_1TempC); Serial.println(F(" [C]"));

//     }
//     else if(menuOption == '2') {
//     // DHT22/11 Humidity and Temperature Sensor #2 - Test Code
//     // Reading humidity in %
//     float dht_2Humidity = dht_2.readHumidity();
//     // Read temperature in Celsius, for Fahrenheit use .readTempF()
//     float dht_2TempC = dht_2.readTempC();
//     Serial.print(F("Humidity: ")); Serial.print(dht_2Humidity); Serial.print(F(" [%]\t"));
//     Serial.print(F("Temp: ")); Serial.print(dht_2TempC); Serial.println(F(" [C]"));

//     }
//     else if(menuOption == '3') {
//     // LCD 16x2 - Test Code
//     // Print a message to the LCD.
//     lcd.setCursor(0, 0);
//     lcd.print("Circuito Rocks !");
//     // Turn off the display:
//     lcd.noDisplay();
//     delay(500);
//     // Turn on the display:
//     lcd.display();
//     delay(500);
//     }
//     else if(menuOption == '4') {
//     // Mini Pushbutton Switch #1 - Test Code
//     //Read pushbutton state.
//     //if button is pressed function will return HIGH (1). if not function will return LOW (0).
//     //for debounce funtionality try also pushButton_1.onPress(), .onRelease() and .onChange().
//     //if debounce is not working properly try changing 'debounceDelay' variable in Button.h
//     bool pushButton_1Val = pushButton_1.read();
//     Serial.print(F("Val: ")); Serial.println(pushButton_1Val);

//     }
//     else if(menuOption == '5') {
//     // Mini Pushbutton Switch #2 - Test Code
//     //Read pushbutton state.
//     //if button is pressed function will return HIGH (1). if not function will return LOW (0).
//     //for debounce funtionality try also pushButton_2.onPress(), .onRelease() and .onChange().
//     //if debounce is not working properly try changing 'debounceDelay' variable in Button.h
//     bool pushButton_2Val = pushButton_2.read();
//     Serial.print(F("Val: ")); Serial.println(pushButton_2Val);

//     }
//     else if(menuOption == '6') {
//     // RTC - Real Time Clock - Test Code
//     //This will display the time and date of the RTC. see RTC.h for more functions such as rtcDS.hour(), rtcDS.month() etc.

//     Serial.print(now.month(), DEC);
//     Serial.print('/');
//     Serial.print(now.day(), DEC);
//     Serial.print('/');
//     Serial.print(now.year(), DEC);
//     Serial.print("  ");
//     Serial.print(now.hour(), DEC);
//     Serial.print(':');
//     Serial.print(now.minute(), DEC);
//     Serial.print(':');
//     Serial.print(now.second(), DEC);
//     Serial.println();
//     delay(1000);
//     }
//     else if(menuOption == '7') {
//     // Relay Module 4-Ch - Test Code
//     //This loop will turn on and off each relay in the array for 0.5 sec
//     for (int i = 0; i < 4; i++) {
//     digitalWrite(RelayModule4chPins[i],HIGH);
//     delay(500);
//     digitalWrite(RelayModule4chPins[i],LOW);
//     delay(500);
//     }
//     }
//     else if(menuOption == '8') {
//     // 9G Micro Servo #1 - Test Code
//     // The servo will rotate to target position and back to resting position with an interval of 500 milliseconds (0.5 seconds)
//     servo9g1_1.attach(SERVO9G1_1_PIN_SIG);         // 1. attach the servo to correct pin to control it.
//     servo9g1_1.write(servo9g1_1TargetPosition);  // 2. turns servo to target position. Modify target position by modifying the 'ServoTargetPosition' definition above.
//     delay(500);                              // 3. waits 500 milliseconds (0.5 sec). change the value in the brackets (500) for a longer or shorter delay in milliseconds.
//     servo9g1_1.write(servo9g1_1RestPosition);    // 4. turns servo back to rest position. Modify initial position by modifying the 'ServoRestPosition' definition above.
//     delay(500);                              // 5. waits 500 milliseconds (0.5 sec). change the value in the brackets (500) for a longer or shorter delay in milliseconds.
//     servo9g1_1.detach();                    // 6. release the servo to conserve power. When detached the servo will NOT hold it's position under stress.
//     }
//     else if(menuOption == '9') {
//     // 9G Micro Servo #2 - Test Code
//     // The servo will rotate to target position and back to resting position with an interval of 500 milliseconds (0.5 seconds)
//     servo9g2_2.attach(SERVO9G2_2_PIN_SIG);         // 1. attach the servo to correct pin to control it.
//     servo9g2_2.write(servo9g2_2TargetPosition);  // 2. turns servo to target position. Modify target position by modifying the 'ServoTargetPosition' definition above.
//     delay(500);                              // 3. waits 500 milliseconds (0.5 sec). change the value in the brackets (500) for a longer or shorter delay in milliseconds.
//     servo9g2_2.write(servo9g2_2RestPosition);    // 4. turns servo back to rest position. Modify initial position by modifying the 'ServoRestPosition' definition above.
//     delay(500);                              // 5. waits 500 milliseconds (0.5 sec). change the value in the brackets (500) for a longer or shorter delay in milliseconds.
//     servo9g2_2.detach();                    // 6. release the servo to conserve power. When detached the servo will NOT hold it's position under stress.
//     }

//     // if (millis() - time0 > timeout)
//     // {
//     //     menuOption = menu();
//     // }

// }



// Menu function for selecting the components to be tested
// Follow serial monitor for instrcutions
// char menu()
// {

//     Serial.println(F("\nWhich component would you like to test?"));
//     Serial.println(F("(1) DHT22/11 Humidity and Temperature Sensor #1"));
//     Serial.println(F("(2) DHT22/11 Humidity and Temperature Sensor #2"));
//     Serial.println(F("(3) LCD 16x2"));
//     Serial.println(F("(4) Mini Pushbutton Switch #1"));
//     Serial.println(F("(5) Mini Pushbutton Switch #2"));
//     Serial.println(F("(6) RTC - Real Time Clock"));
//     Serial.println(F("(7) Relay Module 4-Ch"));
//     Serial.println(F("(8) 9G Micro Servo #1"));
//     Serial.println(F("(9) 9G Micro Servo #2"));
//     Serial.println(F("(menu) send anything else or press on board reset button\n"));
//     while (!Serial.available());

//     // Read data from serial monitor if received
//     while (Serial.available())
//     {
//         char c = Serial.read();
//         if (isAlphaNumeric(c))
//         {

//             if(c == '1')
//     			Serial.println(F("Now Testing DHT22/11 Humidity and Temperature Sensor #1"));
//     		else if(c == '2')
//     			Serial.println(F("Now Testing DHT22/11 Humidity and Temperature Sensor #2"));
//     		else if(c == '3')
//     			Serial.println(F("Now Testing LCD 16x2"));
//     		else if(c == '4')
//     			Serial.println(F("Now Testing Mini Pushbutton Switch #1"));
//     		else if(c == '5')
//     			Serial.println(F("Now Testing Mini Pushbutton Switch #2"));
//     		else if(c == '6')
//     			Serial.println(F("Now Testing RTC - Real Time Clock"));
//     		else if(c == '7')
//     			Serial.println(F("Now Testing Relay Module 4-Ch"));
//     		else if(c == '8')
//     			Serial.println(F("Now Testing 9G Micro Servo #1"));
//     		else if(c == '9')
//     			Serial.println(F("Now Testing 9G Micro Servo #2"));
//             else
//             {
//                 Serial.println(F("illegal input!"));
//                 return 0;
//             }
//             time0 = millis();
//             return c;
//         }
//     }
// }
