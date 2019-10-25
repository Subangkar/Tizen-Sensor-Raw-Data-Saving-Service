#ifndef __rawsensordata_H__
#define __rawsensordata_H__

#include <dlog.h>
#include <service_app.h>
#include <sensor.h>
#include <stdint.h>

#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define DEBUG_ON
#define LOG_TAG "X_rawsensordata"

#define LAUNCHER_APP_ID "org.example.drgbtppg" // an ID of the UI application of our package

#define DATA_RECORDING_DURATION 30
#define DATA_RECORDING_INTERVAL 90
#define DATA_FILE_SIZE_AVG DATA_RECORDING_DURATION
// -------------------------- Data Type Definitions Start ----------------------------------------

// application data (context) that will be passed to functions when needed
typedef struct
{
    sensor_h sensor; // sensor handle
    sensor_listener_h listener; // sensor listener handle
} appdata_s;

typedef struct appdata_t{
	char userid[31];
	uint16_t recording_duration;
	uint16_t recording_interval;
} appdata_t;


typedef enum {
	HEART_RATE,PPG,ACCELEROMETER_X,ACCELEROMETER_Y,ACCELEROMETER_Z,GYROSCOPE_X,GYROSCOPE_Y,GYROSCOPE_Z,PRESSURE,GRAVITY_X,GRAVITY_Y,GRAVITY_Z,ALL
} sensor_t;

struct sensor_values {
	float hr, ppg, acc_x, acc_y, acc_z, gyr_x, gyr_y, gyr_z, pres, grav_x,
			grav_y, grav_z;
} all_sensor_current_vals;

typedef enum {
	STOPPED, RUNNING
} service_state_t;
// -------------------------- Data Type Definitions End ----------------------------------------

#endif /* __rawsensordata_H__ */
