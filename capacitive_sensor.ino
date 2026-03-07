#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define SOIL_MOISTURE_PIN 34   // Analog pin for soil moisture sensor

// Create LCD object (change 0x27 to 0x3F if not working)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Function to smooth the sensor readings (average over 10 readings)
int readSoilMoisture() {
  int totalMoisture = 0;
  for (int i = 0; i < 10; i++) {
    totalMoisture += analogRead(SOIL_MOISTURE_PIN);
    delay(10);
  }
  return totalMoisture / 10;
}

void setup() {
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Soil Moisture");
  delay(2000);
  lcd.clear();
}

void loop() {
  int soilMoisture = readSoilMoisture();

  // Map analog value to percentage (adjust if needed)
  int moisturePercent = map(soilMoisture, 4095, 0, 0, 100);

  Serial.print("Raw Value: ");
  Serial.println(soilMoisture);

  Serial.print("Moisture %: ");
  Serial.println(moisturePercent);

  lcd.clear();
  
  // First row: Percentage
  lcd.setCursor(0, 0);
  lcd.print("Moisture: ");
  lcd.print(moisturePercent);
  lcd.print("%");

  // Second row: Status
  lcd.setCursor(0, 1);
  if (moisturePercent < 40) {
    lcd.print("Status: DRY");
  } else {
    lcd.print("Status: WET");
  }

  delay(2000);
}

