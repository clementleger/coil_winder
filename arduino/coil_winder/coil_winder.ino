#include <Arduino.h>
#include <Bounce2.h>
#include <AccelStepper.h>

#define PIN_SENSOR 11

#define EN_PIN     8
#define DIR_PIN    5
#define STEP_PIN   2

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

Bounce debouncer = Bounce(); 
AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

typedef enum {
  STATE_INIT,
  STATE_LOOPS,
  STATE_SPEED,
  STATE_WINDING
} state_t;

state_t state = STATE_INIT;
unsigned int coil_loops = 130;
unsigned int speed = 2500;

unsigned int cur_loop = 0;
unsigned int last_state = LOW;

void
setup()
{
  Serial.begin(9600);

  pinMode(EN_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  digitalWrite(EN_PIN, 0);
  digitalWrite(DIR_PIN, 0);
  digitalWrite(STEP_PIN, 0);

  stepper.setMaxSpeed(4000);
  stepper.setAcceleration(10);

  stepper.setSpeed(speed);

  debouncer.interval(5);
  debouncer.attach(PIN_SENSOR, INPUT_PULLUP);

  last_state = debouncer.read();

}

void state_winding()
{
  byte s = debouncer.read();
  if (s == LOW && last_state == HIGH) {
    cur_loop++;
    Serial.print("Loop ");
    Serial.println(cur_loop);
  }
  last_state = s;
  if (cur_loop == coil_loops) {
    state = STATE_LOOPS;
    cur_loop = 0;
  }
  stepper.runSpeed();
}

void state_init()
{
  byte s = debouncer.read();
  if (s == LOW && s != last_state) {
    state = STATE_LOOPS;
  }
  last_state = s;
  stepper.runSpeed();
}

void state_read_loop() {
  long loops;

  do {
	Serial.println("Enter loop count:");
	while (Serial.available() == 0 ){
	}
	loops = Serial.parseInt();
  } while (loops == 0);
	Serial.println(loops);
  coil_loops = loops;
  
  state = STATE_SPEED;
}

void state_speed() {
  
  state = STATE_WINDING;
  return;

  do {
  Serial.println("Enter speed:");
	while (Serial.available() == 0 ){
	}
	speed = Serial.parseInt();
  } while (speed == 0);

  stepper.setSpeed(speed);	
  Serial.println("Press a key to start winding");
  while (Serial.available() == 0 ){
  }
  state = STATE_WINDING;
}

void loop()
{
  debouncer.update();
  switch(state) {
    case STATE_INIT:
      state_init();
      break;
    case STATE_LOOPS:
      state_read_loop();
      break;
    case STATE_SPEED:
      state_speed();
      break;
    case STATE_WINDING:
      state_winding();
      break;
  }
}
