#include <tizen.h>
#include <service_app.h>
#include "rawsensordata.h"


#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <Ecore.h>

//#include "sensor_params.h"
#include <sensor.h>
#include <device/power.h>
#define LAUNCHER_APP_ID "org.example.ddrbtppg" // an ID of the UI application of our package


#define FONT_SIZE 20
#define SENSOR_FREQ 10
char time_file[256];
bool flag = false; //To check if the data is recorded


float check_count_error_HRM = 0;
float check_count_HRM = 0;
float checking_time = 3; //Checking time duration in seconds
float recording_time = 60; //Recording time duration in seconds


int pointer_PPG = 0;
int pointer_HRM = 0;
int pointer_ACC = 0;
int pointer_GRA = 0;
int pointer_GYR = 0;
int pointer_PRE = 0;

float array_PPG[array_size];
float array_HRM[array_size];
float array_ACCX[array_size];
float array_ACCY[array_size];
float array_ACCZ[array_size];
float array_GRAX[array_size];
float array_GRAY[array_size];
float array_GRAZ[array_size];
float array_GYRX[array_size];
float array_GYRY[array_size];
float array_GYRZ[array_size];
float array_PRES[array_size];

unsigned long long array_ts[array_size];

// -------------------------- Data Type Definitions Start ----------------------------------------

typedef struct _uib_view1_control_context {
	/* add your variables here */

} uib_view1_control_context;

// application data (context) that will be passed to functions when needed
typedef struct appdata
{
    sensor_h sensor; // sensor handle
    sensor_listener_h listener; // sensor listener handle
} appdata_s;


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


// -------------------------- Status Variables Start ----------------------------------------

service_state_t service_state = STOPPED;

sensor_listener_h listener[SENSOR_LAST+1];

unsigned long long fsize=0;

// -------------------------- Status Variables End ----------------------------------------

void update_sensor_current_val(float val, sensor_t type) {

	static FILE* fp = NULL;
	static int16_t read_sensors = 0;

	if (!fp) {
		char fpath[256];
		strcpy(fpath, app_get_data_path());
		strcat(fpath, "ppg_data.csv");
		fp = fopen(fpath, "a");
	}

	if (type == ALL) {
		// resets value
		for (float* p = &all_sensor_current_vals.hr;
				p <= &all_sensor_current_vals.grav_z; ++p) {
			*p = val;
		}
		read_sensors = 0;
	} else {
		float* p = &all_sensor_current_vals.hr;
		p += (int) type;
		*p = val;
		read_sensors |= 1 << ((int) type);
	}
	fsize = ftell(fp) / 1024;
	// append only when file size < 1GB and all sensors data have been updated
	if (fsize < 1 * 1024 * 1024 && (read_sensors == 0x0FFF)) {
		struct sensor_values vals = all_sensor_current_vals;
		fprintf(fp,
				"%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f, %ld\n",
				vals.hr, vals.ppg, vals.acc_x, vals.acc_y, vals.acc_z,
				vals.gyr_x, vals.gyr_y, vals.gyr_z, vals.pres, vals.grav_x,
				vals.grav_y, vals.grav_z, time(NULL));
		read_sensors = 0;
	}
}

unsigned long long get_fileSize(){
	char fpath[256];
	strcpy(fpath, app_get_data_path());
	strcat(fpath, "ppg_data.csv");
	FILE* fp = fopen(fpath, "r");
	if(fp) {
		unsigned long long filesize = ftell(fp);
		fclose(fp);
		return filesize;
	}
	return -1;
}

