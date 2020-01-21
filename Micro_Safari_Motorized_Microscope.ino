#include <AccelStepper.h>



// Define some steppers and the pins the will use
AccelStepper stepperX(AccelStepper::DRIVER, 6, 7); //X
AccelStepper stepperY(AccelStepper::DRIVER, 3, 4); //Y

//limit switch pins on arduino
#define XPosLimit 11 //blk,wht wires
#define XNegLimit 12 //blu, brn wires


unsigned long TimeSinceCheck = 0 ;
unsigned long CheckTimeInterval = 100;
unsigned long LastCheckTime = 0;

//For Joystick Use
int Pot_NoMove_Range = 20; //3 is tested functional minimum
int joystickX = (1023 / 2);
int joystickY = (1023 / 2);

int StandardSpeed = 900;
int XStandardSpeed = StandardSpeed;
int YStandardSpeed = StandardSpeed / 3;

long CartridgeWidth = 0;

//how long after interaction the auto program starts
unsigned long AwayTime = 300000;
unsigned long LastUseTime = 0;
unsigned long TimeSinceUse = 0;

//for AutoMoveRandom()
unsigned long PauseTime = 0;
unsigned long PauseInterval = 10000;
unsigned long TimeSincePause = 0;


void setup() {
delay(100);
  stepperX.setMaxSpeed(18000);
  stepperY.setMaxSpeed(12000);
  stepperX.setSpeed(XStandardSpeed);
  stepperY.setSpeed(YStandardSpeed);
  stepperX.setAcceleration(XStandardSpeed * 10);
  stepperY.setAcceleration(YStandardSpeed * 10);

  Serial.begin(9600);
  pinMode(XPosLimit, INPUT_PULLUP);
  pinMode(XNegLimit, INPUT_PULLUP);

  SetHome();
}




void loop() {
  ReadJoystick();
  JoystickMoveWithLimits();
  AutoMoveCheck();
}









//Move stage when user pushes joystick
void JoystickMoveWithLimits() {

  //Joystick right
  if
  ((joystickX > Pot_NoMove_Range) && (joystickX > ((1023 / 2) + Pot_NoMove_Range)) && (digitalRead(XNegLimit) == HIGH)) {
    int customSpeed = joystickX; // Reads the potentiometer
    int newCustomSpeed = map(customSpeed, ((1023 / 2) + Pot_NoMove_Range), 1023, 0, XStandardSpeed);
    stepperX.moveTo(stepperX.currentPosition() - 1);
    stepperX.setSpeed(-newCustomSpeed);
    stepperX.runSpeed();
    LastUseTime = millis();
  }

  //Joystick left
  if
  ((joystickX < ((1023 / 2) - Pot_NoMove_Range)) && (digitalRead(XPosLimit) == HIGH)) {
    int customSpeed = joystickX; // Reads the potentiometer
    int newCustomSpeed = map(customSpeed, 0, ((1023 / 2) - Pot_NoMove_Range), XStandardSpeed, 0);
    stepperX.moveTo(stepperX.currentPosition() + 1);
    stepperX.setSpeed(newCustomSpeed);
    stepperX.runSpeed();
    LastUseTime = millis();
  }

  //Joystick down
  if
  (joystickY > ((1023 / 2) + Pot_NoMove_Range)) {
    int customSpeed = joystickY; // Reads the potentiometer
    int newCustomSpeed = map(customSpeed, ((1023 / 2) + Pot_NoMove_Range), 1023,  0, YStandardSpeed);
    stepperY.moveTo(stepperY.currentPosition() - 1);
    stepperY.setSpeed(-TrueYSpeed(newCustomSpeed));
    stepperY.runSpeed();
    LastUseTime = millis();
  }

  //Joystick up
  if
  (joystickY < ((1023 / 2) - Pot_NoMove_Range)) {
    int customSpeed = joystickY; // Reads the potentiometer
    int newCustomSpeed = map(customSpeed, 0, ((1023 / 2) - Pot_NoMove_Range), YStandardSpeed, 0);
    stepperY.moveTo(stepperY.currentPosition() + 1);
    stepperY.setSpeed(TrueYSpeed(newCustomSpeed));
    stepperY.runSpeed();
    LastUseTime = millis();
  }
}










//read the joystick every n milliseconds based on CheckTimeInterval
void ReadJoystick() {

  TimeSinceCheck = (millis() - LastCheckTime);

  if (TimeSinceCheck > CheckTimeInterval) {


    joystickX = analogRead(A0);
    joystickY = analogRead(A1);
    TimeSinceCheck = 0;
    LastCheckTime = millis();
  }
}










void SetHome() {

  //check to make sure X pos limit isn't already hit, prior to homing, if it is, move left and then rehome
  if ((digitalRead(XPosLimit) != HIGH)) {
    stepperX.moveTo(-2000);
    stepperX.setSpeed(-2700);
    while (stepperX.distanceToGo() != 0) {
      stepperX.runSpeed();
    }
  }
  //home positive limit

  stepperX.moveTo(1000000);
  stepperX.setSpeed(2700);
  while (digitalRead(XPosLimit) == HIGH) {
    //Serial.println(stepperX.speed());
    stepperX.runSpeed();
  }
  stepperX.setCurrentPosition(0);

  //Move to left limit switch to determine CartridgeWidth
  stepperX.moveTo(-1000000);
  stepperX.setSpeed(-2700);
  while (digitalRead(XNegLimit) == HIGH) {
    stepperX.runSpeed();
  }

  CartridgeWidth = -stepperX.currentPosition();
  stepperX.setCurrentPosition(0);
  Serial.println(CartridgeWidth);
  //move back to center after homing
  stepperX.moveTo(CartridgeWidth / 2);
  stepperX.setSpeed(2700);
  while (stepperX.distanceToGo() != 0) {
    stepperX.runSpeed();
  }
}









