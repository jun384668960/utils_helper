#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

#include "log.h"

#ifdef __cplusplus
extern "C"{
#endif

static char 		s_buffer[MAX_LOG_STR_SIZE] = {0};
static int 			g_level = LOG_DEBUG;
static FILE* 		s_fd = NULL;
static pthread_mutex_t s_lock = PTHREAD_MUTEX_INITIALIZER;

static int LOG_WRITE_FILE(char *buf)
{
	if(buf == NULL) return -1;
	
	if(NULL == s_fd)
    {
        s_fd = fopen(LOG_FILE_NAME1, "a");	
    }
    
    if (NULL != s_fd)
    {
        fprintf(s_fd,"%s", buf);
        if (ftell(s_fd) > MAX_LOG_FILE_SIZE)
        {
            fclose(s_fd);
			s_fd = NULL;
            if(rename(LOG_FILE_NAME1, LOG_FILE_NAME2)) 
            {
                remove(LOG_FILE_NAME2);
                rename(LOG_FILE_NAME1, LOG_FILE_NAME2);
            }
        }
        else
        {
            fclose(s_fd);
            s_fd = NULL;
        }
    }

	return 0;
}


void LOG_PRINT(int priority, const char *t,...)
{
	if((priority < LOG_EMERG) || (priority > LOG_TRACE))
		return;

	if(NULL == t) 
		return;

    if(priority <= g_level)
    {
    	struct timeval v;
		gettimeofday(&v, 0);
		struct tm *p = localtime(&v.tv_sec);
		char fmt[MAX_LOG_STR_SIZE] = {0}; //限制t不能太大
		sprintf(fmt, "%04d/%02d/%02d %02d:%02d:%02d.%03d %s %s\n", 1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, (int)(v.tv_usec/1000),
				priority==LOG_TRACE? "TRACE":(priority==LOG_DEBUG? "DEBUG":(priority==LOG_INFO? "!INFO":(priority==LOG_WARN? "!WARN":(priority==LOG_ERR? "ERROR":"EMERG")))), t);

		pthread_mutex_lock(&s_lock);
		
		va_list params;
		va_start(params, t);
		vsnprintf(s_buffer, MAX_LOG_STR_SIZE, fmt, params);
		va_end(params);
		
		// error等级的才写到日志文件中去
		if(priority <= 1)
		{			
	    	LOG_WRITE_FILE(s_buffer);
		}
		
		if(LOG_EMERG == priority)
		{
			printf(RED"%s"NONE, s_buffer);
		}
		if(LOG_ERR == priority)
		{
			printf(LIGHT_RED"%s"NONE, s_buffer);
		}
		if(LOG_WARN == priority)
		{
			printf(YELLOW"%s"NONE, s_buffer);
		}
		if(LOG_INFO == priority)
		{
			printf(LIGHT_CYAN"%s"NONE, s_buffer);
		}
		if(LOG_DEBUG == priority)
		{
			printf(LIGHT_GREEN"%s"NONE, s_buffer);
		}	
		if(LOG_TRACE == priority)
		{
			printf("%s", s_buffer);
		}
		pthread_mutex_unlock(&s_lock);
    }
	
	return;
}

//===============================================================
//初始化
//===============================================================
int  LOG_INIT() 
{
    FILE *fp = NULL;	
	char szCmd[128] = {0};

	//创建日志目录
	memset(szCmd, 0, sizeof(szCmd));
	sprintf(szCmd, "mkdir %s", LOG_FILE_PATH);
    if((fp = popen(szCmd, "r")) == NULL)
    {
        printf("Fail to mkdir %s\n", LOG_FILE_PATH);
        return -1;
    }
	pclose(fp);

	//创建日志文件
	memset(szCmd, 0, sizeof(szCmd));
	sprintf(szCmd, "touch %s", LOG_FILE_NAME1);
    if((fp = popen(szCmd, "r")) == NULL)
    {
        printf("Fail to touch %s\n", LOG_FILE_NAME1);
        return -1;
    }
	pclose(fp);
   
    printf("Syslog mem file system is OK!\n");

	return 0;
}

int LOG_LEVEL_SET(int level)
{
	g_level = level;
	LOGI_print("set log level:%d", g_level);
	return 0;
}

#ifdef __cplusplus
}
#endif

