#include "pidf/pidf.h"


PIDF::PIDF(double k[4]) {
  kP = k[0];
  kI = k[1];
  kD = k[2];
  kF = k[3];
}

void PIDF::changePID(double k[4]) {
  kP = k[0];
  kI = k[1];
  kD = k[2];
  kF = k[3];

}
    
void PIDF::updatePIDValsSingle(double actual) {

  error = desiredValue - actual;
      
  //Derivative
  derivative = error - prevError;

  //Integral
  totalError += error;

  //Derivative --> set prevError
  prevError = error;
}

void PIDF::updatePIDValsPoint(Point pos) {

  errorX = desiredPoint.x - pos.x;
  errorY = desiredPoint.y - pos.y; 
      
      

  // //Derivative
  // derivative = error - prevError;

  // //Integral
  // totalError += error;

  // //Derivative --> set prevError
  // prevError = error;
}

double PIDF::calculateErrorPower(){
  return (error * kP) + (totalError * kI) + (derivative * kD);
}

double PIDF::calculateErrorXPower(){
  return (errorX * kP) + (totalError * kI) + (derivative * kD);
}

double PIDF::calculateErrorYPower(){
  return (errorY * kP) + (totalError * kI) + (derivative * kD);
}

double PIDF::calculateForwardPower(double targetVal){
  return (targetVal * kF);
}

void PIDF::resetPID() {
  error = 0, errorX = 0, errorY = 0; //SensorValue - DesiredValue : Position
  prevError = 0; //Position 20ms ago
  derivative = 0; //error - prevError : Speed
  totalError = 0;
}


int driverOnlyPID(){

  while(enableDriverPID) {

    //Reset values switch
    if (resetDriveSensors) {
      resetDriveSensors = false;
      LeftDrive.setPosition(0, degrees);
      RightDrive.setPosition(0, degrees);
    }

    //===============================================================
    //------------LATERAL PID------------//

  
    int averagePosition = (LeftDrive.position(degrees)); //averages motor position of the 2 drive
    driveLVals.desiredValue = desiredMotorVal;
    driveRVals.desiredValue = desiredMotorVal;
    
    driveLVals.updatePIDValsSingle(LeftDrive.position(degrees)); //passes motor encoder val
    driveRVals.updatePIDValsSingle(RightDrive.position(degrees)); //passes motor encoder val



    double lateralLMotorPower = driveLVals.calculateErrorPower(); 
    double lateralRMotorPower = driveRVals.calculateErrorPower(); 

    //===============================================================
        


    //===============================================================
    //-------------TURN PID-------------//

    double actualHeading = getInertialReading();
    double turnMotorPower;

    //inertial sensor switch --> inertial must be turned off when using vision
    turnVals.updatePIDValsSingle(actualHeading); //passes inertial sensor val
    turnMotorPower = turnVals.calculateErrorPower(); 


    //===============================================================

    SpinDrive(lateralRMotorPower - turnMotorPower, lateralLMotorPower + turnMotorPower);


    Controller1.Screen.clearScreen();
    Controller1.Screen.setCursor(1,1);
    Controller1.Screen.print("mpos %f, %f", LeftDrive.position(deg), RightDrive.position(deg));
    Controller1.Screen.newLine();
    Controller1.Screen.print("Ine: %f", actualHeading);
    Controller1.Screen.newLine();


    vex::task::sleep(20);
  }

  return 1;
}