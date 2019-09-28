#include <tizen.h>
//#include <service_app.h>
#include "rawsensordata.h"
#include <stdlib.h>

void start_sensors(void *vc);
void stop_sensors();


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
	stop_sensors();
}

void service_app_control(app_control_h app_control, void *data)
{
	dlog_print(DLOG_INFO, LOG_TAG, ">>> service_app_control called...");
	char *caller_id = NULL, *action_value = NULL;

	if ((app_control_get_caller(app_control, &caller_id) == APP_CONTROL_ERROR_NONE)
		&& (app_control_get_extra_data(app_control, "service_action", &action_value) == APP_CONTROL_ERROR_NONE))
	{
		dlog_print(DLOG_INFO, LOG_TAG, ">>> service_app_control condition entered with caller id %s asking %s ...", caller_id, action_value);
		if((caller_id != NULL) && (action_value != NULL)
			&& (!strncmp(caller_id, LAUNCHER_APP_ID, 256))
			&& (!strncmp(action_value,"stop", 256))){
			dlog_print(DLOG_INFO, LOG_TAG, "Stopping Service!");
			free(caller_id);
			free(action_value);
//			stop_sensors();
			service_app_exit();
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
