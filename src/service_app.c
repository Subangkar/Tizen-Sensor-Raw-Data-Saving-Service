#include <tizen.h>
#include "rawsensordata.h"
#include <stdlib.h>
#include <Ecore.h>
#include <device/power.h>

appdata_t appdata;


void start_timed_sensors(void *data);
void stop_sensors();

extern void activity_recognition_start();
extern void activity_recognition_stop();

bool service_app_create(void *data)
{
	device_power_request_lock(POWER_LOCK_CPU, 0);
#ifdef DEBUG_ON
	dlog_print(DLOG_INFO, LOG_TAG, ">>> service_app_create called...");
#endif
//	appdata = *(appdata_t*)data;
	strncpy(appdata.userid, "subangkar", 31);
	appdata.recording_duration=DATA_RECORDING_DURATION;
	appdata.recording_interval=DATA_RECORDING_INTERVAL;

#ifdef DEBUG_ON
	dlog_print(DLOG_INFO, LOG_TAG, ">>> service_app_create %s %u %u...", appdata.userid, appdata.recording_duration, appdata.recording_interval);
#endif
#ifdef DEBUG_ON
	dlog_print(DLOG_INFO, LOG_TAG, "Starting activity recognition...");
#endif
	activity_recognition_start();

	start_timed_sensors(data);
	return true;
}

void service_app_terminate(void *data)
{
#ifdef DEBUG_ON
	dlog_print(DLOG_INFO, LOG_TAG, ">>> service_app_terminate called...");
#endif
	stop_sensors();
	activity_recognition_stop();
}

void service_app_control(app_control_h app_control, void *data)
{
#ifdef DEBUG_ON
	dlog_print(DLOG_INFO, LOG_TAG, ">>> service_app_control called...");
#endif
	char *caller_id = NULL, *action_value = NULL;

	if ((app_control_get_caller(app_control, &caller_id) == APP_CONTROL_ERROR_NONE)
		&& (app_control_get_extra_data(app_control, "service_action", &action_value) == APP_CONTROL_ERROR_NONE))
	{
#ifdef DEBUG_ON
		dlog_print(DLOG_INFO, LOG_TAG, ">>> service_app_control condition entered with caller id %s asking %s ...", caller_id, action_value);
#endif
		if((caller_id != NULL) && (action_value != NULL)
			&& (!strncmp(caller_id, LAUNCHER_APP_ID, 256))
			&& (!strncmp(action_value,"stop", 256))){
#ifdef DEBUG_ON
			dlog_print(DLOG_INFO, LOG_TAG, "Stopping Service!");
#endif
			free(caller_id);
			free(action_value);
//			stop_sensors();
			service_app_exit();
			return;
		}
		else
		{
#ifdef DEBUG_ON
			dlog_print(DLOG_INFO, LOG_TAG, "Unsupported action! Doing nothing...");
#endif
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
