#include "DHT.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RTClib.h>
#include <Wire.h>

#define buzzer 15
#define button 16
#define DHTPIN 18
#define ldr 28
#define DHTTYPE DHT22

bool state = false; // Initial state set to false
bool lastButtonState = HIGH; // To track the previous button state
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(128, 64, &Wire, -1);
RTC_DS1307 rtc;

int light = 0;
int lightThreshold = 500; // Adjust this value as needed for your environment
unsigned long pomodoroStartTime;
unsigned long pomodoroDuration = 25 * 60 * 1000; // 25 minutes in milliseconds
bool pomodoroActive = false;

void setup() {
  pinMode(button, INPUT_PULLUP);
  pinMode(ldr, INPUT);
  pinMode(buzzer, OUTPUT);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (true);
  }
  if (!rtc.begin()) {
    abort();
  }
  if (!rtc.isrunning()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  dht.begin();
  display.setTextColor(WHITE);
}

void pomodoro() {
  pomodoroStartTime = millis();
  pomodoroActive = true;

  display.clearDisplay();
  display.setTextSize(1);
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
    display.setCursor(10, 25);
    display.print("Time's up!");
    display.display();
    return;
  }

  unsigned long minutesRemaining = remainingTime / 60000;
  unsigned long secondsRemaining = (remainingTime % 60000) / 1000;

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 10);
  display.print("Pomodoro Timer Active");
  display.setCursor(20, 30);
  display.print("Time remaining");
  display.setCursor(45, 40);
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
  display.setCursor(110, 10);
  display.print("*C");
  display.setCursor(0, 20);
  display.print("Humidity: ");
  display.setCursor(80, 20);
  display.print(int(humidity));
  display.setCursor(95, 20);
  display.print("%");
  display.setCursor(15, 35);
  display.print(now.toString(buf));
  display.setCursor(30, 45);
  display.print("Time: ");
  display.setCursor(60, 45);
  display.print(now.toString(buf1));
  display.display();
}

void loop() {
  bool buttonState = digitalRead(button); 
  light = analogRead(ldr);
  if (buttonState == LOW && lastButtonState == HIGH) { // Button press detected
    state = !state; // Toggle the state
    if (state) {
      if (light > lightThreshold) { // Check if there is sufficient light
        pomodoro(); // Start Pomodoro if state is true and light is sufficient
      } else {
        state = false; // Reset state to false if light is not sufficient
      }
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
