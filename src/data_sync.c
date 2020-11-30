#include <rawsensordata.h>


#include <curl/curl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include <sys/types.h>

#ifdef BT_ENABLED
#include "btft.h"
#endif


#define SERVER_URL "http://datalab.buet.io:8001/file/upload/" //"https://bayesbeat.herokuapp.com/file/upload/" //"http://192.168.1.104:8000/" // "http://hr-logger.herokuapp.com/data/" // "http://192.168.1.104:8000/"
#define CURL_MAX_TRANS_TIME DATA_FILE_SIZE_AVG
#define CURL_MAX_CONNECT_TIME 20L

int skip_invalid_file_upload = SKIP_INVALID_FILE_UPLOAD;

void update_last_upload_time();

int uploadAllFiles_Wifi(const char* dir);
#ifdef BT_ENABLED
int uploadAllFiles_Bluetooth(const char* dir);
#endif
const char* get_next_filePath(const char* dir);

char *basename(char const *path)
{
    char *s = strrchr(path, '/');
    if (!s)
        return strdup(path);
    else
        return strdup(s + 1);
}


int deleteFile(char* filePath){
  char cmd[256];
  sprintf(cmd, "rm %s", filePath);
  system(cmd);
#ifdef DEBUG_ON
		dlog_print(DLOG_INFO, LOG_TAG, "Deleted file: %s", filePath);
#endif
  return 0;
}

// returns 0 for success
int uploadFile(const char *server_url, const char *filename, const char* filePath)
{
  CURL *curl;
  CURLcode res;
  struct stat file_info;
  FILE *fd;

  fd = fopen(filePath, "rb"); /* open file to upload */
  if (!fd)
    return 1; /* can't continue */

  /* to get the file size */
  if (fstat(fileno(fd), &file_info) != 0)
    return 1; /* can't continue */

  long response_code=0;

  curl = curl_easy_init();
  if (curl)
  {
    char server_path[strlen(server_url) + strlen(filename) + 1];
    strcpy(server_path, server_url);
    strcat(server_path, filename);

#ifdef DEBUG_ON
//    dlog_print(DLOG_INFO, LOG_TAG, "Source      : %s\n", filePath);
//    dlog_print(DLOG_INFO, LOG_TAG, "Destination : %s\n", server_path);
#endif

    /* upload to this place */
    curl_easy_setopt(curl, CURLOPT_URL, server_path);

    /* tell it to "upload" to the URL */
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

    /* set where to read from (on Windows you need to use READFUNCTION too) */
    curl_easy_setopt(curl, CURLOPT_READDATA, fd);

    /* and give the size of the upload (optional) */
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)file_info.st_size);

    /* complete connection within 10 seconds */
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, CURL_MAX_CONNECT_TIME);

    /* complete within 20 seconds */
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, CURL_MAX_TRANS_TIME);

    /* enable verbose for easier tracing */
//    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    res = curl_easy_perform(curl);
    /* Check for errors */
    if (res != CURLE_OK)
    {
#ifdef DEBUG_ON
      dlog_print(DLOG_ERROR, LOG_TAG, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
#endif
      return 1;
    }
    else
    {
      /* now extract transfer info */
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE , &response_code);
    }
    /* always cleanup */
    curl_easy_cleanup(curl);
  }
  fclose(fd);
  return response_code==200 ? 0:1;
}


// returns 0 for success
int postFile(const char *server_url, const char *filename, const char* filePath, const char* id, const char* timestamp)
{
  CURL *curl;
  CURLcode res;
  long response_code = 0;

  struct curl_httppost *formpost=NULL;
  struct curl_httppost *lastptr=NULL;
  struct curl_slist *headerlist=NULL;
  static const char buf[] = "Expect:";

  curl_global_init(CURL_GLOBAL_ALL);

  /* Fill in the file upload field */
  curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "file",
               CURLFORM_FILE, filePath,
               CURLFORM_END);

  /* Fill in the filename field */
  curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "filename",
               CURLFORM_COPYCONTENTS, filename,
               CURLFORM_END);

  /* Fill in the id field */
  curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "device_id",
               CURLFORM_COPYCONTENTS, id,
               CURLFORM_END);

  /* Fill in the timestamp field */
  curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "timestamp",
               CURLFORM_COPYCONTENTS, timestamp,
               CURLFORM_END);


  curl = curl_easy_init();
  /* initalize custom header list (stating that Expect: 100-continue is not
     wanted */
  headerlist = curl_slist_append(headerlist, buf);
  if(curl) {
    /* what URL that receives this POST */
    curl_easy_setopt(curl, CURLOPT_URL, server_url);
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

    /* complete connection within 10 seconds */
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, CURL_MAX_CONNECT_TIME);

    /* complete within 20 seconds */
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, CURL_MAX_TRANS_TIME);

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    /* Check for errors */
    if (res != CURLE_OK)
    {
#ifdef DEBUG_ON
      dlog_print(DLOG_ERROR, LOG_TAG, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
#endif
      return 1;
    }
    else
    {
      /* now extract transfer info */
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE , &response_code);
    }

    /* always cleanup */
    curl_easy_cleanup(curl);

    /* then cleanup the formpost chain */
    curl_formfree(formpost);
    /* free slist */
    curl_slist_free_all (headerlist);
  }
  return response_code == 200 ? 0:1;
}

