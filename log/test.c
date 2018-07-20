#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "log.h"

void* thread_log_write(void* arg)
{
	while(1)
	{
		LOGM_print("***************************");
		LOGE_print("***************************");
		LOGW_print("***************************");
		LOGI_print("***************************");
		LOGD_print("***************************");
		LOGT_print("***************************");
		usleep(200*1000);
	}

	return NULL;
}

int main()
{
	LOG_INIT();

	pthread_t pid;
	pthread_create(&pid, NULL, thread_log_write, NULL);
	while(1)
	{
		LOGM_print("===========================");
		LOGE_print("===========================");
		LOGW_print("===========================");
		LOGI_print("===========================");
		LOGD_print("===========================");
		LOGT_print("===========================");
		usleep(200*1000);
	}
	
	return 0;
}
