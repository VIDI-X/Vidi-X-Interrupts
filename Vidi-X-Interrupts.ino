#define LCD // false = serial output, true = LCD output
/*
#if defined(LCD)
  // LCD output
#else
  // serial output
#endif
*/
#if defined(LCD)
  #include "Adafruit_ILI9341.h"
  #include "Adafruit_GFX.h"
  #include <SPI.h>

  #define TFT_CS   5
  #define TFT_DC  21

  Adafruit_ILI9341 display = Adafruit_ILI9341(TFT_CS, TFT_DC);
#endif

struct Button {
  const uint8_t PIN;
  uint32_t numberKeyPresses;
  bool pressed;
};

Button button1 = {32, 0, false};

//variables to keep track of the timing of recent interrupts
unsigned long button_time = 0;
unsigned long last_button_time = 0;
unsigned long ignore_time = 250; //time in miliseconds to ignore new button state

//variables for the timer interrupt
volatile int interruptCounter;
int totalInterruptCounter;
 
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR isr() {
  button_time = millis();
  if (button_time - last_button_time > ignore_time)
  {
    button1.numberKeyPresses++;
    button1.pressed = true;
    last_button_time = button_time;
  }
}

void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  interruptCounter++;
  portEXIT_CRITICAL_ISR(&timerMux);
}

void setup() {
  #if defined(LCD)
    display.begin();
    display.fillScreen(0);
    display.setRotation(3);
    display.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
    display.setTextSize(3);
    display.println("VIDI X");
    display.println("    microcomputer");
    display.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
    display.setTextSize(2);
    display.println("    Interrupt / Button");
    display.println("     bounce - debounce");
    display.println("  -------- demo ---------");    
    display.setTextColor(ILI9341_RED, ILI9341_BLACK);
    display.setTextSize(1);
    display.printf("ignore_time = %u miliseconds\n", ignore_time);
  #else
    Serial.begin(115200);
    Serial.printf("ignore_time = %u miliseconds\n", ignore_time);
  #endif

  pinMode(button1.PIN, INPUT);
  attachInterrupt(button1.PIN, isr, FALLING);

  timer = timerBegin(1, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 60*1000000, true); // ( 60 * 1000000 ) * 80 == every one minute
  timerAlarmEnable(timer);
}

void loop() {
  if (button1.pressed) {
    
    #if defined(LCD)
      display.print("o");
    #else
      Serial.printf("Button has been pressed %u times\n", button1.numberKeyPresses);
    #endif

    button1.pressed = false;
  }

  if (interruptCounter > 0) {

    portENTER_CRITICAL(&timerMux);
    interruptCounter--;
    portEXIT_CRITICAL(&timerMux);
 
    totalInterruptCounter++;
    
    #if defined(LCD)
      display.print("t");
    #else
      Serial.print("An interrupt as occurred. Total number: ");
      Serial.println(totalInterruptCounter);
    #endif

  }
}
