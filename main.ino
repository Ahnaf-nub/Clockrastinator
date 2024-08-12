#include "DHT.h"
#include <Servo.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RTClib.h>
#include <Wire.h>

#define DHTPIN 18
#define button 16
#define buzzer 15
#define DHTTYPE DHT22

bool state = false; // Initial state set to false
bool lastButtonState = HIGH; // To track the previous button state
Servo servo;
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(128, 64, &Wire, -1);
RTC_DS1307 rtc;

unsigned long pomodoroStartTime;
unsigned long pomodoroDuration = 25 * 60 * 1000; // 25 minutes in milliseconds
bool pomodoroActive = false;

void setup() {
  Serial.begin(115200);
  pinMode(button, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }
  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  servo.attach(28);
  dht.begin();
}

void pomodoro() {
  pomodoroStartTime = millis();
  pomodoroActive = true;

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.print("Pomodoro Timer Started");
  display.display();
}

void displayPomodoroTimeRemaining() {
  unsigned long elapsedTime = millis() - pomodoroStartTime;
  unsigned long remainingTime = pomodoroDuration - elapsedTime;

  if (remainingTime <= 0) {
    pomodoroActive = false;
    state = false; // Set state to false when time is up
    digitalWrite(buzzer, HIGH);
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(10, 25);
    display.print("Time's up!");
    display.display();
    return;
  }

  unsigned long minutesRemaining = remainingTime / 60000;
  unsigned long secondsRemaining = (remainingTime % 60000) / 1000;

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.print("Pomodoro Timer Active");
  display.setCursor(0, 30);
  display.print("Time remaining: ");
  display.setCursor(0, 40);
  display.print(minutesRemaining);
  display.print("m ");
  display.print(secondsRemaining);
  display.print("s");
  display.display();
}

void displayOtherInfo() {
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
  display.setCursor(15, 30);
  display.print(now.toString(buf));
  display.setCursor(0, 40);
  display.print("Time: ");
  display.setCursor(50, 40);
  display.print(now.toString(buf1));
  display.display();
}

void loop() {
  bool buttonState = digitalRead(button); // Read the current button state
  if (buttonState == LOW && lastButtonState == HIGH) { // Button press detected
    state = !state; // Toggle the state
    if (state) {
      pomodoro(); // Start Pomodoro if state is true
    } else {
      pomodoroActive = false; // Stop Pomodoro if state is false
    }
    delay(50); // Debounce delay
  }

  lastButtonState = buttonState; // Update the last button state

  if (state && pomodoroActive) {
    displayPomodoroTimeRemaining(); // Show remaining time if Pomodoro is active
  } else {
    digitalWrite(buzzer, LOW);
    displayOtherInfo(); // Show other information if Pomodoro is not active
  }
}
