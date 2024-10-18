#include "DHT.h"
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <RTClib.h>
#include <Wire.h>

#define buzzer D10
#define button D0
#define DHTPIN D3
#define ldr D2

bool state = false; // Initial state set to false
bool lastButtonState = HIGH; // To track the previous button state
DHT dht(DHTPIN, DHT11);
Adafruit_SSD1306 display(128, 64, &Wire, -1);
RTC_DS1307 rtc;

unsigned long pomodoroStartTime;
unsigned long pomodoroDuration = 25 * 60 * 1000; // 25 minutes in milliseconds
bool pomodoroActive = false;
unsigned long buttonPressTime = 0;
bool gameActive = false;  // Toggle for game mode

// Pong game variables
int ballX, ballY;
int ballDirX = 1, ballDirY = 1;
int playerPaddleY = 0, aiPaddleY = 0;
const int paddleHeight = 20;
const int paddleSpeed = 2; // Speed at which paddles move
bool paddleMovingUp = false;

// Animation frames for the clock icon
#define FRAME_DELAY (42)
#define FRAME_WIDTH (32)
#define FRAME_HEIGHT (32)
#define FRAME_COUNT (sizeof(frames) / sizeof(frames[0]))
const byte PROGMEM frames[][128] = {
  {0,0,0,0,0,1,0,0,0,15,224,0,0,24,48,0,0,19,200,0,0,38,104,0,0,44,44,0,0,41,44,0,0,44,40,0,0,38,200,0,0,19,136,0,0,16,16,0,0,24,16,0,0,8,16,0,0,16,16,0,0,49,140,0,7,227,135,224,12,1,128,48,16,0,0,8,39,128,3,228,44,64,2,52,41,103,198,150,105,100,36,150,44,104,22,52,39,200,19,228,19,16,8,136,24,96,6,24,7,192,3,224,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,1,0,0,0,15,224,0,0,24,48,0,0,19,200,0,0,38,104,0,0,44,44,0,0,45,164,0,0,44,44,0,0,54,200,0,0,19,136,0,0,16,16,0,0,8,16,0,0,8,16,0,0,16,16,0,0,49,140,0,7,227,135,192,28,1,128,48,16,0,0,8,39,128,3,228,44,64,2,52,73,103,198,150,105,100,36,148,44,104,22,52,39,216,19,228,16,48,8,200,28,96,4,24,7,192,3,224,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,1,128,0,0,7,224,0,0,8,16,0,0,19,200,0,0,38,108,0,0,36,36,0,0,45,164,0,0,36,36,0,0,54,108,0,0,19,200,0,0,24,16,0,0,8,16,0,0,8,16,0,0,16,16,0,0,113,136,0,7,195,135,128,24,1,128,112,49,0,0,24,39,192,1,200,44,64,2,36,75,103,196,180,73,108,36,148,108,72,22,52,39,208,19,100,16,48,25,200,28,224,12,24,3,128,7,224,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,7,224,0,0,12,24,0,0,25,200,0,0,18,100,0,0,20,52,0,0,52,148,0,0,52,52,0,0,18,100,0,0,17,200,0,0,8,8,0,0,8,16,0,0,8,16,0,0,16,16,0,1,241,136,0,15,131,142,0,16,1,129,224,39,128,0,48,44,192,1,136,72,64,7,232,75,71,196,36,72,72,101,180,108,216,36,36,39,144,54,100,16,96,19,200,15,192,8,24,0,0,7,240,0,0,1,128,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,3,240,0,0,4,24,0,0,8,196,0,0,27,116,0,0,18,18,0,0,18,210,0,0,18,18,0,0,27,54,0,0,9,228,0,0,8,8,0,0,8,24,0,0,8,16,0,0,16,16,0,7,225,152,0,24,3,140,0,32,1,131,128,111,128,0,224,72,192,0,16,91,64,7,216,90,79,132,104,72,216,77,40,39,144,41,40,48,32,44,104,29,192,55,200,2,0,17,144,0,0,12,48,0,0,7,192,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,1,240,0,0,3,24,0,0,4,68,0,0,5,242,0,0,9,26,0,0,11,74,0,0,11,74,0,0,9,26,0,0,12,246,0,0,4,4,0,0,4,8,0,0,8,8,0,15,240,16,0,24,65,152,0,32,3,136,0,79,129,142,0,88,192,3,128,82,64,0,224,82,70,3,48,88,223,143,144,79,176,200,88,32,96,75,72,24,192,74,72,7,0,76,208,0,0,39,144,0,0,16,32,0,0,31,192,0,0,3,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,32,0,0,1,252,0,0,3,6,0,0,2,123,0,0,4,141,0,0,5,165,0,0,5,165,0,0,4,141,0,0,4,249,0,0,4,34,0,0,4,6,0,15,136,8,0,56,240,8,0,34,1,152,0,79,3,136,0,88,129,8,0,86,128,6,0,210,128,3,128,88,158,0,192,79,51,14,96,32,96,155,160,31,192,144,176,2,0,150,144,0,0,144,160,0,0,89,160,0,0,79,96,0,0,48,192,0,0,31,128,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,60,0,0,0,198,0,0,1,129,0,0,1,124,128,0,3,70,128,0,2,90,192,0,2,66,128,0,2,102,128,0,2,61,128,31,132,3,0,32,124,6,0,71,0,12,0,89,129,136,0,144,131,152,0,150,129,24,0,208,128,8,0,79,16,4,0,102,124,2,0,48,194,1,128,15,1,28,128,0,1,34,64,0,1,99,64,0,1,73,64,0,1,99,64,0,1,62,64,0,0,156,128,0,0,99,0,0,0,62,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14,0,0,0,51,128,0,0,64,192,0,0,159,64,0,0,177,32,0,1,173,32,31,129,33,32,48,65,51,96,102,62,30,64,89,128,0,192,80,128,3,0,214,129,140,0,80,131,136,0,77,129,16,0,102,0,16,0,48,248,24,0,15,4,8,0,0,4,4,0,0,2,2,0,0,6,251,0,0,4,141,0,0,5,165,0,0,5,165,0,0,4,141,0,0,2,251,0,0,3,2,0,0,1,204,0,0,0,112,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15,128,3,0,48,96,29,192,39,48,48,32,109,144,39,16,88,208,72,136,154,80,82,71,24,208,90,64,13,144,72,128,7,32,47,129,128,96,48,3,135,128,28,225,24,0,7,48,16,0,0,8,48,0,0,8,16,0,0,8,16,0,0,8,24,0,0,25,200,0,0,22,104,0,0,20,36,0,0,53,164,0,0,20,36,0,0,19,104,0,0,25,152,0,0,12,48,0,0,3,192,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,224,0,0,12,16,0,0,25,136,0,0,22,72,3,192,52,36,12,96,37,164,25,16,20,35,55,200,22,97,196,104,27,128,13,40,12,0,4,40,3,193,134,104,0,99,131,216,0,49,0,48,0,16,31,224,0,16,32,0,0,16,64,0,0,32,64,0,0,64,64,0,0,158,64,0,0,179,64,0,1,45,32,0,1,173,32,0,0,161,96,0,0,158,64,0,0,64,128,0,0,49,128,0,0,14,0,0},
  {0,0,0,0,0,0,0,0,0,48,0,0,0,220,0,0,3,2,0,0,2,249,0,0,6,141,0,0,5,165,0,0,4,165,0,0,4,141,128,0,2,120,128,0,1,0,129,192,1,128,102,96,0,96,48,24,0,32,3,200,0,33,134,44,0,51,132,164,0,33,4,164,0,96,6,44,1,128,3,200,2,0,248,16,4,225,135,96,13,177,1,0,11,27,0,0,11,74,0,0,11,26,0,0,13,178,0,0,4,228,0,0,2,12,0,0,1,240,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,7,128,0,0,24,96,0,0,51,48,0,0,47,144,0,0,104,80,0,0,91,80,0,0,74,88,0,0,44,208,0,0,39,16,0,0,16,48,0,0,16,32,0,0,24,32,0,0,16,48,0,0,16,16,0,3,225,143,192,24,3,128,96,48,1,3,16,111,128,15,208,72,192,8,72,90,71,139,72,90,76,72,88,72,216,44,208,39,176,55,176,48,96,24,96,13,192,15,192,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,124,0,0,0,194,0,0,1,153,0,0,1,108,128,0,2,198,128,0,2,218,128,0,2,66,128,0,2,100,128,14,2,25,128,49,134,3,0,68,124,6,0,95,16,12,0,145,128,8,0,164,129,144,0,180,131,144,0,145,1,8,0,78,0,12,0,96,252,3,0,63,132,25,0,0,2,124,128,0,2,66,128,0,2,90,64,0,3,66,128,0,1,102,128,0,1,61,128,0,0,131,0,0,0,126,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,192,0,0,24,96,7,224,19,144,8,48,38,200,19,200,44,104,36,104,41,36,101,40,44,103,197,168,38,192,4,104,19,128,3,200,24,0,0,16,7,129,128,96,0,99,143,192,0,17,24,0,0,24,16,0,0,24,32,0,0,16,48,0,0,17,16,0,0,39,208,0,0,44,72,0,0,41,104,0,0,41,104,0,0,36,72,0,0,55,144,0,0,24,48,0,0,15,192,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,56,0,0,0,230,0,0,1,129,0,0,1,60,128,0,2,70,128,0,2,210,128,0,2,210,128,0,2,70,128,0,1,60,128,0,1,128,128,0,0,192,67,240,0,96,126,8,0,48,1,228,0,16,1,22,0,17,130,90,0,19,130,90,0,33,3,18,0,192,1,244,1,128,124,12,2,120,199,24,2,204,129,224,4,132,128,0,4,181,128,0,4,133,0,0,2,249,0,0,2,51,0,0,1,134,0,0,0,120,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,3,240,0,0,4,8,0,0,9,228,0,0,27,52,0,0,18,148,0,0,18,214,0,0,18,20,0,0,11,228,0,0,8,136,0,0,8,24,0,0,12,16,0,0,12,16,0,0,8,16,0,1,240,24,0,7,1,143,128,8,3,128,112,19,193,0,16,22,96,3,200,52,32,6,44,36,163,197,164,22,36,37,164,19,200,54,108,8,24,19,200,6,112,8,24,3,128,7,96,0,0,1,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,30,0,0,0,97,128,0,0,204,192,0,0,158,64,0,0,161,64,0,1,45,32,0,1,33,96,0,1,51,64,0,1,30,192,7,195,0,128,24,126,3,0,51,128,4,0,38,192,8,0,40,65,136,0,43,99,136,0,40,65,8,0,38,192,4,0,19,156,3,0,24,98,0,192,7,193,14,64,0,1,17,32,0,0,165,160,0,0,165,160,0,0,177,32,0,0,159,64,0,0,64,192,0,0,49,128,0,0,14,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,224,0,0,24,16,0,0,19,216,7,192,38,104,8,48,37,40,19,152,109,168,54,78,196,104,36,34,7,200,45,32,0,16,36,96,0,96,54,193,135,128,19,131,140,0,12,1,8,0,7,248,24,0,0,12,24,0,0,4,8,0,0,4,4,0,0,4,98,0,0,4,250,0,0,5,137,0,0,13,105,0,0,5,9,0,0,4,155,0,0,6,114,0,0,3,4,0,0,0,248,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,240,0,0,4,8,0,224,9,236,3,24,27,52,12,12,18,18,9,244,18,211,51,18,26,16,194,218,9,224,2,82,12,0,3,54,7,1,129,228,1,227,128,8,0,17,15,240,0,8,24,0,0,8,16,0,0,8,32,0,0,24,48,0,0,16,16,0,0,39,144,0,0,36,72,0,0,41,104,0,0,41,104,0,0,36,88,0,0,55,144,0,0,16,48,0,0,15,224,0,0,3,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,240,0,0,1,12,0,0,2,34,0,0,4,251,0,0,5,137,0,0,5,45,0,0,5,13,1,248,4,152,134,4,6,240,120,246,3,0,1,154,1,192,1,74,0,33,131,106,0,19,129,26,0,17,1,242,0,16,12,68,0,16,55,152,0,32,32,224,0,96,64,0,0,204,64,0,0,159,64,0,1,161,32,0,1,45,32,0,0,161,96,0,0,147,64,0,0,204,192,0,0,97,128,0,0,63,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,124,0,0,0,198,0,0,1,17,0,0,1,124,128,0,2,70,128,0,2,210,128,0,2,194,128,0,2,68,128,0,1,60,192,64,1,128,99,248,0,192,62,4,0,32,0,242,0,17,129,154,0,19,129,75,0,17,129,106,0,48,1,10,0,96,24,242,0,192,126,4,1,0,195,152,2,120,128,240,2,196,128,0,2,150,128,0,2,146,128,0,2,196,128,0,2,121,0,0,1,3,0,0,0,206,0,0,0,56,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,8,0,0,0,127,0,0,0,192,128,0,0,158,64,0,1,35,64,0,1,41,96,0,1,105,64,0,1,35,64,0,0,190,64,0,0,136,64,0,0,96,32,0,0,32,51,240,0,16,30,12,0,17,128,100,0,19,129,242,0,49,1,26,0,96,3,107,1,128,3,10,2,0,121,154,4,241,140,246,5,153,6,12,9,9,3,248,9,105,0,0,13,9,0,0,4,250,0,0,4,102,0,0,3,12,0,0,1,240,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,31,0,0,0,49,128,0,0,100,64,0,0,95,32,0,0,145,160,0,0,180,176,0,0,180,160,0,0,145,160,0,0,79,32,0,0,96,32,0,0,32,32,0,0,16,48,0,0,16,31,240,0,17,128,24,0,51,128,4,0,97,1,242,3,128,3,26,6,0,2,74,12,192,226,74,9,225,155,18,18,18,9,246,18,210,6,12,18,18,3,248,11,52,0,224,9,228,0,0,4,8,0,0,3,240,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,15,128,0,0,48,96,0,0,39,32,0,0,77,144,0,0,88,208,0,0,82,80,0,0,88,208,0,0,72,144,0,0,103,48,0,0,48,32,0,0,16,32,0,0,16,16,0,0,16,24,0,0,17,143,240,0,51,128,24,1,193,128,196,6,0,1,246,8,0,2,18,19,192,66,218,22,35,242,90,20,178,27,50,20,180,9,228,22,52,4,12,19,236,3,248,8,8,0,64,6,48,0,0,3,192,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,15,192,0,0,16,96,0,0,39,144,0,0,108,216,0,0,72,72,0,0,75,72,0,0,72,72,0,0,108,208,0,0,39,144,0,0,16,16,0,0,16,48,0,0,16,16,0,0,16,16,0,0,49,143,128,0,227,129,240,7,129,128,8,8,0,1,228,17,128,3,54,55,192,2,18,36,35,230,210,37,166,18,18,36,36,27,116,38,104,9,236,19,200,4,24,8,16,3,240,15,224,0,0,0,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,3,0,0,0,15,224,0,0,24,48,0,0,51,144,0,0,36,72,0,0,40,104,0,0,105,40,0,0,104,104,0,0,38,200,0,0,51,144,0,0,16,16,0,0,24,16,0,0,24,16,0,0,16,16,0,0,49,140,0,1,227,135,224,15,1,128,24,24,0,0,204,51,128,3,228,36,64,2,22,45,99,230,210,41,36,54,146,44,108,18,52,38,200,9,228,19,152,12,8,24,48,7,48,7,224,0,224,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,1,0,0,0,15,224,0,0,24,48,0,0,19,216,0,0,36,104,0,0,44,44,0,0,41,44,0,0,44,40,0,0,38,200,0,0,19,136,0,0,16,16,0,0,24,16,0,0,8,16,0,0,16,16,0,0,49,140,0,7,227,135,224,14,1,128,48,16,0,0,8,39,128,3,228,44,64,2,52,41,103,198,150,105,36,38,146,44,104,22,52,39,200,19,228,19,16,8,136,24,96,6,56,7,192,3,224,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

const byte PROGMEM framest[][128] = {
  {0,3,192,0,0,6,240,0,0,12,48,0,0,12,48,0,0,12,48,0,0,12,48,0,0,15,252,0,0,12,48,0,0,12,60,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,48,0,0,24,24,0,0,48,12,0,0,96,6,0,0,64,2,0,0,192,2,0,0,192,3,0,0,192,3,0,0,64,3,0,0,64,2,0,0,96,6,0,0,96,4,0,0,48,28,0,0,14,112,0,0,7,224,0},
  {0,3,192,0,0,6,240,0,0,12,48,0,0,12,48,0,0,12,48,0,0,12,48,0,0,15,252,0,0,12,48,0,0,12,60,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,48,0,0,24,24,0,0,48,12,0,0,96,6,0,0,64,2,0,0,192,2,0,0,192,3,0,0,192,3,0,0,64,3,0,0,64,2,0,0,96,6,0,0,96,4,0,0,48,28,0,0,14,112,0,0,7,224,0},
  {0,3,192,0,0,6,240,0,0,12,48,0,0,12,48,0,0,12,48,0,0,12,48,0,0,15,252,0,0,12,48,0,0,12,60,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,48,0,0,24,24,0,0,48,12,0,0,96,6,0,0,64,2,0,0,192,2,0,0,192,3,0,0,192,3,0,0,64,3,0,0,64,2,0,0,96,6,0,0,96,4,0,0,48,28,0,0,14,112,0,0,7,224,0},
  {0,3,192,0,0,6,240,0,0,12,48,0,0,12,48,0,0,12,48,0,0,12,48,0,0,15,252,0,0,12,48,0,0,12,60,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,48,0,0,24,24,0,0,48,12,0,0,96,6,0,0,64,2,0,0,192,2,0,0,192,3,0,0,192,3,0,0,64,3,0,0,64,2,0,0,96,6,0,0,96,4,0,0,48,28,0,0,14,112,0,0,7,224,0},
  {0,7,192,0,0,14,224,0,0,12,48,0,0,8,48,0,0,8,48,0,0,15,240,0,0,14,60,0,0,8,48,0,0,8,60,0,0,8,60,0,0,8,48,0,0,8,60,0,0,8,48,0,0,8,60,0,0,8,60,0,0,8,48,0,0,12,60,0,0,12,16,0,0,24,24,0,0,48,12,0,0,96,6,0,0,96,2,0,0,64,3,0,0,192,3,0,0,192,3,0,0,64,3,0,0,64,3,0,0,96,6,0,0,48,6,0,0,56,12,0,0,15,120,0,0,7,224,0},
  {0,7,192,0,0,14,224,0,0,12,32,0,0,8,48,0,0,11,176,0,0,15,240,0,0,8,60,0,0,8,48,0,0,8,60,0,0,8,48,0,0,8,48,0,0,8,60,0,0,8,48,0,0,8,60,0,0,8,60,0,0,8,48,0,0,12,60,0,0,12,16,0,0,24,24,0,0,48,12,0,0,96,6,0,0,96,2,0,0,64,3,0,0,64,1,0,0,64,3,0,0,64,3,0,0,64,3,0,0,96,2,0,0,48,6,0,0,56,12,0,0,15,120,0,0,7,224,0},
  {0,7,192,0,0,14,224,0,0,8,32,0,0,8,32,0,0,15,224,0,0,14,48,0,0,8,60,0,0,8,48,0,0,8,60,0,0,8,48,0,0,8,48,0,0,8,60,0,0,8,48,0,0,8,60,0,0,8,60,0,0,8,48,0,0,12,60,0,0,12,16,0,0,24,24,0,0,48,12,0,0,96,6,0,0,96,3,0,0,64,3,0,0,64,1,0,0,64,1,0,0,64,3,0,0,64,3,0,0,96,2,0,0,48,6,0,0,24,12,0,0,15,120,0,0,3,224,0},
  {0,7,192,0,0,14,224,0,0,12,32,0,0,8,48,0,0,8,48,0,0,8,48,0,0,8,60,0,0,8,48,0,0,15,252,0,0,8,48,0,0,8,48,0,0,8,60,0,0,8,48,0,0,8,60,0,0,8,60,0,0,8,48,0,0,12,60,0,0,12,16,0,0,24,24,0,0,48,12,0,0,96,6,0,0,96,2,0,0,64,3,0,0,64,1,0,0,64,3,0,0,64,3,0,0,64,3,0,0,96,2,0,0,48,6,0,0,56,12,0,0,15,120,0,0,7,224,0},
  {0,7,192,0,0,14,224,0,0,12,48,0,0,8,48,0,0,8,48,0,0,8,48,0,0,8,60,0,0,8,48,0,0,8,60,0,0,8,60,0,0,8,48,0,0,8,60,0,0,8,48,0,0,8,60,0,0,14,252,0,0,15,240,0,0,12,60,0,0,12,16,0,0,24,24,0,0,48,12,0,0,96,6,0,0,96,2,0,0,64,3,0,0,192,3,0,0,192,3,0,0,64,3,0,0,64,3,0,0,96,6,0,0,48,6,0,0,56,12,0,0,15,120,0,0,7,224,0},
  {0,3,192,0,0,6,240,0,0,12,48,0,0,12,48,0,0,12,48,0,0,12,48,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,48,0,0,31,248,0,0,51,204,0,0,96,6,0,0,64,2,0,0,192,2,0,0,192,3,0,0,192,3,0,0,64,3,0,0,64,2,0,0,96,6,0,0,96,4,0,0,48,28,0,0,14,112,0,0,7,224,0},
  {0,3,192,0,0,6,240,0,0,12,48,0,0,12,48,0,0,12,48,0,0,12,48,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,112,0,0,31,248,0,0,48,12,0,0,96,6,0,0,64,2,0,0,192,2,0,0,192,3,0,0,192,3,0,0,64,3,0,0,64,2,0,0,96,6,0,0,96,4,0,0,48,28,0,0,14,112,0,0,7,224,0},
  {0,3,192,0,0,6,240,0,0,12,48,0,0,12,48,0,0,12,48,0,0,12,48,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,60,0,0,12,48,0,0,12,60,0,0,15,240,0,0,24,24,0,0,48,12,0,0,96,6,0,0,64,2,0,0,192,2,0,0,192,3,0,0,192,3,0,0,64,3,0,0,64,2,0,0,96,6,0,0,96,4,0,0,48,28,0,0,14,112,0,0,7,224,0},
  {0,3,192,0,0,6,240,0,0,12,48,0,0,12,48,0,0,12,48,0,0,12,48,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,60,0,0,12,48,0,0,15,252,0,0,15,240,0,0,24,24,0,0,48,12,0,0,96,6,0,0,64,2,0,0,192,2,0,0,192,3,0,0,192,3,0,0,64,3,0,0,64,2,0,0,96,6,0,0,96,4,0,0,48,28,0,0,14,112,0,0,7,224,0},
  {0,3,192,0,0,6,240,0,0,12,48,0,0,12,48,0,0,12,48,0,0,12,48,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,60,0,0,12,48,0,0,15,252,0,0,12,48,0,0,24,24,0,0,48,12,0,0,96,6,0,0,64,2,0,0,192,2,0,0,192,3,0,0,192,3,0,0,64,3,0,0,64,2,0,0,96,6,0,0,96,4,0,0,48,28,0,0,14,112,0,0,7,224,0},
  {0,3,192,0,0,6,240,0,0,12,48,0,0,12,48,0,0,12,48,0,0,12,48,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,60,0,0,12,48,0,0,15,252,0,0,12,48,0,0,24,24,0,0,48,12,0,0,96,6,0,0,64,2,0,0,192,2,0,0,192,3,0,0,192,3,0,0,64,3,0,0,64,2,0,0,96,6,0,0,96,4,0,0,48,28,0,0,14,112,0,0,7,224,0},
  {0,3,192,0,0,6,240,0,0,12,48,0,0,12,48,0,0,12,48,0,0,12,48,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,60,0,0,12,48,0,0,15,252,0,0,12,48,0,0,24,24,0,0,48,12,0,0,96,6,0,0,64,2,0,0,192,2,0,0,192,3,0,0,192,3,0,0,64,3,0,0,64,2,0,0,96,6,0,0,96,4,0,0,48,28,0,0,14,112,0,0,7,224,0},
  {0,3,192,0,0,6,240,0,0,12,48,0,0,12,48,0,0,12,48,0,0,12,48,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,60,0,0,12,48,0,0,15,252,0,0,13,176,0,0,24,24,0,0,48,12,0,0,96,6,0,0,64,2,0,0,192,2,0,0,192,3,0,0,192,3,0,0,64,3,0,0,64,2,0,0,96,6,0,0,96,4,0,0,48,28,0,0,14,112,0,0,7,224,0},
  {0,3,192,0,0,6,112,0,0,12,48,0,0,12,16,0,0,12,16,0,0,12,16,0,0,12,30,0,0,12,16,0,0,12,60,0,0,12,60,0,0,12,16,0,0,12,60,0,0,12,16,0,0,12,48,0,0,12,60,0,0,12,48,0,0,12,124,0,0,15,240,0,0,24,24,0,0,48,12,0,0,96,6,0,0,64,2,0,0,192,2,0,0,192,2,0,0,192,3,0,0,192,2,0,0,64,2,0,0,96,6,0,0,96,4,0,0,48,24,0,0,30,112,0,0,7,224,0},
  {0,3,224,0,0,7,112,0,0,4,48,0,0,12,16,0,0,12,16,0,0,12,16,0,0,12,30,0,0,12,16,0,0,12,28,0,0,12,30,0,0,12,16,0,0,12,30,0,0,12,16,0,0,12,16,0,0,12,60,0,0,12,16,0,0,8,60,0,0,15,240,0,0,25,248,0,0,48,12,0,0,96,6,0,0,192,6,0,0,192,2,0,0,192,2,0,0,128,2,0,0,192,2,0,0,192,2,0,0,64,6,0,0,96,12,0,0,48,24,0,0,30,112,0,0,7,192,0},
  {0,3,224,0,0,7,112,0,0,4,16,0,0,4,16,0,0,12,16,0,0,12,16,0,0,12,30,0,0,12,16,0,0,12,24,0,0,12,30,0,0,12,16,0,0,12,30,0,0,12,16,0,0,12,16,0,0,12,60,0,0,12,16,0,0,8,60,0,0,12,48,0,0,31,248,0,0,48,12,0,0,96,6,0,0,192,6,0,0,192,2,0,0,192,2,0,0,128,2,0,0,192,2,0,0,192,2,0,0,64,6,0,0,96,12,0,0,48,24,0,0,30,112,0,0,7,192,0},
  {0,3,224,0,0,7,112,0,0,4,48,0,0,12,16,0,0,12,16,0,0,12,16,0,0,12,30,0,0,12,16,0,0,12,28,0,0,12,30,0,0,12,16,0,0,12,30,0,0,12,16,0,0,12,16,0,0,15,252,0,0,12,48,0,0,8,60,0,0,8,48,0,0,24,24,0,0,48,12,0,0,96,6,0,0,192,6,0,0,192,2,0,0,192,2,0,0,128,2,0,0,192,2,0,0,192,2,0,0,64,6,0,0,96,12,0,0,48,24,0,0,30,112,0,0,7,192,0},
  {0,3,192,0,0,6,112,0,0,12,48,0,0,12,16,0,0,12,16,0,0,12,16,0,0,12,30,0,0,15,240,0,0,15,124,0,0,12,60,0,0,12,16,0,0,12,60,0,0,12,16,0,0,12,48,0,0,12,60,0,0,12,48,0,0,12,60,0,0,8,48,0,0,24,24,0,0,48,12,0,0,96,6,0,0,64,2,0,0,192,2,0,0,192,2,0,0,192,3,0,0,192,2,0,0,64,2,0,0,96,6,0,0,96,4,0,0,48,24,0,0,30,112,0,0,7,224,0},
  {0,3,192,0,0,6,240,0,0,12,48,0,0,15,240,0,0,15,240,0,0,12,48,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,48,0,0,24,24,0,0,48,12,0,0,96,6,0,0,64,2,0,0,192,2,0,0,192,3,0,0,192,3,0,0,64,3,0,0,64,2,0,0,96,6,0,0,96,4,0,0,48,28,0,0,14,112,0,0,7,224,0},
  {0,3,192,0,0,6,240,0,0,12,48,0,0,15,240,0,0,12,48,0,0,12,48,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,48,0,0,24,24,0,0,48,12,0,0,96,6,0,0,64,2,0,0,192,2,0,0,192,3,0,0,192,3,0,0,64,3,0,0,64,2,0,0,96,6,0,0,96,4,0,0,48,28,0,0,14,112,0,0,7,224,0},
  {0,3,192,0,0,6,240,0,0,12,48,0,0,12,48,0,0,15,176,0,0,15,240,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,48,0,0,24,24,0,0,48,12,0,0,96,6,0,0,64,2,0,0,192,2,0,0,192,3,0,0,192,3,0,0,64,3,0,0,64,2,0,0,96,6,0,0,96,4,0,0,48,28,0,0,14,112,0,0,7,224,0},
  {0,3,192,0,0,6,240,0,0,12,48,0,0,12,48,0,0,12,48,0,0,15,240,0,0,15,252,0,0,12,48,0,0,12,60,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,48,0,0,24,24,0,0,48,12,0,0,96,6,0,0,64,2,0,0,192,2,0,0,192,3,0,0,192,3,0,0,64,3,0,0,64,2,0,0,96,6,0,0,96,4,0,0,48,28,0,0,14,112,0,0,7,224,0},
  {0,3,192,0,0,6,240,0,0,12,48,0,0,12,48,0,0,12,48,0,0,12,48,0,0,15,252,0,0,12,48,0,0,12,60,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,48,0,0,24,24,0,0,48,12,0,0,96,6,0,0,64,2,0,0,192,2,0,0,192,3,0,0,192,3,0,0,64,3,0,0,64,2,0,0,96,6,0,0,96,4,0,0,48,28,0,0,14,112,0,0,7,224,0},
  {0,3,192,0,0,6,240,0,0,12,48,0,0,12,48,0,0,12,48,0,0,12,48,0,0,15,252,0,0,12,48,0,0,12,60,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,60,0,0,12,48,0,0,12,60,0,0,12,48,0,0,24,24,0,0,48,12,0,0,96,6,0,0,64,2,0,0,192,2,0,0,192,3,0,0,192,3,0,0,64,3,0,0,64,2,0,0,96,6,0,0,96,4,0,0,48,28,0,0,14,112,0,0,7,224,0}
};

// Function to draw the paddles for Pong
void drawPaddle(int x, int y) {
  display.fillRect(x, y, 2, paddleHeight, WHITE);
}

// Function to draw the ball
void drawBall(int x, int y) {
  display.fillRect(x, y, 4, 4, WHITE);
}

// Function to move the ball in Pong
void moveBall() {
  display.fillRect(ballX, ballY, 4, 4, BLACK);  // Erase previous ball position
  ballX += ballDirX;
  ballY += ballDirY;

  // Ball collision with top and bottom
  if (ballY <= 0 || ballY >= display.height() - 2) ballDirY = -ballDirY;

  // Ball collision with player paddle
  if (ballX <= 2 && ballY >= playerPaddleY && ballY <= playerPaddleY + paddleHeight) {
    ballDirX = -ballDirX;
  }

  // Ball collision with AI paddle
  if (ballX >= display.width() - 4 && ballY >= aiPaddleY && ballY <= aiPaddleY + paddleHeight) {
    ballDirX = -ballDirX;
  }

  // Reset ball position if it goes out
  if (ballX <= 0 || ballX >= display.width()) {
    ballX = display.width() / 2;  // Reset ball to center
    ballY = display.height() / 2;
    ballDirX = -ballDirX;  // Change direction
  }

  drawBall(ballX, ballY);
}

// Function to initialize the Pong game
void setupGame() {
  ballX = display.width() / 2;
  ballY = display.height() / 2;
  playerPaddleY = display.height() / 2 - paddleHeight / 2;
  aiPaddleY = display.height() / 2 - paddleHeight / 2;
}

// AI paddle control for Pong
void aiPaddleControl() {
  if (ballY > aiPaddleY + paddleHeight / 2 && aiPaddleY < display.height() - paddleHeight) {
    aiPaddleY += 2;
  } else if (ballY < aiPaddleY + paddleHeight / 2 && aiPaddleY > 0) {
    aiPaddleY -= 2;
  }
}

// Function to display the Pong game
void playPongGame() {
  // Move player paddle up or down
  if (paddleMovingUp) {
    playerPaddleY -= paddleSpeed;
    if (playerPaddleY <= 0) {
      playerPaddleY = display.height() - paddleHeight;  // Reset to bottom once it reaches the top
    }
  }

  display.clearDisplay();

  // Draw paddles and ball
  drawPaddle(0, playerPaddleY);
  aiPaddleControl();
  drawPaddle(display.width() - 2, aiPaddleY);
  moveBall();

  display.display();
  delay(40);  // Adjust speed of the game
}

void setup() {
  pinMode(button, INPUT_PULLUP);  // Set button pin as input with pull-up resistor
  pinMode(buzzer, OUTPUT);        // Set buzzer pin as output
  
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
  display.clearDisplay();
}

void drawAnimatedClock(int x, int y) {
  static int frame = 0;
  display.drawBitmap(x, y, frames[frame], FRAME_WIDTH, FRAME_HEIGHT, WHITE);
  frame = (frame + 1) % FRAME_COUNT;
}

void drawTemp(int x, int y) {
  static int frame = 0;
  display.drawBitmap(x, y, framest[frame], FRAME_WIDTH, FRAME_HEIGHT, WHITE);
  frame = (frame + 1) % FRAME_COUNT;
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
  display.setTextSize(1.5);
  display.setTextColor(WHITE);

  drawTemp(95, 5);  

  display.setCursor(0, 10);
  display.print("Temp: ");
  display.setCursor(40, 10);
  display.print(temperature);
  display.setCursor(70, 10);
  display.print((char)247); // Print degree symbol
  display.print("C");

  display.setCursor(0, 25);
  display.print("Humi: ");
  display.setCursor(50, 25);
  display.print(int(humidity));
  display.setCursor(65, 25);
  display.print("%");

  display.setCursor(12, 40);
  display.print(now.toString(buf));

  // Display the formatted time with AM/PM
  display.setCursor(20, 50);
  display.print("Time: ");
  display.setCursor(60, 50);
  display.print(currentTime);

  display.display();
  delay(FRAME_DELAY);  // Small delay to control animation speed
}

void displayPomodoroTimeRemaining() {
  unsigned long elapsedTime = millis() - pomodoroStartTime;
  long remainingTime = pomodoroDuration - elapsedTime; // Ensure this is signed to handle negative times
  
  if (remainingTime <= 0) {
    remainingTime = 0; // Clamp the remaining time to 0 to prevent negative values
    pomodoroActive = false;
    state = false; // Set state to false when time is up
    
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(10, 25);
    display.print("Time's up!");
    display.display();
    
    digitalWrite(buzzer, HIGH);
    delay(1000);
    digitalWrite(buzzer, LOW);

    displayOtherInfo();  // Display other info after Pomodoro ends
    return;
  }

  unsigned long minutesRemaining = remainingTime / 60000;
  unsigned long secondsRemaining = (remainingTime % 60000) / 1000;
  
  // Calculate the progress as a percentage
  float progress = (float)elapsedTime / pomodoroDuration; // Progress fraction (0.0 to 1.0)
  int progressBarWidth = 100;  // Width of the progress bar (max is 100 pixels)
  int progressBarFilled = (int)(progress * progressBarWidth); // Calculate the filled portion of the bar
  
  // Display the remaining Pomodoro time and animation
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 10);
  display.print("Pomodoro Timer Active");
  display.setTextSize(2);
  display.setCursor(40, 28);
  display.print(minutesRemaining);
  display.print("m ");
  display.print(secondsRemaining);
  display.print("s");
  
  // Draw the progress bar
  display.drawRect(10, 55, progressBarWidth, 5, WHITE); // Draw the outline of the bar
  display.fillRect(10, 55, progressBarFilled, 5, WHITE); // Fill the bar based on progress
  
  // Draw the animated clock icon
  drawAnimatedClock(0, 20);  // Display animation during Pomodoro timer

  display.display();
  delay(FRAME_DELAY);  // Small delay to control animation speed
}

void checkButtonPressForPaddle() {
  bool buttonState = digitalRead(button);
  if (buttonState == HIGH) {
    paddleMovingUp = true;
  } else {
    paddleMovingUp = false;
  }
}

void loop() {
  int ldrValue = digitalRead(ldr);

  // Check if the light is not detected (ldr == 1) to activate the Pong game
  if (ldrValue == 1) {
    if (!gameActive) {
      setupGame();  // Initialize the Pong game
    }
    gameActive = true;
  } else {
    gameActive = false;
  }

  // If the Pong game is active and light is not detected
  if (gameActive) {
    checkButtonPressForPaddle();  // Check if the button is pressed to move paddle
    playPongGame();               // Play the Pong game
  } else {
    // Normal mode - Handle button press to toggle Pomodoro timer
    bool buttonState = digitalRead(button);
    if (buttonState == HIGH && lastButtonState == LOW) {  // Button press detected
      state = !state;  // Toggle the state
      if (state) {
        pomodoroStartTime = millis();  // Start the Pomodoro timer
        pomodoroActive = true;
      } else {
        pomodoroActive = false;
        displayOtherInfo();  // Display other info when not in Pomodoro mode
      }
      delay(50);  // Debounce delay
    }

    lastButtonState = buttonState;

    if (pomodoroActive) {
      displayPomodoroTimeRemaining();  // Display Pomodoro timer
    } else {
      displayOtherInfo();  // Display other info (temperature, humidity, time)
    }
  }
}
