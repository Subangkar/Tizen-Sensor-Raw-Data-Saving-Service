#include "rawsensordata.h"

#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include <sys/types.h>
#include "statvars.h"


char user_id[256];

int trim(char * s);

// reads "tizenID sl" and returns id: "tizenID_sl"
int get_id_from_config(char* config_dir, char* id){
	char filePath[256];
	strcpy(filePath, config_dir);
	if(filePath[strlen(filePath)-1]!='/'){
	strcat(filePath, "/");
	}
	int pathSize = strlen(filePath);
	char* filename = "config.cfg";
	strcpy(filePath+pathSize, filename);
	FILE *config_file = fopen(filePath, "r");
	if(!config_file){
		dlog_print(DLOG_ERROR, LOG_TAG, "Failed to open file: %s", filePath);
		return 0;
	}
	fscanf(config_file,"%s", id);
	fclose(config_file);
	trim(id);
	if(!strlen(id)) return 0;
	return 1;
}

void load_profile_id_from_config(){
	if(!get_id_from_config(app_get_data_path(), user_id)){
		strcpy(user_id, "None"); //USER_ID
	}
	dlog_print(DLOG_INFO, LOG_TAG, "Loaded profile: %s", user_id);
}
