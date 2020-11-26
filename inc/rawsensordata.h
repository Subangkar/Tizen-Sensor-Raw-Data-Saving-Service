#ifndef __rawsensordata_H__
#define __rawsensordata_H__

#include <service_app.h>
#include <sensor.h>
#include <stdint.h>


#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
//#define DEBUG_ON
#ifdef DEBUG_ON
#include <dlog.h>
#define LOG_TAG "X_rawsensordata"
#endif
// #define BT_ENABLED

#define LAUNCHER_APP_ID "org.example.drgbtppg" // an ID of the UI application of our package

#define DEVICE_TEST "device_test_id01_1"
#define DEVICE1_ID "device1_r5akbonr70x"
#define DEVICE2_ID "device2_r5ak104tvqe"
#define DEVICE3_ID "device3_r5akcocc7gf"
#define DEVICE4_ID "device4_r5ak104tvqe"
#define DEVICE5_ID "device5_r5ak104tvqe"

#define USER_ID DEVICE3_ID

#define DATA_RECORDING_DURATION (3*60)
#define DATA_RECORDING_INTERVAL (80*60)
#define DATA_FILE_SIZE_AVG DATA_RECORDING_DURATION
#define WAIT_TIME_UPLOAD (180*60)

#define VALID_HR 30
#define INVALID_HR_MAX_DURATION 30
#define NEG_HR_MAX_DURATION 20

#define SKIP_INVALID_FILE_UPLOAD 0

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
