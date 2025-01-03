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
  STATE_LOOPS,
  STATE_SPEED,
  STATE_WINDING
} state_t;

state_t state = STATE_LOOPS;
unsigned int coil_loops = 130;
unsigned int speed = 2000;

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

  stepper.setMaxSpeed(2000);
  stepper.setAcceleration(20);

  debouncer.interval(5);
  debouncer.attach(PIN_SENSOR, INPUT_PULLUP);

}

unsigned int cur_loop = 0;
unsigned int last_state = LOW;

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

void state_loop() {
  Serial.println("Enter number of loops [130]:");
  long loops = Serial.parseInt();
  if (loops != 0)
    coil_loops = loops;
  state = STATE_SPEED;
}

void state_speed() {
  Serial.println("Enter speed:");
  long s = Serial.parseInt();
  if (s != 0)
    speed = s;
  stepper.setSpeed(speed);	
  Serial.println("Starting winding");
  state = STATE_WINDING;
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