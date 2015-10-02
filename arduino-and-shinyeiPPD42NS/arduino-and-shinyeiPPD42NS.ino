/*
 * This uses the Liquid Crystal library from https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads GNU General Public License, version 3 (GPL-3.0)
 * Pin Connections:
 * LCD I2C:
 *      SCL => A5
 *      SDA => A4
 *      VCC => pin 12
 *      GND => GND
 * Shinyei PPD42NS:
 *      JST Pin 1  => Arduino GND
 *      JST Pin 3  => Arduino 5VDC
 *      JST Pin 4  => Arduino Digital Pin 3

Dylos Air Quality Chart (1 micron)+
1000+ = VERY POOR
350 - 1000 = POOR
100 - 350 = FAIR
50 - 100 = GOOD
25 - 50 = VERY GOOD
0 - 25 = TOTALLY EXCELLENT, DUDE!
 */

#include <SoftwareSerial.h>
#include <MemoryFree.h>
#include <stdlib.h>
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C	lcd(0x27,2,1,0,4,5,6,7); // 0x27 is the I2C bus address for an unmodified module

// custom charaters for progress bar
byte p1[8] = {
  0x10,
  0x10,
  0x10,
  0x10,
  0x10,
  0x10,
  0x10,
  0x10};

byte p2[8] = {
  0x18,
  0x18,
  0x18,
  0x18,
  0x18,
  0x18,
  0x18,
  0x18};

byte p3[8] = {
  0x1C,
  0x1C,
  0x1C,
  0x1C,
  0x1C,
  0x1C,
  0x1C,
  0x1C};

byte p4[8] = {
  0x1E,
  0x1E,
  0x1E,
  0x1E,
  0x1E,
  0x1E,
  0x1E,
  0x1E};

byte p5[8] = {
  0x1F,
  0x1F,
  0x1F,
  0x1F,
  0x1F,
  0x1F,
  0x1F,
  0x1F};
  
// values for progress bar
#define lenght 16.0
double percent=100.0;
unsigned char b;
unsigned int peace;
bool firstTime = true; // for showing a countdown screen on the LCD

// settings for dust sensor
const byte numReadings = 36;     // moving average period
const byte particlePin = 3; // P1 pin, second from left on board
unsigned long duration;
unsigned long startTime_ms;
unsigned long sampleTime_ms = 5000;
int timeRemaining;
unsigned long lowPulseOccupancy = 0;
float ratio = 0;
float concentration = 0;
float average;
int particles; // for converting the average to an integer, fractions of a particle don't make sense
int firstTimeCounter = 1;
float alpha = 2/(float(numReadings) + 1); // exponential moving average recursive weighting, using conventional 2/(N + 1)

void setup() {
  // ### setup LCD screen ###
  pinMode(12, OUTPUT);
  digitalWrite(12, HIGH);
  lcd.setBacklightPin(3,POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.begin(16, 2); // critically, this must come before createChar
  lcd.createChar(0, p1);
  lcd.createChar(1, p2);
  lcd.createChar(2, p3);
  lcd.createChar(3, p4);
  lcd.createChar(4, p5);
  
  lcd.clear();
  
  Serial.begin(9600);
    
  pinMode(particlePin,INPUT);
  
  startTime_ms = millis(); // get the current time in milliseconds
  lcd.setCursor(0,0);
}

void loop() {
  if (firstTime) {
    printStatus();
  }
  duration = pulseIn(particlePin, LOW);
  lowPulseOccupancy = lowPulseOccupancy+duration;

  if ((millis()-startTime_ms) >= sampleTime_ms) // if the sample time has been exceeded
  {
    ratio = lowPulseOccupancy/(sampleTime_ms*10.0);  // Integer percentage 0=>100; divide by 1000 to convert us to ms, multiply by 100 for %, end up dividing by 10
    concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio; // using spec sheet curve for shinyei PPD42ns

    // calculate the exponential moving average:
    if (firstTime) {
      average = concentration;
      firstTime = false;
    }
    else {
      average = alpha * concentration + (1 - alpha) * average;
    }
    particles = int(average); // fractions of a particle don't make sense
    
    lcd.clear();
    lcd.setCursor(0, 0);
    Serial.print("average: ");
    Serial.println(particles);
    Serial.print("concentration: ");
    Serial.println(concentration);
    if (average > 1000) { // air quality is VERY POOR
      lcd.print("very poor");
    }
    else if (average > 350) { // air quality is POOR
      lcd.print("poor");
    }
    else if (average > 100) { // air quality is FAIR
      lcd.print("fair");
    }
    else if (average > 50) { // air quality is GOOD
      lcd.print("good");
    }
    else if (average > 25) { // air quality is VERY GOOD
      lcd.print("very good");
    }
    else { // air quality is EXCELLENT (<25 counts)
      lcd.print("excellent");
    }
    lcd.setCursor(0, 1);
    lcd.print(particles);
    lowPulseOccupancy = 0;
    startTime_ms = millis(); // reset the timer for sampling at the end so it is as accurate as possible
  }
}

void printStatus() {
  lcd.setCursor(0, 0);

  percent = (millis() - startTime_ms)/float(sampleTime_ms)*100.0;

  lcd.print("startin up...");
  timeRemaining = (sampleTime_ms - (millis()-startTime_ms))/1000;
  if (timeRemaining > 9) {
    lcd.print(timeRemaining);
    lcd.print("s");
  }
  else {
    lcd.print(" ");
    lcd.print(timeRemaining);
    lcd.print("s");
  }
  
  lcd.setCursor(0,1);

  double a=lenght/100*percent;

  // drawing black rectangles on LCD

  if (a>=1) {

    for (int i=1;i<a;i++) {

      lcd.write(byte(4));

      b=i;
    }

    a=a-b;

  }

  peace=a*5;

  // drawing charater's colums

  switch (peace) {

  case 0:

    break;

  case 1:
    lcd.write(byte(0));

    break;

  case 2:
    lcd.write(byte(1));
    break;

  case 3:
    lcd.write(byte(2));
    break;

  case 4:
    lcd.write(byte(3));
    break;

  }

  //clearing line
  for (int i =0;i<(lenght-b);i++) {
    lcd.print(" ");
  }
}
