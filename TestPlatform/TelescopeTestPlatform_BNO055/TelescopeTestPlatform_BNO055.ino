#include <Adafruit_BNO055.h>
#include <String.h>
#include <Arduino.h>

// Motor steps per revolution. Most steppers are 200 steps or 1.8 degrees/step
#define MOTOR_STEPS 200


// All the wires needed for full motor functionality
#define DIR 10       // Stepper Direction
#define STEP 11      // Stepper Output
#define EN  12       // Output Enable (Active Low)

#define MS1 2     // MS1 -> HIGH, MS2 -> LOW, MS3 -> LOW is half step
#define MS2 3     // MS1 -> LOW, MS2 -> HIGH, MS3 -> LOW is quarter step
#define MS3 4
#define D1 6       // PWM Actuator control signal
#define D2 7       // Disable LOW active HIGH
#define IN1 9      // IN1 and IN2 control Direction of Actuator
#define IN2 8      // IN1 HIGH, IN2 LOW -> one direction
                    // IN1 LOW, IN2 HIGH -> opposite direction

          
#define RATE_I_CONSTANT 10 // The integrating constant for the actuator rate control
#define RATE_P_CONSTANT 75 // The proportional constant for the actuator rate control

// User constants
#define MAXRANGE  300         // in degrees
#define MAXRATE   2           // in degrees/sec
#define MAXAMPL   20          // in Degrees
#define GEAR_RATIO  22        // overall gear ratio of drive train

// Global Variables //

unsigned long Print_time = 0;
bool AllowPrint;

// Stepper Rotation control variables
unsigned long Stepper_time = 0;
float Stepper_dir = true;
float Stepper_range = 0;
bool Stepper_overflow = false;
bool StepperActuator_sync = true; // This will cause the stepper to change direction at the same time as the actuator
                                  // This allows for the largest uninteruppted test interval
                                  
bool Display_rate = false;        // Allows the stepper to be driven faster than HASP for displaying functionality

// Actuator rate control variables
float Prev_angle = 0;
float Angle = 0;
float Rate_error = 0;
float Rate_error_integral;
unsigned long Actuator_time = 0;
float Actuator_rate = 0;
float Actuator_ampl = 0;
bool UpDown = true;


// Setup the BNO055 object
Adafruit_BNO055 imuSensor = Adafruit_BNO055(BNO055_ID,BNO055_ADDRESS_A);
imu::Vector<3> EulerData;

