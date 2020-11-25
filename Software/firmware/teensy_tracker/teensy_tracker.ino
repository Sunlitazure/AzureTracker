#include <i2c_t3.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BNO055.h"
#include "utility/imumaths.h"
#include <string.h>

                                    //id, address
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);

#define BUFLEN 64


void processCmd();

char inBuf[BUFLEN], bufHistory[BUFLEN];
int ptr;
sensors_event_t orientationData, // abs euler vector, 360 degree sphere
                angVelocityData, // rotation speed, rad/s
                linearAccelData, // linear acceleration on axis (accel - gravity), m/s^2
                magnetometerData, // mag field strength on axis, uT
                accelerometerData, // acel on axis (accel w/ gravity), m/s^2
                gravityData; // gravity (accel - movement), m/s^2

typedef struct {
  char key[BUFLEN];
  bool (*funct)(const char* argStr);
} Cmd;

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
}

void loop() {
  if(Serial.available() > 0)
  {
    memcpy(bufHistory, inBuf, BUFLEN);
    while(Serial.available() > 0) {
      inBuf[ptr] = Serial.read();

      if( (inBuf[ptr] == '\r') | (inBuf[ptr] == '\n') | (inBuf[ptr] == '\0') )
      {
        inBuf[ptr] = '\0';
        ptr = 0;
        processCmd();
        while(Serial.available() > 0) { Serial.read(); }
      }
      else
      {
        ++ptr;
      }
    }
   
  }

  bno.getEvent(&orientationData, Adafruit_BNO055::VECTOR_EULER);
  bno.getEvent(&angVelocityData, Adafruit_BNO055::VECTOR_GYROSCOPE);
  bno.getEvent(&linearAccelData, Adafruit_BNO055::VECTOR_LINEARACCEL);
  bno.getEvent(&magnetometerData, Adafruit_BNO055::VECTOR_MAGNETOMETER);
  bno.getEvent(&accelerometerData, Adafruit_BNO055::VECTOR_ACCELEROMETER);
  bno.getEvent(&gravityData, Adafruit_BNO055::VECTOR_GRAVITY);

  delay(1000);
}


bool testCmd(const char *argstr)
{
  Serial.println("This Is A Test");
  Serial.println(argstr + 1);
  return 0;
}

bool trackerInfoCmd(const char *argstr)
{
  Serial.println("AzureTracker_Hw1");
  return 0;
}

bool inBufCmd(const char *argstr)
{
  Serial.println(bufHistory);
  return 0;
}


Cmd cmd[] = 
  {
    {"getTrackerInfo", trackerInfoCmd},
    {"test", testCmd,},
    {"inBuf", inBufCmd},
  };


void processCmd()
{
  unsigned int i;
  for(i=0; i<(sizeof(cmd)/sizeof(cmd[0])); ++i)
  {
    if(strncmp(cmd[i].key, inBuf, strlen(cmd[i].key)) == 0)
    {
      cmd[i].funct(inBuf + strlen(cmd[i].key));
      return;
    }
  }

  Serial.println("Invalid Input");
}