/* Define callback */
void example_sensor_callback(sensor_h sensor, sensor_event_s *event, void *user_data)
{
    /*
       If a callback is used to listen for different sensor types,
       it can check the sensor type
    */

    sensor_type_e type;
    sensor_get_type(sensor, &type);
    char *formatted_label = (char*)malloc(256 * sizeof(char));
    if (type == SENSOR_HRM) {
		update_sensor_current_val(event->values[0], HEART_RATE);
    }
    if (type == SENSOR_HRM_LED_GREEN) {
		update_sensor_current_val(event->values[0], PPG);
    }
    if (type == SENSOR_ACCELEROMETER) {
		update_sensor_current_val(event->values[0], ACCELEROMETER_X);
		update_sensor_current_val(event->values[1], ACCELEROMETER_Y);
		update_sensor_current_val(event->values[2], ACCELEROMETER_Z);
    }
    if (type == SENSOR_GYROSCOPE) {
		update_sensor_current_val(event->values[0], GYROSCOPE_X);
		update_sensor_current_val(event->values[1], GYROSCOPE_Y);
		update_sensor_current_val(event->values[2], GYROSCOPE_Z);
	}
    if (type == SENSOR_PRESSURE) {
		update_sensor_current_val(event->values[0], PRESSURE);
	}
    if (type == SENSOR_GRAVITY) {
		update_sensor_current_val(event->values[0], GRAVITY_X);
		update_sensor_current_val(event->values[1], GRAVITY_Y);
		update_sensor_current_val(event->values[2], GRAVITY_Z);
	}
}


// ---------------------------- Sensor Utility Functions Declarations Start ------------------------------
Eina_Bool end_sensor(sensor_listener_h listener);

void start_sensor(sensor_type_e sensor_type, void *vc);
// ---------------------------- Sensor Utility Functions Declarations End ------------------------------

void sensor_not_supported(const char* sensor_name){
	//Record an Error if the sensor is not supported, else continue.
	time_t rawtime;
	struct tm * timeinfo;
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );

	FILE * fp = fopen("/opt/usr/media/Downloads/possible_errors.csv","a");
	fprintf (fp, "%s is not available: %s \n", sensor_name, asctime (timeinfo));
	fclose (fp);

	dlog_print(DLOG_ERROR, LOG_TAG, "%s not supported! Service is useless, exiting...", sensor_name);
	service_app_exit();
}

void start_sensors(void *vc);
void stop_sensors();
void pause_sensors(void *vc);

void start_sensors(void *vc){
	if(service_state == RUNNING)
		return;

	for (int i = 0; i <= SENSOR_LAST; i++){
		listener[i] = -1;
	}

	//PPG
	bool supported_PPG = false;
	sensor_type_e sensor_type_PPG = SENSOR_HRM_LED_GREEN;
	sensor_is_supported(sensor_type_PPG, &supported_PPG);
	if (!supported_PPG) {
		sensor_not_supported("PPG");
	} else{
		start_sensor(sensor_type_PPG,vc);
	}

	//HRM
	bool supported_HRM = false;
	sensor_type_e sensor_type_HRM = SENSOR_HRM;
	sensor_is_supported(sensor_type_HRM, &supported_HRM);
	if (!supported_HRM) {
		sensor_not_supported("HRM");
	} else{
		start_sensor(sensor_type_HRM, vc);
	}

	//ACC (x,y,z)
	bool supported_ACC = false;
	sensor_type_e sensor_type_ACC = SENSOR_ACCELEROMETER;
	sensor_is_supported(sensor_type_ACC, &supported_ACC);
	if (!supported_ACC) {
		sensor_not_supported("ACC");
	} else{
		start_sensor(sensor_type_ACC, vc);
	}
	//Gravity (x,y,z)
	bool supported_Gravity = false;
	sensor_type_e sensor_type_Gravity = SENSOR_GRAVITY;
	sensor_is_supported(sensor_type_Gravity, &supported_Gravity);
	if (!supported_Gravity) {
		sensor_not_supported("Gravity");
	} else{
		start_sensor(sensor_type_Gravity, vc);
	}

	//Gyroscope (x,y,z)
	bool supported_Gyro = false;
	sensor_type_e sensor_type_Gyro = SENSOR_GYROSCOPE;
	sensor_is_supported(sensor_type_Gyro, &supported_Gyro);
	if (!supported_Gyro) {
		sensor_not_supported("Gyro");
	} else{
		start_sensor(sensor_type_Gyro, vc);
	}

	//Atmospheric pressure
	bool supported_Pres = false;
	sensor_type_e sensor_type_Pres = SENSOR_PRESSURE;
	sensor_is_supported(sensor_type_Pres, &supported_Pres);
	if (!supported_Pres) {
		sensor_not_supported("Pressure");
	} else{
		start_sensor(sensor_type_Pres, vc);
	}
	service_state = RUNNING;
//	ecore_timer_add(10, pause_sensors, vc);
}

