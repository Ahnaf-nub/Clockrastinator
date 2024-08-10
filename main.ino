#include "DHT.h"
#include <Servo.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RTClib.h>
#include <Wire.h>
#define DHTPIN 18
#define button 16
#define DHTTYPE DHT22
bool state = 0;
Servo servo;
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(128, 64, &Wire, -1);
RTC_DS1307 rtc;
void setup() {
  Serial.begin(115200);
  pinMode(button, INPUT_PULLUP);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }
  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  servo.attach(28);
  dht.begin();
}
void loop() {
  DateTime now = rtc.now();
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  char buf[] = "DDD, MMM DD YYYY";
  char buf1[] = "hh:mm";
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.print("Temperature: ");
  display.setCursor(80, 10);
  display.print(temperature);
  display.setCursor(0, 20);
  display.print("Humidity: ");
  display.setCursor(80, 20);
  display.print(humidity);
  display.setCursor(20, 30);
  display.print(now.toString(buf));
  display.setCursor(0, 40);
  display.print("Time: ");
  display.setCursor(50, 40);
  display.print(now.toString(buf1));
  display.display();
}
