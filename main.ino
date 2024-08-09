#include "DHT.h"
#include <Servo.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RTClib.h>
#include <Wire.h>
#define DHTPIN 18
#define button 16
#define DHTTYPE DHT22
Servo servo;
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(128, 64, &Wire, -1);
void setup() {
  Serial.begin(115200);
  pinMode(button, INPUT_PULLUP);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }
  servo.attach(28);
  dht.begin();
}
void loop() {
  if button == true {
  float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    display.print("Temperature: ");
    display.setCursor(0, 80);
    display.print(temperature);
    display.setCursor(10, 10);
    display.print("Humidity: ");
    display.setCursor(10, 80);
    display.print(humidity);
    display.display();
    delay(1000);
  }
  else{
    // gonna add later
  }
}