//move the stage automatically once user is away for a certain amount of time defined by AwayTime
void AutoMoveCheck() {
  TimeSinceUse = (millis() - LastUseTime);
  if (TimeSinceUse > AwayTime) {
    AutoMoveSineKinda();
  }
}








void AutoMoveSineKinda() {
  if ((digitalRead(XNegLimit) == HIGH) && (digitalRead(XPosLimit) == HIGH) && (stepperX.distanceToGo() == 0)) {
    stepperX.moveTo(CartridgeWidth);
    stepperX.setSpeed(200.0);
  }
  if (digitalRead(XNegLimit) == LOW) {
    stepperX.moveTo(CartridgeWidth);
    stepperX.setSpeed(200.0);
    stepperY.move(3);
    stepperY.setSpeed(TrueYSpeed(YStandardSpeed));
    while (stepperY.distanceToGo() != 0) {
      stepperY.runSpeed();
    }
  }
  if (digitalRead(XPosLimit) == LOW) {
    stepperX.moveTo(0);
    stepperX.setSpeed(-200.0);
    stepperY.move(3);
    stepperY.setSpeed(TrueYSpeed(YStandardSpeed));
    while (stepperY.distanceToGo() != 0) {
      stepperY.runSpeed();
    }
  }
  stepperX.runSpeed();
}





/*
  void AutoMoveSine() {
  if ((digitalRead(XNegLimit) == HIGH) && (digitalRead(XPosLimit) == HIGH) && (stepperX.distanceToGo() == 0)) {
    stepperX.moveTo(CartridgeWidth);
    stepperX.setSpeed(200.0);
  }
  stepperY.move(1);
  stepperY.setSpeed(TrueYSpeed(1));
  if (digitalRead(XNegLimit) == LOW) {
    stepperX.moveTo(CartridgeWidth);
    stepperX.setSpeed(200.0);
    }

  if (digitalRead(XPosLimit) == LOW) {
    stepperX.moveTo(0);
    stepperX.setSpeed(-200.0);

  }
  stepperX.runSpeed();
  stepperY.runSpeed();
  }
*/







//move the stage to a random location every n seconds
void AutoMoveRandomRetry() {
  TimeSincePause = millis() - PauseTime;
    //choose a new location if waited for long enough
  if ((TimeSincePause > PauseInterval)&&(stepperX.distanceToGo()==0)&&(stepperY.distanceToGo()==0)) {
    stepperX.moveTo(random(0, CartridgeWidth));
    stepperY.move(random(-255, 255));   
    PauseTime = millis(); 
  }
  if(stepperX.distanceToGo()>0){
  stepperX.setSpeed((stepperX.distanceToGo() / 5));
  }
    if(stepperX.distanceToGo()<0){
  stepperX.setSpeed(-(stepperX.distanceToGo() / 5));
  }
    if(stepperY.distanceToGo()>0){
  stepperY.setSpeed((stepperY.distanceToGo()));
  }
    if(stepperY.distanceToGo()<0){
  stepperY.setSpeed(-(stepperY.distanceToGo()));
  }

  stepperX.runSpeed();
  stepperY.runSpeed();


}










/*
  //move the stage to a random location every n seconds
  void AutoMoveRandom() {
  //choose a new location if waited for long enough
  if (TimeSincePause > PauseInterval) {
    stepperX.moveTo(random(0, CartridgeWidth));
    stepperY.move(random(-255, 255));
    //if statements calculate the x and y speeds needed to get to position at same time.
    if (stepperX.distanceToGo() > 0) {
      stepperX.maxSpeed((StandardSpeed * stepperY.distanceToGo())*sqrt((stepperX.distanceToGo() ^ 2) + (stepperY.distanceToGo() ^ 2)));
    }
    if (stepperX.distanceToGo() < 0) {
      stepperX.maxSpeed(-(StandardSpeed * stepperY.distanceToGo())*sqrt((stepperX.distanceToGo() ^ 2) + (stepperY.distanceToGo() ^ 2)));
    }
    if (stepperY.distanceToGo() > 0) {
      stepperY.maxSpeed((StandardSpeed * stepperX.distanceToGo())*sqrt((stepperY.distanceToGo() ^ 2) + (stepperX.distanceToGo() ^ 2)));
    }
    if (stepperY.distanceToGo() < 0) {
      stepperY.maxSpeed(-(StandardSpeed * stepperX.distanceToGo())*sqrt((stepperY.distanceToGo() ^ 2) + (stepperX.distanceToGo() ^ 2)));
    }
    PauseTime = millis();
  }

  stepperX.run();
  stepperY.run();
  TimeSincePause = millis() - PauseTime;

  }

*/









//input speed for Y axis, output true speed based on radially how far from the center of the circle the X axis is.
//output_speed == input_speed at outermost viewable diameter of cartridge (i.e: current position == 11304 steps)
int TrueYSpeed(int input_speed) {
  int output_speed = (input_speed * (12890 / (1586 + (11304 - stepperX.currentPosition()))));
  return output_speed;
}

/*
  int TrueYSpeedmod(int input_speed) {
  int output_speed = (input_speed + (input_speed / (12890 / (1586 + stepperX.currentPosition()))));
  //Serial.println(150 *((1586+stepperX.currentPosition())/12890));
  return output_speed;
  }
*/

