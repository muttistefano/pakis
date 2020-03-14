// include the library code:
#include <LiquidCrystal.h>

// pin schermo
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//pin sens temp
const int sensorPinT = A0;
double tempT = 0.0;

// servo
#include <Servo.h>
Servo myServo1;
int cnt = 0;
int angle;


// make some custom characters:
byte heart[8] = {
  0b00000,
  0b01010,
  0b11111,
  0b11111,
  0b11111,
  0b01110,
  0b00100,
  0b00000
};

byte smiley[8] = {
  0b00000,
  0b00000,
  0b01010,
  0b00000,
  0b00000,
  0b10001,
  0b01110,
  0b00000
};

byte frownie[8] = {
  0b00000,
  0b00000,
  0b01010,
  0b00000,
  0b00000,
  0b00000,
  0b01110,
  0b10001
};

byte armsDown[8] = {
  0b00100,
  0b01010,
  0b00100,
  0b00100,
  0b01110,
  0b10101,
  0b00100,
  0b01010
};

byte armsUp[8] = {
  0b00100,
  0b01010,
  0b00100,
  0b10101,
  0b01110,
  0b00100,
  0b00100,
  0b01010
};

void setup() {
  // initialize LCD and set up the number of columns and rows:
  lcd.begin(16, 2);

  // create a new character
  lcd.createChar(0, heart);
  // create a new character
  lcd.createChar(1, smiley);
  // create a new character
  lcd.createChar(2, frownie);
  // create a new character
  lcd.createChar(3, armsDown);
  // create a new character
  lcd.createChar(4, armsUp);


  

  // set the cursor to the top left
  lcd.setCursor(0, 0);

  // Print a message to the lcd.
  lcd.print("T: ");
  lcd.setCursor(0, 1);
  lcd.print("T: ");
  lcd.setCursor(7, 0);
  lcd.print("U: ");
  lcd.setCursor(7, 1);
  lcd.print("U: ");
  //lcd.write(byte(0)); // when calling lcd.write() '0' must be cast as a byte
  //lcd.print(" Arduino! ");
  //lcd.write((byte)1);

  //Setup servo
  myServo1.attach(6);

}



void loop() {
  // read the potentiometer on A0:
  
  int tempread = analogRead(A0);
  tempT = (double)tempread / 1024;
  tempT = tempT * 5;
  tempT = tempT - 0.5;
  tempT = tempT * 100;
  lcd.setCursor(2, 0);
  lcd.print(tempT);
  angle = map(cnt,0,1023,0,179);
  myServo1.write(angle);
  lcd.setCursor(2, 1);
  lcd.print(angle);
  delay(100);
  cnt = cnt + 2;
}
