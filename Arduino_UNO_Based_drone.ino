/*
  DIY Mini Drone Altitude and Speed Control using Potentiometer and HC-SR04
  Modifications: Altitude control adjusted for max potentiometer value.
*/

// define global variables

// constants that don't change (the variables are read-only)
const int triggerPin = 8;        // trigger pin for the ultrasonic sensor
const int echoPin = 7;
const int PWMPin = 10;           // pin for the motor PWM signal
const int min_height = 10;        // minimum desired height in centimeters
const int max_height = 150;       // maximum desired height in centimeters 
const float K = 3;               // proportional controller gain
const int ramp_time = 4;        // delay for motor ramp up/ramp down 

// speed control pins from user's code
const int motor_pin = 11;        // pin for enabling motor control
const int pot = A0;              // pin for potentiometer input (speed control)

// variables that can change
int target_dist = 0;             // target distance in centimeters (set with potentiometer)
int pot_reading = 0;             // potentiometer reading for height control
int speed_pot_reading = 0;       // potentiometer reading for speed control
long duration;                   // duration of the ultrasonic ping
long cm;                         // measured distance in centimeters
int PWM_offset = 150;            // offset for PWM signal (roughly makes the drone hover)
int PWM_signal = 0;              // PWM value
int error = 0;                   // difference between target distance and measured distance
int motor_speed = 0;             // mapped speed control based on the second potentiometer

void setup() {    // setup code that only runs once
  pinMode(triggerPin, OUTPUT);    // set ultrasonic trigger pin as output
  pinMode(echoPin, INPUT);        // set ultrasonic echo pin as input
  pinMode(motor_pin, OUTPUT);     // set motor pin as output
  Serial.begin(9600);             // initialize serial communication for debugging
  LiftOff();                      // give drone a short boost to prevent bad sensor readings
  delay(200);                     // wait briefly before entering loop
}

void loop() {

  // Trigger the ultrasonic sensor
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(triggerPin, LOW);

  // Measure duration of echo pulse
  duration = pulseIn(echoPin, HIGH);

  // Convert the time into a distance in centimeters
  cm = microsecondsToCentimeters(duration);

  // Altitude control using first potentiometer
  pot_reading = analogRead(pot);                                  // read potentiometer for altitude control
  target_dist = map(pot_reading, 0, 1023, min_height, max_height); // map the potentiometer reading to a height
  error = target_dist - cm;                                       // calculate difference between target height and actual height
  PWM_signal = K * error + PWM_offset;                            // proportional controller for altitude
  if(PWM_signal > 255) {
    PWM_signal = 255;                                             // limit PWM signal
  } else if(PWM_signal < 0) {
    PWM_signal = 0;
  }
  
  // Motor speed control using second potentiometer
  speed_pot_reading = analogRead(A1);                // read from another potentiometer (speed control)
  motor_speed = map(speed_pot_reading, 0, 1023, 0, 255); // map potentiometer reading to motor speed
  
  // Send PWM signals for both altitude control and speed control
  analogWrite(PWMPin, PWM_signal);    // altitude control
  analogWrite(motor_pin, motor_speed); // motor speed control

  // Debugging information
  Serial.print("Target distance: ");
  Serial.println(target_dist);
  Serial.print(" Measured distance: ");
  Serial.println(cm);
  Serial.print(" Error: ");
  Serial.println(error);
  Serial.print(" Altitude PWM: ");
  Serial.println(PWM_signal);
  Serial.print(" Motor Speed PWM: ");
  Serial.println(motor_speed);
}

long microsecondsToCentimeters(long microseconds) {
  return microseconds / 29 / 2;  // convert echo time to centimeters
}

void LiftOff() {         // slowly ramp up motor speed to lift off
  while (PWM_signal < PWM_offset) {
    analogWrite(PWMPin, PWM_signal);
    PWM_signal++;
    delay(ramp_time);
  }
}
