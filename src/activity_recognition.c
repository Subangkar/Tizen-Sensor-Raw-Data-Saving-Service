#include <activity_recognition.h>
#include <Ecore.h>

activity_h handles[ACTIVITY_IN_VEHICLE+1];
activity_type_e current_activity = ACTIVITY_STATIONARY;


void start_timed_sensors(void *data);

void
activity_callback(activity_type_e activity, const activity_data_h data,
                          double timestamp, activity_error_e error, void *user_data);

void activity_recognition_start(){
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
   int result;
   activity_accuracy_e accuracy;

   if (error != ACTIVITY_ERROR_NONE) {
	   return;
   }

   result = activity_get_accuracy(data, &accuracy);

   if (accuracy != ACTIVITY_ACCURACY_LOW && activity != current_activity) {
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
