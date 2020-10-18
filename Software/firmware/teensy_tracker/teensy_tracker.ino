#include <i2c_t3.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BNO055.h"
#include "utility/imumaths.h"

                                    //id, address
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);

#define BUFLEN 64

char inBuf[BUFLEN];
int ptr;

void setup()
{
  Serial.begin(115200);
  Serial.println("begin test");

  // Initialize sensor
  if(!bno.begin())
  {
    Serial.print("BNO055 not detected");
    while (1);

    delay(1000);
  }

  ptr = 0;
}

void loop() {

  while(Serial.available() > 0) {
    inBuf[ptr] = Serial.read();
    ++ptr;
  }
  ptr = 0;

  processCmd();

  sensors_event_t orientationData, // abs euler vector, 360 degree sphere
                  angVelocityData, // rotation speed, rad/s
                  linearAccelData, // linear acceleration on axis (accel - gravity), m/s^2
                  magnetometerData, // mag field strength on axis, uT
                  accelerometerData, // acel on axis (accel w/ gravity), m/s^2
                  gravityData; // gravity (accel - movement), m/s^2

  bno.getEvent(&orientationData, Adafruit_BNO055::VECTOR_EULER);
  bno.getEvent(&angVelocityData, Adafruit_BNO055::VECTOR_GYROSCOPE);
  bno.getEvent(&linearAccelData, Adafruit_BNO055::VECTOR_LINEARACCEL);
  bno.getEvent(&magnetometerData, Adafruit_BNO055::VECTOR_MAGNETOMETER);
  bno.getEvent(&accelerometerData, Adafruit_BNO055::VECTOR_ACCELEROMETER);
  bno.getEvent(&gravityData, Adafruit_BNO055::VECTOR_GRAVITY);


  delay(1000);
}




void processCmd()
{
  int i;
  for(i=0; i < BUFLEN; ++i)
  {
    
  }
}
