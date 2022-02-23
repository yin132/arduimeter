#include <MS5611.h>
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

MS5611 baro;
int32_t pressure;
float filtered = 0;
float mtr;
float ref = 0;

// define some values used by the panel and buttons
int lcd_key = 0;
int adc_key_in = 0;
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

// read the buttons
int read_LCD_buttons() {
    adc_key_in = analogRead(0);      // read the value from the sensor
    // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
    // we add approx 50 to those values and check to see if we are close
    if (adc_key_in > 1000)
        return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
    // For V1.1 us this threshold
    if (adc_key_in < 50) return btnRIGHT;
    if (adc_key_in < 250) return btnUP;
    if (adc_key_in < 450) return btnDOWN;
    if (adc_key_in < 650) return btnLEFT;
    if (adc_key_in < 850) return btnSELECT;

    return btnNONE;  // when all others fail, return this...
}


void setup() {
    // Start barometer
    baro = MS5611();
    baro.begin();
    // Start serial (UART)
    Serial.begin(9600);
    delay(2);
    // Start LCD
    lcd.begin(16, 2);
    lcd.clear();
    lcd.print("Height m");

}

void loop() {
    // Read pressure
    pressure = baro.getPressure();

    if (filtered != 0) {
        filtered = filtered + 0.05 * (pressure - filtered);
    } else {
        filtered = pressure;          // first reading so set filtered to reading
    }
    if (ref == 0) {
        ref = filtered / 12.0;
    }

    if (read_LCD_buttons() == btnSELECT) {
        ref = filtered / 12.0;
    }

    mtr = ref - filtered / 12.0;

    // Send pressure via serial (UART);
    Serial.println(mtr);
    lcd.setCursor(0, 1);         // line 2
    lcd.print(mtr);


}
