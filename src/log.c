#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define LOGFILE "gl.log"
 
void LogCreate()
{
    FILE *file = fopen(LOGFILE, "w");
    fclose(file);
}

void Log(char *message)
{
    time_t time_;
    struct tm *date;
    time(&time_);
    date = localtime(&time_);
    FILE *file = fopen(LOGFILE, "a");

    if (file == NULL)
        return;

    printf("%d:%.d:%.d %s\n", (*date).tm_hour, (*date).tm_min, (*date).tm_sec, message);
    fprintf(file, "%d/%d/%d %d:%.d:%.d %s\n",(*date).tm_year+1900, (*date).tm_mon+1, (*date).tm_mday,
        (*date).tm_hour, (*date).tm_min, (*date).tm_sec, message);
    fclose(file);
}
 
void LogErr(char *message)
{
    Log(message);
    exit(0);
}