void setup() {
  // Setup output Pins
  pinMode(DIR, OUTPUT);
  pinMode(STEP, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(EN, OUTPUT);
  pinMode(MS1,OUTPUT);
  pinMode(MS2,OUTPUT);
  pinMode(MS3,OUTPUT);
  
  // Enable Serial for Status Updates
  Serial.begin(115200);
  
  // List available commands
  Serial.println("Commands: RANGE: Rotation Range to Rotate Across in Degrees");
  Serial.println("\t\t\t\t\tRATE: Rate of Vertical Oscillation in deg/s");
  Serial.println("\t\t\t\t\tAMPL: Amplitude of Vertical Oscillation in Degrees");
  Serial.println("\t\t\t\t\tRANGE: Range of rotation in Degrees");
  Serial.println("\t\t\t\t\tSYNC: Synchronise the direction change for both tilt and rotation");
  Serial.println("\t\t\t\t\tDISP: Adjust the rotation rate to be 4 times the HASP rate for displaying");
  Serial.println("All Commands are followed by ':Float' where Float is the value to set");
  AllowPrint = true;
  delay(1000);
  
  // Using MS pins set microstep to 1/16th steps
  // The micro step ratio can be used to adjust speed of rotation
  digitalWrite(MS1, HIGH);  
  digitalWrite(MS2, HIGH);  
  digitalWrite(MS3, HIGH);
  
  Stepper_time = millis();

  // Initialize the IMU
  if (!imuSensor.begin())
  {
    Serial.println("No IMU detected");
    while(1);
  }
  
  // Set the clock divider for the stepper PWM
  TCCR2B = TCCR2B & 0b11111000 | 0x03;
}

void loop() {  
  if(AllowPrint)
  {
    Print_time = millis();
    Serial.print("Oscil. Rate: ");
    Serial.print(Actuator_rate);
    Serial.print("\t\t");
    Serial.print("Oscil. Amplitude: ");
    Serial.print(Actuator_ampl);
    Serial.print("\t\t");
    Serial.print("Rotation Range: ");
    Serial.print(Stepper_range);
    Serial.print("\t\t");
    Serial.print("Synchronization: ");
    if (StepperActuator_sync)
    {
      Serial.print("On");
    }
    else
    {
      Serial.print("Off");
    }
    Serial.print("\t\t");
    Serial.print("Display Mode: ");
    if (Display_rate)
    {
      Serial.println("On");
    }
    else
    {
      Serial.println("Off");
    }
    AllowPrint = false;
  }
  driveActuator(Actuator_ampl, Actuator_rate);
  driveStepper(Stepper_range);
    
}

int getActuatorPWM(float driveStrength)
{
  float drivePWM = 0;
  drivePWM = map(driveStrength,0,255,255,0); // map the control loop output to the PWM range
  if (drivePWM > 255)
  {
    drivePWM = 255;
  }
  if(drivePWM < 0)
  {
    drivePWM = 0;
  }
  return int(drivePWM);
}

void driveActuator(int range, float rate)
{
  EulerData = imuSensor.getVector(Adafruit_BNO055::VECTOR_EULER);
  
  Prev_angle = Angle;     // recored the previous angle for rate control
  Angle = EulerData[1];   // in current orientation platform tips in Y
  
  float dt = float(millis() - Actuator_time)/1000.0f; // Find the change in time since the last update in seconds
  Actuator_time = millis();                           // updated the previous time with the cur time for next loop
  float desiredRate = rate;  // gives us desired degrees/sec
  float currentRate = (Angle-Prev_angle)/dt;  // Change in angle divided by change in time to produce deg/sec
  if (currentRate < 0)
  {
    currentRate = currentRate*-1;
  }

  Rate_error_integral += (desiredRate-currentRate)*dt;   // This accumulates the signed rate error to correct the actuator speed, it is tunable with RATE_I_CONSTANT
  Rate_error = RATE_P_CONSTANT*(desiredRate-currentRate);// Calculate the proportional error
  Rate_error += RATE_I_CONSTANT*Rate_error_integral;     // Sum the propotional and integral errors

  if (Rate_error < 0)
  {
    Rate_error = 0;
  }
  else if(Rate_error > 255)
  {
    Rate_error = 255;
  }

  int PWM = getActuatorPWM(Rate_error);    // Convert the accumulated rate error into PWM to drive the actuator
  if (range == 0)
  {
    PWM = 255;
    Rate_error_integral = 0;    // Turn off the actuator and zero the integral term
  }
  
  if(Angle < range/2.0f and UpDown)   // Change direction after reaching lower tilt limit
  {
    analogWrite(D1,PWM);
    digitalWrite(D2,HIGH);
    digitalWrite(IN1,HIGH);
    digitalWrite(IN2,LOW);
  }
  else if(Angle > -1*range/2.0f and !UpDown) // Change direction after reaching upper tilt limit
  {
    analogWrite(D1,PWM);
    digitalWrite(D2,HIGH);
    digitalWrite(IN1,LOW);
    digitalWrite(IN2,HIGH);
  }
  else
  {
    UpDown = !UpDown;
    if (StepperActuator_sync)
    {
      Stepper_dir = !Stepper_dir;   // If sync is enable then change the stepper direction with the tilt table
    }
  }
}

void driveStepper(float range)
{
  if (Display_rate)
  {
    // Set the clock divider for the stepper PWM
    TCCR2B = TCCR2B & 0b11111000 | 0x02;    // Lower the clock divider by a factor of 4 to rotate faster
    range = range/4;
  }
  else
  {
    // Set the clock divider for the stepper PWM
    TCCR2B = TCCR2B & 0b11111000 | 0x03;    // Closest setting to HASP payload rotation rates
  }
  
  if (range == 0)
  {
    digitalWrite(EN,HIGH);    // Disable the stepper if no rotation is wanted
  }
  else
  {
    analogWrite(STEP,125);    // Write a 50% duty cycle for consistant stepping
    digitalWrite(EN,LOW);     // Enable the stepper
    if (!StepperActuator_sync)
    {
      float rotationTime = 8.35f*((range*GEAR_RATIO)/360.0f);   // Calculate the time it will take to travel the given range
    
      if((millis() - Stepper_time) >= rotationTime*1000)      // Wait the calculated time then switch directions
      {
        Stepper_dir = !Stepper_dir;
        Stepper_time = millis();
      }
    }
    
    if (Stepper_dir)
    {
      digitalWrite(DIR,LOW);
    }
    else
    {
      digitalWrite(DIR,HIGH);
    }
  }
}

void serialEvent()
{
  digitalWrite(D2,LOW); // Disable the stepper before changing settings
  char packetId[5];     // Character buffer for incoming serial packets
  int switchVal;
  
  int i = Serial.readBytesUntil(':', packetId, 4);  // read at most 4 bytes until a ':' and put them in the buffer
  
  if(i == 0)
  {
    Serial.println("Serial Error No Valid Data Read, Please Re-enter Values");
  }
  else
  {
    packetId[i] = '\0';
    
    float data = Serial.parseFloat();
    if((String)packetId == (String)"RATE")
    {
      switchVal = 1;
    }
    else if((String)packetId == (String)"AMPL")
    {
      switchVal = 2;
    }
    else if((String)packetId == (String)"RANG")
    {
      switchVal = 3;
    }
    else if((String)packetId == (String)"SYNC")
    {
      switchVal = 4;
    }
    else if((String)packetId == (String)"DISP")
    {
      switchVal = 5;
    }
    else
    {
      Serial.println("Serial Error No Valid Data Read, Please Re-enter Values\n");
      switchVal = 0;
    }
    
    switch(switchVal){
      case 1:
        if(data <= MAXRATE)
        {
          Actuator_rate = data;
          Serial.print("\nRate set to ");
          Serial.println(data);
          Serial.print("\n");
        }
        else
        {
          Actuator_rate = MAXRATE;
          Serial.print("\nMax rate is ");
          Serial.print(MAXRATE);
          Serial.println(" :Rate set to max\n");
        }
        AllowPrint = true;
        delay(100);
        break;
      case 2:
        if(data <= MAXAMPL)
        {
          Actuator_ampl = data;
          Serial.print("\nAmplitude set to");
          Serial.println(data);
          Serial.print("\n");
        }
        else
        {
          Actuator_ampl = MAXAMPL;
          Serial.print("\nMax ampltiude is ");
          Serial.print(MAXAMPL);
          Serial.println(" :Amplitude set to max\n");
          
        }
        AllowPrint = true;
        delay(100);
        break;
     case 3:
        if(data <= MAXRANGE)
        {
          Stepper_range = data;
          Serial.print("\nRange set to: ");
          Serial.println(data);
          Serial.print("\n");
          
        }
        else
        {
          Stepper_range = MAXRANGE;
          Serial.print("\nMax  rotation range is ");
          Serial.print(MAXRANGE);
          Serial.println(" :Range set to max\n");
        }
        AllowPrint = true;
        delay(100);
        break;
     case 4:
        if(data == 0)
        {
          StepperActuator_sync = false;
          Serial.println("\nThe direction changes are no longer synchronous\n");
          
        }
        else
        {
          StepperActuator_sync = true;
          Serial.println("\nThe direction changes are now synchronous\n");
        }
        AllowPrint = true;
        delay(100);
        break;
      case 5:
        if(data == 0)
        {
          Display_rate = false;
          Serial.println("\nDisplay Mode is No longer Active\n");
        }
        else
        {
          Display_rate = true;
          Serial.println("\nDisplay Mode is Now Active\n");
        }
        AllowPrint = true;
        delay(100);
        break;
    }
  }
}


