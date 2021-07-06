/*  Arduino Test Track
 *  
 *  A simple turnout with sensors servos and signals usings millis()
 *   
 */

/* definging readable names for the I/O pins */

#define frontSensorPin  2
#define turnoutServoPin  6                //PWM pin
#define turnoutThrowPin 7
#define turnoutClosePin 8
#define frogRelayPin 13
#define signalServoPin 5           //PWM pin
#define divergeSensorPin 4
#define throughSensorPin 3
#define signalGreenPin 10          //PWM pin
#define signalRedPin   9           //PWM pin



/* Setup servo limits */
#define turnoutClosedPosition 40      // angle
#define turnoutThrownPosition 90      // angle
#define dangerPosition  80      // angle
#define proceedPosition 150     // angle
#define turnoutMoveSpeed 30  // [ms] lower number is faster
#define signalThrowSpeed 10   // [ms] lower number is faster


#include<Servo.h>
/* assign servos */
Servo turnoutServo;
Servo signalServo;

byte turnoutPosition   = turnoutClosedPosition;
byte turnoutTarget = turnoutClosedPosition;
byte signalPosition = proceedPosition;
byte signalTarget   = proceedPosition;
unsigned long turnoutMoveDelay, signalThrowDelay;
byte turnoutState = 1;
byte turnoutTransition;
bool turnoutIsClosed = true;



void throwTurnout(){
  turnoutMoveDelay = millis() + turnoutMoveSpeed;
  if (turnoutPosition < turnoutTarget) turnoutPosition++;
  if (turnoutPosition > turnoutTarget) turnoutPosition--;
  turnoutServo.write(turnoutPosition);
}

void moveSignal(){
  signalThrowDelay = millis() + signalThrowSpeed;
  if (signalPosition < signalTarget) signalPosition++;
  if (signalPosition > signalTarget) signalPosition--;
  signalServo.write(signalPosition);
}


// Function to control the two aspect colour signal
void colourSignalDanger(bool danger){
  digitalWrite(signalGreenPin, !danger);
  digitalWrite(signalRedPin, danger);
}


void setup() {
  // put your setup code here, to run once:
  pinMode(divergeSensorPin, INPUT);
  pinMode(throughSensorPin, INPUT);
  pinMode(frontSensorPin, INPUT);
  
  pinMode(turnoutThrowPin, INPUT_PULLUP);
  pinMode(turnoutClosePin, INPUT_PULLUP);
  
  pinMode(frogRelayPin, OUTPUT);
  
  pinMode(signalGreenPin, OUTPUT);
  pinMode(signalRedPin, OUTPUT);

  /* initailise Signal and its position */
  signalServo.attach(signalServoPin);
  signalServo.write(signalPosition);
  
  /* initailise turnout and its position */
  turnoutServo.attach(turnoutServoPin);
  turnoutServo.write(turnoutTarget);
  
  /* initailise colour signal and frog positions */
  colourSignalDanger(true);
  digitalWrite(frogRelayPin, turnoutIsClosed);
  
  Serial.begin(9600);
}

void loop() {
    /* turnout control */
switch(turnoutState) {
  case 1: // Route set to through
    if (digitalRead(turnoutThrowPin) == LOW) turnoutTransition = 12;
  break;

  case 2: // signal to danger
    if (signalPosition == dangerPosition)  turnoutTransition = 23;
  break;

  case 3: // throwing turnout
    if (turnoutPosition == turnoutThrownPosition) {
      turnoutIsClosed = false;
      digitalWrite(frogRelayPin, turnoutIsClosed);
      turnoutTransition = 34;
    } 
  break;

  case 4: // colour signal to danger
    turnoutTransition = 45;
  break;
  
  case 5: // Route set to diverge
    if (digitalRead(turnoutThrowPin) == LOW) turnoutTransition = 56;
  break;

  case 6: // colour signal to danger
    turnoutTransition = 67;
  break;

  case 7: // closing turnout
  if (turnoutPosition == turnoutClosedPosition) {
    turnoutIsClosed = true;
    digitalWrite(frogRelayPin, turnoutIsClosed);
    turnoutTransition = 78;
  }  
  break;

  case 8: // signal to danger
    if (signalPosition == proceedPosition)  turnoutTransition = 81;
  break;
}

switch(turnoutTransition) { 
  case 12: // button pressed
    signalTarget = dangerPosition;
    turnoutTransition = 0;
    turnoutState = 2;
  break;

  case 23: // signal at danger: throw turnout
    turnoutTarget = turnoutThrownPosition;
    turnoutTransition = 0;
    turnoutState = 3;
  break;

  case 34:  // turnout is at thrown: change colour signal
    colourSignalDanger(false);
    turnoutTransition = 0;
    turnoutState = 4;
  break;

  case 45: // colour signal at proceed
    turnoutTransition = 0;
    turnoutState = 5; 
  break;
  
  case 56: // button pressed
    colourSignalDanger(true);
    turnoutTransition = 0;
    turnoutState = 6;
  break;

  case 67: // colour signal at danger
    turnoutTarget = turnoutClosedPosition;
    turnoutTransition = 0;
    turnoutState = 7;
  break;

  case 78:  // turnout is at closed
    signalTarget = proceedPosition;
    turnoutTransition = 0;
    turnoutState = 8;
  break;

  case 81: // signal at proceed
    turnoutTransition = 0;
    turnoutState = 1; 
  break;
}
  if (turnoutPosition != turnoutTarget && millis() > turnoutMoveDelay) throwTurnout();
  if (signalPosition != signalTarget && millis() > signalThrowDelay) moveSignal();

  /* reading sensors */
  if(digitalRead(frontSensorPin) == LOW){
    if (turnoutIsClosed){
      signalTarget = dangerPosition;
    } else {
      colourSignalDanger(true);
    }
  }
  
  if(digitalRead(throughSensorPin) == LOW && turnoutIsClosed) signalTarget = proceedPosition;
  if(digitalRead(divergeSensorPin) == LOW && !turnoutIsClosed) colourSignalDanger(false);

  
}
