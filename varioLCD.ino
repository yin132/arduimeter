// variometer and altimeter by yin132, 2021

#include <MS5611.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <RunningAverage.h>

MS5611 baroSense;
LiquidCrystal_I2C lcd(0x27,16, 2);
const byte led = 13;

int buzzerPin = 3; // pin for buzzer
void(* resetFunc) (void) = 0; // default address for resetting
int runningAvgBuffer = 30; // number of values in running average

// readings variables
float altimeterSetting = 30.21;
int stationElev = 0;
int32_t pressure;
int32_t temperature;
int pressureAlt;
float verticalSpeed;

// timers for loops
unsigned long timePrint = 0;
unsigned long timeVSI = 0;

// variables for the vario
float toneFreqLowpass;
float lowpassFast;
float lowpassSlow;
float toneFreq;
int ddsAcc;

// init for running average
RunningAverage x(runningAvgBuffer);   // to store x data (time)
RunningAverage y(runningAvgBuffer);   // to store y data
RunningAverage xy(runningAvgBuffer);  // to store x*y, needed for slope calculation
RunningAverage x2(runningAvgBuffer);  // to store x*x, needed for slope calculation

// initalize
void setup() {
  pinMode(8, OUTPUT); // using pins 8 and 10 as V5
  digitalWrite(8, HIGH);
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
  
  digitalWrite(led, 0);

  baroSense = MS5611();
  baroSense.begin();

  Serial.begin(9600);
  delay(100);


  lcd.begin();  //initialize lcd screen
  lcd.backlight(); // turn on the backlight
  lcd.clear();
  delay(1000);

  pressure = baroSense.getPressure();
  verticalSpeed = 0;
  lowpassFast = lowpassSlow = pressure;

  initRunningAvgs();
    
  timePrint = millis();
}

// main
void loop() {
  getValues();
  playVario();

  if (millis() - timeVSI >= 100) {
    addToRunningAvg();
    timeVSI = millis();
    if (millis() - timePrint >= 1000) {
      updateVSI();
      updateScreen();
      timePrint = millis();
      
      if (millis() >= 1200000) {
        resetFunc();
      }
    }
  }
}

void initRunningAvgs() {
  // start clean
  x.clear();
  y.clear();
  xy.clear();
  x2.clear();
  
  while(x.getCount() < runningAvgBuffer) {
    getValues();
    addToRunningAvg();
  }
}

void addToRunningAvg() {
  float currTime = millis() / 1000.000;
  
  y.addValue(pressureAlt);
  x.addValue(currTime);
  xy.addValue(pressureAlt * currTime);
  x2.addValue(currTime * currTime);
}

// play the tone for the variometer
void playVario() {
  lowpassFast = lowpassFast + (pressure - lowpassFast) * 0.1;
  lowpassSlow = lowpassSlow + (pressure - lowpassSlow) * 0.05;
  toneFreq = (lowpassSlow - lowpassFast) * 5;
  toneFreqLowpass = toneFreqLowpass + (toneFreq - toneFreqLowpass) * 0.1;
  toneFreq = constrain(toneFreqLowpass, -500, 500);
  ddsAcc += toneFreq * 100 + 2000;

  if (toneFreq < -50 || toneFreq > 5 && ddsAcc > 0 ) {
    tone(buzzerPin, toneFreq + 510);
  }
  else {
    noTone(buzzerPin);
  }
}

// get pressure and temperature from MS5611
void getValues() {
  temperature = baroSense.getTemperature();
  pressure = baroSense.getPressure();
  pressureAlt = toPAlt(pressure);
}

// convert a prssure into a pressure altitude
int32_t toPAlt(int32_t p) {
  int result = (1.0 - pow((float) p / 101325.0, 0.190284)) * 145366.45;
  return result;
}

// convert pressure altitude and temperature to density altitude
int getDensityAlt() {
  return (int) (pressureAlt + (118.8 * ((temperature / 100.0) - 15)));
}

// convert pressure altitude to indicated altitude using altimeter setting
int getIndicatedAlt() {
  return pressureAlt - ((29.92 - altimeterSetting) * 1000);
}

// calculates the regression for vertical speed
void updateVSI() {
  verticalSpeed = (xy.getAverage() - (x.getAverage() * y.getAverage()));
  verticalSpeed = verticalSpeed / (x2.getAverage() - (x.getAverage() * x.getAverage())); // calculate slope
  verticalSpeed = verticalSpeed * 60; // convert to minutes
}

// updates the lcd
void updateScreen() {
    lcd.clear();
    lcd.setCursor(0, 0);
//    lcd.print(toSpacedString((int) pressureAlt));
    lcd.print((int) pressureAlt);
    
    lcd.setCursor(0, 1);
//    lcd.print(toSpacedString((int) verticalSpeed));
    lcd.print((int) verticalSpeed);

    Serial.println(pressureAlt);
}

String toSpacedString(int num) {
  String numAsString = String(num);
  Serial.println(numAsString);
  while (numAsString.length() < 6) {
    Serial.println(numAsString.length());
    numAsString = "_" + numAsString;
  }
  return numAsString;
}