void stop_sensors(){
	if(service_state == STOPPED)
		return;
	for (int i = 0; i <= SENSOR_LAST; i++){
		if(listener[i] != -1){
			end_sensor(listener[i]);
			listener[i] = -1;
		}
	}
	service_state = STOPPED;
}

void pause_sensors(void *vc){
	stop_sensors(vc);
	ecore_timer_add(10, start_sensors, vc);
}

// ---------------------------- Sensor Utility Functions Definitions Start ------------------------------
// stops single sensor
Eina_Bool end_sensor(sensor_listener_h listener){
	// Release all resources.
	sensor_listener_stop(listener);
	sensor_destroy_listener(listener);
	return ECORE_CALLBACK_CANCEL;
}

// starts single sensor
void start_sensor(sensor_type_e sensor_type, void *vc){
	//Set sensors and start recording
	sensor_h sensor;
	sensor_get_default_sensor(sensor_type, &sensor);
	sensor_create_listener(sensor, &listener[sensor_type]);
	sensor_listener_set_event_cb(listener[sensor_type], 1000/SENSOR_FREQ, example_sensor_callback, vc); //25Hz
	sensor_listener_set_option(listener[sensor_type], SENSOR_OPTION_ALWAYS_ON);
	sensor_listener_start(listener[sensor_type]);
	//End the sensors after the "recording time".
	// ecore_timer_add(recording_time,end_sensor,listener);
}
// ---------------------------- Sensor Utility Functions Definitions End ------------------------------




bool service_app_create(void *data)
{
	dlog_print(DLOG_INFO, LOG_TAG, ">>> service_app_create called...");

	start_sensors(data);

	dlog_print(DLOG_INFO, LOG_TAG, "Starting sensor service...");

	return true;
}

void service_app_terminate(void *data)
{
	dlog_print(DLOG_INFO, LOG_TAG, ">>> service_app_terminate called...");
	stop_sensors(data);
}

void service_app_control(app_control_h app_control, void *data)
{
	dlog_print(DLOG_INFO, LOG_TAG, ">>> service_app_control called...");
	char *caller_id = NULL, *action_value = NULL;

	if ((app_control_get_caller(app_control, &caller_id) == APP_CONTROL_ERROR_NONE)
		&& (app_control_get_extra_data(app_control, "service_action", &action_value) == APP_CONTROL_ERROR_NONE))
	{
		dlog_print(DLOG_INFO, LOG_TAG, ">>> service_app_control called...");
		if((caller_id != NULL) && (action_value != NULL)
			&& (!strncmp(caller_id, LAUNCHER_APP_ID, 256))
			&& (!strncmp(action_value,"stop", 256))){
			dlog_print(DLOG_INFO, LOG_TAG, "Stopping Service!");
			free(caller_id);
			free(action_value);
//			service_app_exit();
			stop_sensors(data);
			return;
		}
		else
		{
			dlog_print(DLOG_INFO, LOG_TAG, "Unsupported action! Doing nothing...");
			free(caller_id);
			free(action_value);
			caller_id = NULL;
			action_value = NULL;
		}
	}

}

static void
service_app_lang_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LANGUAGE_CHANGED*/
	return;
}

static void
service_app_region_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

static void
service_app_low_battery(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_BATTERY*/
}

static void
service_app_low_memory(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_MEMORY*/
}

int main(int argc, char* argv[])
{
	// we declare ad as a structure appdata_s defined earlier
	appdata_s ad = {0,};
	// we keep the template code below…
	//(…)
	// …and then modify the line below to pass an address of ad
    //char ad[50] = {0,};
	service_app_lifecycle_callback_s event_callback;
	app_event_handler_h handlers[5] = {NULL, };

	event_callback.create = service_app_create;
	event_callback.terminate = service_app_terminate;
	event_callback.app_control = service_app_control;

	service_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, service_app_low_battery, &ad);
	service_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, service_app_low_memory, &ad);
	service_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, service_app_lang_changed, &ad);
	service_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, service_app_region_changed, &ad);

//	return service_app_main(argc, argv, &event_callback, ad);
	return service_app_main(argc, argv, &event_callback, &ad);
}
