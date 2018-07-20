#ifndef   _LOG_H_
#define   _LOG_H_

#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h>
#include <stdlib.h>

//宏定义
#define MAX_LOG_STR_SIZE	1024
#define MAX_LOG_FILE_SIZE	100*1024
#define LOG_FILE_PATH 		"./"
#define LOG_FILE_NAME1		LOG_FILE_PATH"syslog"
#define LOG_FILE_NAME2		LOG_FILE_PATH"syslog2"

#define   LOG_EMERG   0
#define   LOG_ERR     1
#define   LOG_WARN    2
#define   LOG_INFO    3
#define   LOG_DEBUG   4
#define   LOG_TRACE   5

//打印字体颜色
#define NONE         "\033[m" 
#define RED          "\033[0;32;31m" 
#define LIGHT_RED    "\033[1;31m" 
#define GREEN        "\033[0;32;32m" 
#define LIGHT_GREEN  "\033[1;32m" 
#define BLUE         "\033[0;32;34m" 
#define LIGHT_BLUE   "\033[1;34m" 
#define DARY_GRAY    "\033[1;30m" 
#define CYAN         "\033[0;36m" 
#define LIGHT_CYAN   "\033[1;36m" 
#define PURPLE       "\033[0;35m" 
#define LIGHT_PURPLE "\033[1;35m" 
#define BROWN        "\033[0;33m" 
#define YELLOW       "\033[1;33m" 
#define LIGHT_GRAY   "\033[0;37m" 
#define WHITE        "\033[1;37m"

//系统日志相关函数
int  LOG_INIT();
int  LOG_LEVEL_SET(int level);
void LOG_PRINT(int priority, const char *t,...);

#ifdef LOG
#define LOGT_print(t, ...)	LOG_PRINT(LOG_TRACE, "[%s][%04d]"t"", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOGD_print(t, ...) 	LOG_PRINT(LOG_DEBUG, "[%s][%04d]"t"", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOGI_print(t, ...) 	LOG_PRINT(LOG_INFO,  "[%s][%04d]"t"", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOGW_print(t, ...)	LOG_PRINT(LOG_WARN,  "[%s][%04d]"t"", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOGE_print(t, ...) 	LOG_PRINT(LOG_ERR,   "[%s][%04d]"t"", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOGM_print(t, ...) 	LOG_PRINT(LOG_EMERG, "[%s][%04d]"t"", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define LOGT_print(t, ...)
#define LOGD_print(t, ...)
#define LOGI_print(t, ...)
#define LOGW_print(t, ...)
#define LOGE_print(t, ...)
#define LOGM_print(t, ...)
#endif

#ifdef __cplusplus
}
#endif
#endif

