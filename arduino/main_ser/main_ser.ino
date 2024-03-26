// EPS32: https://image.dfrobot.com/image/data/DFR0654/pinout.png
// BNO055: https://learn.adafruit.com/adafruit-bno055-absolute-orientation-sensor/pinouts
// Myoware: https://learn.sparkfun.com/tutorials/getting-started-with-the-myoware-20-muscle-sensor-ecosystem/introduction
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

#define BNO055_ADDRESS_A 0x28
#define BNO055_ADDRESS_B 0x29

const int adr_pin = 17;
int semg = 0;                                                 // SEMG sensor value
  
Adafruit_BNO055 bno1 = Adafruit_BNO055(55, BNO055_ADDRESS_A);
Adafruit_BNO055 bno2 = Adafruit_BNO055(56, BNO055_ADDRESS_B); // this sensor should have ADR pin set high


void setup(void) 
{
  /* Force adress change on first BNO055 */
  pinMode(adr_pin, OUTPUT);
  digitalWrite(adr_pin, HIGH);

  
  /* Initialise the sensor */
  if(!bno1.begin() || !bno2.begin())
  {
    while(1);
  }

  // do the calibration...
  // while (!bno1.isFullyCalibrated() || !bno2.isFullyCalibrated()) {}

  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }
  
  delay(1000);
    
  bno1.setExtCrystalUse(true);
  bno2.setExtCrystalUse(true);
}

void loop(void) 
{
  /* Get semg data */
  semg = analogRead(A0);

  /* Get quaternion data */
  imu::Quaternion q1 = bno1.getQuat();
  imu::Quaternion q2 = bno2.getQuat();

  /* Calculate the conjugate of quat1 to get its inverse */
  imu::Quaternion q1Inv = q1.conjugate();

  /* Calculate the relative quaternion by multiplying quat1Inverse with quat2 */
  imu::Quaternion rq = q1Inv * q2;
  
  /* Extract the relative pitch angle from the relative quaternion */
  // float phi = atan2(2 * (rq.w() * rq.x() + rq.y() * rq.z()), 1 - 2 * (rq.x()*rq.x() + rq.y()*rq.y()));
  // float theta = asin(2 * (rq.w() * rq.y() - rq.z() * rq.x()));
  float psi   = atan2(2 * (rq.w() * rq.z() + rq.x() * rq.y()), 1 - 2 * (rq.y()*rq.y() + rq.z()*rq.z()));

  /* Convert the relative pitch angle from radians to degrees */
  // phi = phi * 180.0 / M_PI;
  // theta = theta * 180.0 / M_PI;
  // psi = psi * 180.0 / M_PI;   // Angle de rotation

  /* Print out the relative pitch angle */
  Serial.print(psi, 2);
  Serial.print(" ");
  Serial.println(semg);
  
  delay(10);
}
