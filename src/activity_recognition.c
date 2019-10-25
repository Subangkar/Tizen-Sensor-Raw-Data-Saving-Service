#include <rawsensordata.h>


#include <activity_recognition.h>
#include <Ecore.h>

activity_h handles[ACTIVITY_IN_VEHICLE+1];
activity_type_e current_activity = 0;
char activity_names[][2]={
		[ACTIVITY_STATIONARY]="S",
		[ACTIVITY_IN_VEHICLE]="V",
		[ACTIVITY_WALK]="W",
		[ACTIVITY_RUN]="R",
		[0]="N"
};

void start_timed_sensors(void *data);

void
activity_callback(activity_type_e activity, const activity_data_h data,
                          double timestamp, activity_error_e error, void *user_data);

void activity_recognition_start(){
#ifdef DEBUG_ON
	dlog_print(DLOG_INFO, LOG_TAG, ">>> activity_recognition_start called...");
#endif
	activity_create(&handles[ACTIVITY_STATIONARY]);
	activity_start_recognition(handles[ACTIVITY_STATIONARY], ACTIVITY_STATIONARY, activity_callback, NULL);
	activity_create(&handles[ACTIVITY_WALK]);
	activity_start_recognition(handles[ACTIVITY_WALK], ACTIVITY_WALK, activity_callback, NULL);
	activity_create(&handles[ACTIVITY_RUN]);
	activity_start_recognition(handles[ACTIVITY_RUN], ACTIVITY_RUN, activity_callback, NULL);
	activity_create(&handles[ACTIVITY_IN_VEHICLE]);
	activity_start_recognition(handles[ACTIVITY_IN_VEHICLE], ACTIVITY_IN_VEHICLE, activity_callback, NULL);
}

void
activity_callback(activity_type_e activity, const activity_data_h data,
                          double timestamp, activity_error_e error, void *user_data)
{
#ifdef DEBUG_ON
   dlog_print(DLOG_INFO, LOG_TAG, ">>> activity_callback called...");
#endif
   if (error != ACTIVITY_ERROR_NONE) {
	   return;
   }

   int result;
   activity_accuracy_e accuracy;
   result = activity_get_accuracy(data, &accuracy);
#ifdef DEBUG_ON
   dlog_print(DLOG_INFO, LOG_TAG, ">>> activity detected with accuracy: %d...", accuracy);
#endif

   if (accuracy != ACTIVITY_ACCURACY_LOW && activity != current_activity) {
#ifdef DEBUG_ON
	   dlog_print(DLOG_WARN, LOG_TAG, ">>> activity changed...");
#endif
	   current_activity=activity;
	   start_timed_sensors(user_data);
   }
}

void activity_recognition_stop(){
	for (int i = 0; i <= ACTIVITY_IN_VEHICLE; ++i) {
	    activity_stop_recognition(handles[i]);
	    // If the handle will not be used anymore, its resources needs be released explicitly.
	    activity_release(handles[i]);
	}
}
