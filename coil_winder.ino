#include <Arduino.h>

#include "clsPCA9555.h"
#include "ST7036.h"
#include <Wire.h>
#include <EEPROM.h>
#include <Bounce2.h>

#define LCD_WIDTH  16
#define LCD_HEIGHT 2
#define PIN_SENSOR 10

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

PCA9555 ioport(0x20);
ST7036 lcd(LCD_HEIGHT, LCD_WIDTH, 0x3E);
Bounce debouncer = Bounce(); 

#define CHAR_UP  (byte(0))
byte up[8] = {
  0b00100,
  0b01110,
  0b11111,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
};

#define CHAR_UPDOWN 1
byte updown[8] = {
  0b00100,
  0b01110,
  0b11111,
  0b00000,
  0b11111,
  0b01110,
  0b00100,
  0b00000
};

#define CHAR_DOWN 2
byte down[8] = {
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b11111,
  0b01110,
  0b00100,
  0b00000
};

const char *button_str[] = {
  "Up",
  "Ok",
  "Right",
  "Left",
  "Down"
};

typedef enum {
	BUTTON_NONE = 0,
	BUTTON_UP,
	BUTTON_OK,
	BUTTON_RIGHT,
	BUTTON_LEFT,
  BUTTON_DOWN,
} button_t;

typedef enum {
  STATE_LOOPS,
  STATE_SPEED,
  STATE_WINDING
} state_t;

state_t state = STATE_LOOPS;
unsigned int coil_loops = 130;
unsigned int speed = 100;

byte button_pressed()
{
  for (uint8_t i = 0; i < 5; i++){
    if (!ioport.digitalRead(i)) {
      delay(200);
      return i + 1;
    }
  }

  return BUTTON_NONE;
}

bool get_touch_state()
{
  debouncer.update();
  return !debouncer.read();
}

void
setup()
{
  Serial.begin(9600);
  Wire.begin();

  debouncer.interval(5);
  debouncer.attach(PIN_SENSOR, INPUT_PULLUP);

  for (uint8_t i = 0; i < 5; i++)
    ioport.pinMode(i, INPUT);

  /* Backlight */
  ioport.pinMode(13, OUTPUT);
  
  ioport.digitalWrite(13, 1);

  lcd.init();
  lcd.setContrast(255);
  lcd.load_custom_character(CHAR_UP, up);
  lcd.load_custom_character(CHAR_DOWN, down);
  lcd.load_custom_character(CHAR_UPDOWN, updown);

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Coil Winder V1");
  delay(2000);
  lcd.clear();
}

void state_loop()
{
  byte button;

  lcd.setCursor(0,0);
  lcd.print("Loop count:");
  lcd.setCursor(1,0);
  lcd.print(coil_loops);

  button = button_pressed();
  switch(button) {
    case BUTTON_UP:
      coil_loops++;
      break;
    case BUTTON_DOWN:
      if (coil_loops > 1)
        coil_loops--;
      break;
    case BUTTON_OK:
      state = STATE_SPEED;
      lcd.clear();
      return;
    default:
      break;
  }
}

void state_speed()
{
  byte button;

  lcd.setCursor(0,0);
  lcd.print("Speed:");
  lcd.setCursor(1,0);
  lcd.print(speed);

  button = button_pressed();
  switch(button) {
    case BUTTON_UP:
      speed++;
      break;
    case BUTTON_DOWN:
      if (speed > 1)
        speed--;
      break;
    case BUTTON_OK:
      state = STATE_WINDING;
      lcd.clear();
      return;
    default:
      break;
  }
}

unsigned int cur_loop = 0;
unsigned int last_state = LOW;

void state_winding()
{
  lcd.setCursor(0,0);
  lcd.print("Winding:");
  lcd.setCursor(1,0);
  lcd.print(cur_loop);
  byte s = debouncer.read();
  if (s == HIGH && last_state == LOW) {
    cur_loop++;
  }
  last_state = s;
  if (cur_loop == coil_loops) {
    state = STATE_LOOPS;
    cur_loop = 0;
  }
}

void loop()
{
  debouncer.update();
  switch(state) {
    case STATE_LOOPS:
      state_loop();
      break;
    case STATE_SPEED:
      state_speed();
      break;
    case STATE_WINDING:
      state_winding();
      break;
  }
}