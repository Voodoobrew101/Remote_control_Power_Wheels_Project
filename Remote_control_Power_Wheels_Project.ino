/**************************************************************************/
/* 
This program is intended to allow 2 inputs and one output for making a remote control power wheels
Input one is a pwm signal from the remote reciever
Input two is variable voltage from a 5v hall sensor pedal. 
  I think my pedal is not great because the voltage range is pretty small.
Output is in the form of a pwm signal that works with RC car speed controllers.

There is a govenor conected to an aux channel that agjust the max speed the pedal will alow.

I use spektrum remotes wich have a signal range of 1100-1900 pwm 
The failsafe for spektrum recievers is 1500 if the remote is not turned on. 
This Sets the Govenor to 50% when the remote is turned off
  The remote off govener speed can be adjused if your esc has a throttle curve.
    Set the 50% mark of the throttle curve to what you want the throttle off govenor to be.

My Forward/Neutral/Reverse switch is 3 position SPDT On-Off-On 
*/
/**************************************************************************/

//ESC & Throttle settings
const int forwardMax = 1900;
const int forwardMin = 1530;  //throttle Deadband start
const int neutral = 1500;     //Center of neutral Deadband
const int reverseMin = 1470;  //throttle Deadband end
const int reverseMax = 1100;
const int myESCpin = 10;

//Remote & Govenor Settings
const int remoteThrottlePin = 5;
const int remoteGovenorPin = 6;
const int framerate = 22000; //in microseconds
const int remoteGovenorHigh = 1900; 
const int remoteGovenorLow = 1100;


//Car, Pedal & Forward/Neutral/Reverse switch settings
const int pedalMin = 210;     //my min pedal raw input is about 200 so I set it slightly higer the reduce false pedal input
const int pedalMax = 380;     //my max pedal input is about 378 
const int pedalGovenor = 100; //percentage
const int pedalPin = A2;
const int forwardPin = 7;
const int reversePin = 8;

int finalpedalInput = 0;
int adjpedalInput = 0;

void setup() {
  Serial.begin(9600);
  pinMode(myESCpin, OUTPUT);
  pinMode(forwardPin, INPUT);
  digitalWrite(forwardPin, HIGH);
  pinMode(reversePin, INPUT);
  digitalWrite(reversePin, HIGH);
}

void loop()   {
  int pwmOutput = neutral;
// Use the Remote input first
  int remoteThrottleInput = pulseIn(remoteThrottlePin, HIGH);
  if (remoteThrottleInput != 0 && remoteThrottleInput <= reverseMin|| remoteThrottleInput >= forwardMin) {
    pwmOutput = remoteThrottleInput;
  }

//  Is car disabled
  else {
    int variableGovenor = pulseIn(remoteGovenorPin, HIGH);
    int variablePedalGovenor = map (variableGovenor, remoteGovenorLow, remoteGovenorHigh, 0, 100);    // map variableGovenor to 1-100
    variablePedalGovenor = constrain(variablePedalGovenor, 0, 100);
    int pedalInput = analogRead(pedalPin);                                    //  = 200-378

// Are we hitting the pedal?
    if (pedalInput > pedalMin){
      pedalInput = constrain (pedalInput, pedalMin, pedalMax);
      adjpedalInput = map(pedalInput, pedalMin, pedalMax, 0, 100);    // map pedal input to 1-100
      finalpedalInput = adjpedalInput * variablePedalGovenor / 100;   // Apply Pedal Govenor to adjpedalInput
      finalpedalInput = constrain (finalpedalInput, 0, 100);

// What direction are we going?
      bool carForward = digitalRead(forwardPin);
      bool carReverse = digitalRead(reversePin);

// Are we going forward?
      if (carForward == 0) {
        pwmOutput = map(finalpedalInput, 0, 100, forwardMin, forwardMax);
      }

// Then are we going backward?
      else if (carReverse == 0) {
        pwmOutput = map(finalpedalInput, 0, 100, reverseMin, reverseMax);
      }
    } 
// Then we should be in neutral
    else {
      pwmOutput = neutral;
    }   
  }
  // Throttle out as pwm
  digitalWrite(myESCpin, HIGH);
  delayMicroseconds(pwmOutput);
  digitalWrite(myESCpin, LOW);
  delayMicroseconds(framerate - pwmOutput);

}
