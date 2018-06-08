#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "utils_log.h"
#include "stream_manager.h"
#include "stream_define.h"

void* shm_stream_read(void* param)
{
	
	shm_stream_t* handle = shm_stream_create("read_th", "stream", STREAM_MAX_USER, STREAM_MAX_FRAMES, STREAM_VIDEO_MAX_SIZE, SHM_STREAM_READ, SHM_STREAM_MALLOC);

	int count = 0;
	while(1)
	{
		unsigned char* data;
		unsigned int length;
		frame_info info;
		unsigned char string[1024] = {0};
		
		int ret = shm_stream_get(handle, &info, &data, &length);
		if(ret != 0)
		{
		}
		else
		{
			LOGI_print("pts:%llu length:%d info=>length:%d", info.pts, length, info.length);
			int cnt = shm_stream_remains(handle);
			LOGI_print("shm_stream_remains cnt:%d", cnt);
			count++;
		}
		if(count > 20)
			break;

		usleep(2*1000);
	}
	shm_stream_destory(handle);
	
	return NULL;
}


int main()
{
	shm_stream_t* handle = shm_stream_create("write", "stream", STREAM_MAX_USER, STREAM_MAX_FRAMES, STREAM_VIDEO_MAX_SIZE, SHM_STREAM_WRITE, SHM_STREAM_MALLOC);
	if(handle == NULL)
	{
		LOGE_print("shm_stream_create error");
		return -1;
	}

	pthread_t pid;
	pthread_create(&pid, NULL, shm_stream_read, NULL);

	int count = 0;
	while(1)
	{
		unsigned char data[1024] = {0};
		unsigned int length;
		sprintf(data, "======================%d", count);
		length = strlen(data);
		
		frame_info info;
		info.length = length;
		info.pts = count;
		
		int ret = shm_stream_put(handle,info,data,length);
		if(ret != 0)
		{
			LOGE_print("shm_stream_put error");
		}
		LOGI_print("data=>string:%s length:%d", data, length);
		LOGI_print("info=>length:%d pts:%d", info.length, info.pts);
		
		count++;
		if(count > 30)
			break;
		
		usleep(1000*1000);
	}
	
	shm_stream_destory(handle);
	return 0;
}
