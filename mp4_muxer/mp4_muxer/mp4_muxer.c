#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef WIN32
#include <./win/unistd.h>
#else
#include <unistd.h>
#endif
#include <fcntl.h>
#include <errno.h>

#include "file_sink.h"
#include "mp4_builder.h"
#include "mp4_muxer.h"

int          mEventMap;
int          mFileCounter;
int          mEventFileCounter;
//++char        *mpFileName;
//++char        *mpTmpName;
//++char        *mpPathName;
//++char        *mpBaseName;

#ifndef EXTRA_NAME_LEN
#define EXTRA_NAME_LEN 60
#endif
extern AM_VIDEO_INFO mH264Info;

ERR SetMediaSink (const char *destStr); 
ERR CreateNextSplitFile (const char *pFileName);   

ERR SetMediaSink (const char *pFileName)
{
	if (pFileName == NULL) 
	{
	  	return ME_ERROR;
	}

	return CreateNextSplitFile(pFileName);
}

ERR CreateNextSplitFile (const char *pFileName)
{
	/* Mp4 muxer doesn't begin to run. */

	if (Mp4_CreateFile(pFileName) != ME_OK) 
	{
		return ME_ERROR;
	}

	if (Mp4_Setbuf(IO_TRANSFER_BLOCK_SIZE) != ME_OK) 
	{    
	}

	return ME_OK;

}

ERR  MP4Mux_OnInfo (AM_VIDEO_INFO *pvInfo,AM_AUDIO_INFO *paInfo)
{	
	InitH264(pvInfo);
	InitAudio(paInfo);
	InitProcess();
	
	return 0;
}

int MP4Mux_Init()
{
	Init();
	
	return 1;	
}

int MP4Mux_GetVideoInfo(U8* pData, UINT size,UINT framerate, AM_VIDEO_INFO* h264_info)
{
	put_VideoInfo(pData,size, framerate, h264_info);
	
	return 1;
}

int MP4Mux_Open(const char *filename)
{
	ERR ret = ME_OK;

	ret = SetMediaSink (filename);
	if(ret == ME_ERROR)
	{
		printf("MP4_Mux_Open failed !!\n");
		return ME_ERROR;
	}

	MP4Mux_Init();
	
	return ret;
}

int MP4Mux_GetRecordTime()
{
	return (mVideoDuration / mH264Info.scale);
}

int MP4Mux_WriteVideoData(unsigned char *buf, int framesize, unsigned int timestamp)
{
	return put_VideoData(buf,framesize,timestamp);
}

int MP4Mux_WriteAudioData(unsigned char *buf, int framesize, unsigned int timestamp)
{
	return put_AudioData(buf,framesize, 1);
}

void MP4Mux_Close()
{   
    FinishProcess();
	Mp4_CloseFile();
}
