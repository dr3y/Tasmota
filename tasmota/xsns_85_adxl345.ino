/*
  xsns_32_mpu6050.ino - MPU6050 gyroscope and temperature sensor support for Tasmota

  Copyright (C) 2021  Andrey Shur

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef USE_I2C
#ifdef USE_ADXL345
/*********************************************************************************************\
 * ADXL345 3 axis gyroscope and temperature sensor
 *
 * Source: Oliver Welter, with special thanks to Jeff Rowberg
 *
 * I2C Address: 0x68 or 0x69 with AD0 HIGH
\*********************************************************************************************/

#define XSNS_85                          85
#define XI2C_25                          25  // See I2CDEVICES.md

#define ADXL345                          "ADXL345"

#include "Adafruit_Sensor.h"
#include "Adafruit_ADXL345_U.h"
Adafruit_ADXL345_Unified adxl345 = Adafruit_ADXL345_Unified(12345);

#define ADXL345_ADDR            0x53

struct {
  uint16_t ax = 0;
  uint16_t ay = 0;
  uint16_t az = 0;
  bool ready = false;
} adxl345_sensors;


/********************************************************************************************/
void adxl345Detect(void){
  if (!I2cSetDevice(ADXL345_ADDR)) { return; }
  if (!adxl345.begin()) { return; }

  I2cSetActiveFound(ADXL345_ADDR, "ADXL345");
  adxl345.setRange(ADXL345_RANGE_2_G);
  adxl345.setDataRate(ADXL345_DATARATE_1_56_HZ);
  adxl345_sensors.ready = true;
}

void adxl345PerformReading(void)
{
  sensors_event_t event;
  adxl345.getEvent(&event);
  adxl345_sensors.ax = event.acceleration.x;
  adxl345_sensors.ay = event.acceleration.y;
  adxl345_sensors.az = event.acceleration.z;
}


#define D_YAW "Yaw"
#define D_PITCH "Pitch"
#define D_ROLL "Roll"

#ifdef USE_WEBSERVER
const char HTTP_SNS_AXIS[] PROGMEM =
  "{s}" ADXL345 " " D_AX_AXIS "{m}%s{e}"                              // {s} = <tr><th>, {m} = </th><td>, {e} = </td></tr>
  "{s}" ADXL345 " " D_AY_AXIS "{m}%s{e}"                              // {s} = <tr><th>, {m} = </th><td>, {e} = </td></tr>
  "{s}" ADXL345 " " D_AZ_AXIS "{m}%s{e}";                              // {s} = <tr><th>, {m} = </th><td>, {e} = </td></tr>

#endif // USE_WEBSERVER

#define D_JSON_AXIS_AX "AccelXAxis"
#define D_JSON_AXIS_AY "AccelYAxis"
#define D_JSON_AXIS_AZ "AccelZAxis"

void adxl345Show(bool json)
{
  adxl345PerformReading();

  char axis_ax[33];
  dtostrfd(adxl345_sensors.ax, Settings.flag2.axis_resolution, axis_ax);
  char axis_ay[33];
  dtostrfd(adxl345_sensors.ay, Settings.flag2.axis_resolution, axis_ay);
  char axis_az[33];
  dtostrfd(adxl345_sensors.az, Settings.flag2.axis_resolution, axis_az);

  if (json) {
    char json_axis_ax[25];
    snprintf_P(json_axis_ax, sizeof(json_axis_ax), PSTR(",\"" D_JSON_AXIS_AX "\":%s"), axis_ax);
    char json_axis_ay[25];
    snprintf_P(json_axis_ay, sizeof(json_axis_ay), PSTR(",\"" D_JSON_AXIS_AY "\":%s"), axis_ay);
    char json_axis_az[25];
    snprintf_P(json_axis_az, sizeof(json_axis_az), PSTR(",\"" D_JSON_AXIS_AZ "\":%s"), axis_az);

#ifdef USE_WEBSERVER
  } else {
    WSContentSend_PD(HTTP_SNS_AXIS, axis_ax, axis_ay, axis_az);
#endif // USE_WEBSERVER
  }
}

/*********************************************************************************************\
 * Interface
\*********************************************************************************************/

bool Xsns85(uint8_t function)
{
  if (!I2cEnabled(XI2C_25)) { return false; }

  bool result = false;

  if (FUNC_INIT == function) {
    adxl345Detect();
  }
  else if (adxl345_sensors.ready) {
    switch (function) {
      case FUNC_EVERY_SECOND:
          adxl345PerformReading();
        break;
      case FUNC_JSON_APPEND:
        adxl345Show(1);
        break;
#ifdef USE_WEBSERVER
      case FUNC_WEB_SENSOR:
        adxl345Show(0);
        adxl345PerformReading();
        break;
#endif // USE_WEBSERVER
    }
  }
  return result;
}

#endif // USE_ADXL345
#endif // USE_I2C
