#include <rawsensordata.h>


#include <curl/curl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <dlog.h>
#include <stdlib.h>

#include <stdio.h>
#include <sys/types.h>

#define SERVER_URL "http://192.168.0.103:8000/"
#define CURL_MAX_TRANS_TIME DATA_FILE_SIZE_AVG
#define CURL_MAX_CONNECT_TIME 10L

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

    dlog_print(DLOG_INFO, LOG_TAG, "Source      : %s\n", filePath);
    dlog_print(DLOG_INFO, LOG_TAG, "Destination : %s\n", server_path);

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
      dlog_print(DLOG_ERROR, LOG_TAG, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
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

void trim(char * s) {
    char * p = s;
    int l = strlen(p);

    while(isspace(p[l - 1])) p[--l] = 0;
    while(* p && isspace(* p)) ++p, --l;

    memmove(s, p, l + 1);
}


// uploads files serially and deletes at once the file is uploaded
int uploadAllFiles(const char* dir){
  char cmd[256];
  sprintf(cmd, "ls -F  %s | grep -Ev '/|@|=|>|\\|' | sed s/*// | grep -E '*.csv'", dir);

  FILE *fileList = popen(cmd, "r");

  char filePath[256];
  strcpy(filePath, dir);
  if(filePath[strlen(filePath)-1]!='/'){
    strcat(filePath, "/");
  }
  int pathSize = strlen(filePath);

  char filename[256];
  while (fgets(filename, 256, fileList) != NULL)
  {
    trim(filename);
    strcpy(filePath+pathSize, filename);
    dlog_print(DLOG_WARN, LOG_TAG, "Uploading %s\n", filePath);
    if(uploadFile(SERVER_URL, filename, filePath)) return 0;
    dlog_print(DLOG_INFO, LOG_TAG, "\"%s\" Uploaded\n", filename);
    sprintf(cmd, "rm %s", filePath);
    system(cmd);
  }
  return 0;
}
