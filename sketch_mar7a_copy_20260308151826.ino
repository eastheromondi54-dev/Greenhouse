#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// Pin Definitions
const int BUZZER_PIN = 4;
const int TRIG_PIN = 5;
const int ECHO_PIN = 6;
const int SOIL_MOISTURE_PIN = 34;
const int DHT22_PIN = 21;

// Constants
const float SOUND_SPEED = 0.034; // Speed of sound in cm/us
const int MAX_DISTANCE_CM = 50;  // Maximum distance for buzzer alert
const int SOIL_DRY_THRESHOLD = 40; // Soil moisture percentage threshold
const int READINGS_COUNT = 10;   // Number of readings for averaging
const unsigned long SENSOR_READ_INTERVAL = 2000; // 2 seconds between readings

// Objects
LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht22(DHT22_PIN, DHT22);

// Variables
unsigned long lastSensorRead = 0;
float distance = 0.0;
int soilMoisture = 0;
int moisturePercent = 0;
float humidity = 0.0;
float temperatureC = 0.0;
float temperatureF = 0.0;

// Function to read soil moisture with averaging
int readSoilMoisture() {
  int totalMoisture = 0;
  for (int i = 0; i < READINGS_COUNT; i++) {
    totalMoisture += analogRead(SOIL_MOISTURE_PIN);
    delay(10);
  }
  return totalMoisture / READINGS_COUNT;
}

// Function to read ultrasonic sensor
float readDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long timing = pulseIn(ECHO_PIN, HIGH, 30000); // Timeout after 30ms
  if (timing == 0) return 999.9; // No echo received
  
  return (timing * SOUND_SPEED) / 2;
}

// Function to update LCD display
void updateLCD() {
  lcd.clear();
  
  // First row: Soil moisture and distance
  lcd.setCursor(0, 0);
  lcd.print("S:");
  lcd.print(moisturePercent);
  lcd.print("% ");
  
  lcd.print("D:");
  if (distance < 100) {
    lcd.print((int)distance);
  } else {
    lcd.print("---");
  }
  lcd.print("cm");
  
  // Second row: Temperature and humidity
  lcd.setCursor(0, 1);
  if (!isnan(temperatureC)) {
    lcd.print((int)temperatureC);
    lcd.print("C ");
    lcd.print((int)humidity);
    lcd.print("%H");
  } else {
    lcd.print("Sensor Error");
  }
}

// Function to control buzzer based on distance
void controlBuzzer() {
  static unsigned long lastBuzzerToggle = 0;
  static bool buzzerState = false;
  
  if (distance <= MAX_DISTANCE_CM && distance > 0) {
    // Buzzer pattern: beep every 500ms when object is close
    if (millis() - lastBuzzerToggle >= 500) {
      buzzerState = !buzzerState;
      if (buzzerState) {
        tone(BUZZER_PIN, 1000);
      } else {
        noTone(BUZZER_PIN);
      }
      lastBuzzerToggle = millis();
    }
  } else {
    noTone(BUZZER_PIN);
    buzzerState = false;
  }
}

// Function to print all sensor data to Serial
void printSerialData() {
  Serial.println("=== SENSOR READINGS ===");
  
  // Distance
  Serial.print("Distance: ");
  if (distance < 100) {
    Serial.print(distance);
    Serial.print(" cm | ");
    Serial.print(distance / 2.54);
    Serial.println(" in");
  } else {
    Serial.println("Out of range");
  }
  
  // Soil moisture
  Serial.print("Soil Moisture: ");
  Serial.print(soilMoisture);
  Serial.print(" raw | ");
  Serial.print(moisturePercent);
  Serial.println("%");
  Serial.print("Soil Status: ");
  Serial.println(moisturePercent < SOIL_DRY_THRESHOLD ? "DRY" : "WET");
  
  // DHT22
  if (!isnan(humidity) && !isnan(temperatureC)) {
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print("% | ");
    Serial.print("Temperature: ");
    Serial.print(temperatureC);
    Serial.print("°C ~ ");
    Serial.print(temperatureF);
    Serial.println("°F");
  } else {
    Serial.println("DHT22 sensor error!");
  }
  
  Serial.println("======================");
}

void setup() {
  Serial.begin(9600);
  
  // Initialize pins
  pinMode(ECHO_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Set initial states
  digitalWrite(TRIG_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  
  // Initialize DHT22
  dht22.begin();
  
  // Welcome message
  lcd.setCursor(0, 0);
  lcd.print("System Starting");
  lcd.setCursor(0, 1);
  lcd.print("Please wait...");
  delay(2000);
  lcd.clear();
}

void loop() {
  // Read sensors at specified interval
  if (millis() - lastSensorRead >= SENSOR_READ_INTERVAL) {
    
    // Read all sensors
    distance = readDistance();
    soilMoisture = readSoilMoisture();
    moisturePercent = map(soilMoisture, 4095, 0, 0, 100);
    moisturePercent = constrain(moisturePercent, 0, 100);
    
    humidity = dht22.readHumidity();
    temperatureC = dht22.readTemperature();
    temperatureF = dht22.readTemperature(true);
    
    // Update display and serial
    updateLCD();
    printSerialData();
    
    lastSensorRead = millis();
  }
  
  // Control buzzer (runs independently)
  controlBuzzer();
}