// trims trailing wsp inplace and returns updated str_len
int trim(char * s) {
    char * p = s;
    int l = strlen(p);

    while(isspace(p[l - 1])) p[--l] = 0;
    while(*p && isspace(*p)) ++p, --l;

    memmove(s, p, l + 1);
    return l;
}


const char* get_next_filePath(const char* dir){
  static char cmd[256];
  static char filePath[256];
  static char filename[256];

  sprintf(cmd, "ls -F  %s | grep -Ev '/|@|=|>|\\|' | sed s/*// | grep -E '*.csv' | head -n 1", dir);

  strcpy(filePath, dir);
  if(filePath[strlen(filePath)-1]!='/'){
    strcat(filePath, "/");
  }

  FILE *fileList = popen(cmd, "r");
  if (!fileList) return NULL;

  if (fgets(filename, 256, fileList) != NULL && trim(filename))   {
      strcat(filePath, filename);
      return filePath;
  }
  pclose(fileList);
  return NULL;
}

// uploads files serially and deletes at once the file is uploaded
int uploadAllFiles(const char* dir){
  char cmd[256];
#ifdef DEBUG_ON
    dlog_print(DLOG_INFO, LOG_TAG, "Trying to Upload All");
#endif

  if(skip_invalid_file_upload){
    sprintf(cmd, "find %s -maxdepth 1 -type f -name '*.csv' -size -%dk -exec rm -r {} \\;", dir, INVALID_HR_MAX_DURATION+1);
    system(cmd);
  }

#ifdef BT_ENABLED
  uploadAllFiles_Bluetooth(dir);
#else
  uploadAllFiles_Wifi(dir);
#endif

  return 1;
}

// returns 0 for success
int uploadAllFiles_Wifi(const char* dir){
  char cmd[256];
  char filePath[256];
  char filename[256];
  char id[256]="\0", timestamp[256]="\0";

  // sprintf(cmd, "ls -F  %s | grep -Ev '/|@|=|>|\\|' | sed s/*// | grep -E '*.csv'", dir); // grep -Ev used to ignore files in subfolders
  sprintf(cmd, "ls -F %s | grep -Ev '/' | grep -E '*.csv'", dir); //| grep -Ev '/|@|>|\\' | sed s/*// 

  FILE *fileList = popen(cmd, "r");
  if (!fileList) return 1;

  strcpy(filePath, dir);
  if(filePath[strlen(filePath)-1]!='/'){
    strcat(filePath, "/");
  }

  int pathlen = strlen(filePath);

  while (fgets(filename, 256, fileList) != NULL && trim(filename))
  {
    strcpy(filePath+pathlen, filename); // strcat skipped to avoid multiple filename concat on filePath
    sscanf(filename, "_%255[^_]_%255[^.]", id, timestamp);
#ifdef DEBUG_ON
    dlog_print(DLOG_INFO, LOG_TAG, "Uploading %s with id: %s timestamp:%s\n", filePath, id, timestamp);
#endif
    if(*id == '\0' || *timestamp == '\0') continue;
  //  if(uploadFile(SERVER_URL, filename, filePath)) {
    if(postFile(SERVER_URL, filename, filePath, id, timestamp)){
   	pclose(fileList);
   	return 0;
   }
#ifdef DEBUG_ON
    dlog_print(DLOG_INFO, LOG_TAG, "\"%s\" Uploaded\n", filename);
#endif
#ifdef DEBUG_ON
    dlog_print(DLOG_INFO, LOG_TAG, "Deleting \"%s\"\n", filename);
#endif
    deleteFile(filePath);
    *id = '\0';
    *timestamp = '\0';
  }
  pclose(fileList);

  update_last_upload_time();
  return 1;
}

#ifdef BT_ENABLED
int uploadAllFiles_Bluetooth(const char* dir){
  if(find_peers()){
#ifdef DEBUG_ON
    dlog_print(DLOG_INFO, LOG_TAG, "Peer Found Bluetooth\n");
#endif
    const char* filePath;
    if(filePath = get_next_filePath(dir))
      send_file(filePath);
  }
  return 0;
}
#endif
