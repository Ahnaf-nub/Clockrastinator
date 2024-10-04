#include "DHT.h"
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <RTClib.h>
#include <Wire.h>

#define buzzer 2
#define button 21
#define DHTPIN 22

bool state = false; // Initial state set to false
bool lastButtonState = HIGH; // To track the previous button state
DHT dht(DHTPIN, DHT11);
Adafruit_SSD1306 display(128, 64, &Wire, -1);
RTC_DS1307 rtc;

unsigned long pomodoroStartTime;
unsigned long pomodoroDuration = 25 * 60 * 1000; // 25 minutes in milliseconds
bool pomodoroActive = false;

void setup() {
  pinMode(button, INPUT_PULLUP);  // Set button pin as input with pull-up resistor
  pinMode(buzzer, OUTPUT);        // Set buzzer pin as output
  
  // Initialize display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (true);
  }

  // Initialize RTC
  if (!rtc.begin()) {
    abort();
  }
  if (!rtc.isrunning()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Initialize DHT sensor
  dht.begin();
  display.setTextColor(WHITE);
  display.clearDisplay();

  // Test the buzzer in setup
  for (int i = 0; i < 3; i++) {
    digitalWrite(buzzer, HIGH);  // Turn on the buzzer
    delay(500);                  // Wait for half a second
    digitalWrite(buzzer, LOW);   // Turn off the buzzer
    delay(500);                  // Wait for half a second
  }
}

void displayOtherInfo() {
  DateTime now = rtc.now();
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  char buf[] = "DDD, MMM DD YYYY";

  // Get the current hour and determine AM or PM
  int hour = now.hour();
  String period = "AM";
  if (hour >= 12) {
    period = "PM";
    if (hour > 12) hour -= 12;  // Convert to 12-hour format
  } else if (hour == 0) {
    hour = 12;  // Adjust for midnight
  }

  // Convert the hour, minute, and second to strings for formatting
  String currentTime = String(hour) + ":" + (now.minute() < 10 ? "0" : "") + String(now.minute()) + " " + period;

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0, 10);
  display.print("Temperature: ");
  display.setCursor(80, 10);
  display.print(temperature);
  display.setCursor(110, 10);
  display.print((char)247); // Print degree symbol
  display.print("C");

  display.setCursor(0, 20);
  display.print("Humidity: ");
  display.setCursor(80, 20);
  display.print(int(humidity));
  display.setCursor(95, 20);
  display.print("%");

  display.setCursor(15, 35);
  display.print(now.toString(buf));

  // Display the formatted time with AM/PM
  display.setCursor(20, 48);
  display.print("Time: ");
  display.setCursor(60, 48);
  display.print(currentTime);

  display.display();
}

void displayPomodoroTimeRemaining() {
  unsigned long elapsedTime = millis() - pomodoroStartTime;
  unsigned long remainingTime = pomodoroDuration - elapsedTime;

  if (remainingTime <= 0) {
    pomodoroActive = false;
    state = false; // Set state to false when time is up
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(10, 25);
    display.print("Time's up!");
    delay(1000);  // Brief delay before showing other info

    // Buzz the buzzer for 1 second when time is up
    digitalWrite(buzzer, HIGH);
    delay(1000);
    digitalWrite(buzzer, LOW);

    displayOtherInfo();  // Display other info after Pomodoro ends
    display.display();
    return;
  }

  unsigned long minutesRemaining = remainingTime / 60000;
  unsigned long secondsRemaining = (remainingTime % 60000) / 1000;

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 10);
  display.print("Pomodoro Timer Active");
  display.setCursor(20, 25);
  display.print("Time remaining");
  display.setTextSize(2);
  display.setCursor(20, 45);
  display.print(minutesRemaining);
  display.print("m ");
  display.print(secondsRemaining);
  display.print("s");
  display.display();
}

void loop() {
  bool buttonState = digitalRead(button);
  
  if (buttonState == LOW && lastButtonState == HIGH) { // Button press detected
    state = !state; // Toggle the state
    if (state) {
      pomodoroStartTime = millis();
      pomodoroActive = true; // Start Pomodoro if state is true
    } else {
      pomodoroActive = false; // Stop Pomodoro if state is false
    }
    delay(50); // Debounce delay
  }

  lastButtonState = buttonState; // Update the last button state

  if (state && pomodoroActive) {
    displayPomodoroTimeRemaining(); // Show remaining time if Pomodoro is active
  } else {
    displayOtherInfo(); // Show other information if Pomodoro is not active
  }
}
