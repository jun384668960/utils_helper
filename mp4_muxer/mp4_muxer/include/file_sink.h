#ifndef __FILE_SINK_H__
#define __FILE_SINK_H__

#ifndef IO_SIZE_KB
#define IO_SIZE_KB (1024)
#endif

#ifndef IO_TRANSFER_BLOCK_SIZE
#define IO_TRANSFER_BLOCK_SIZE (512 * IO_SIZE_KB)
#endif

#ifndef MAX_FILE_NAME_LEN
#define MAX_FILE_NAME_LEN (256)
#endif

#include "mp4_types.h"




#if 0

ERR OpenFile(const char *pFileName);
ERR CloseFile();
file_off_t GetFileSize();
file_off_t TellFile();
ERR SeekFile(file_off_t offset);
ERR AdvanceFile(file_off_t offset);
int ReadFile(void *pBuffer, UINT size);

#endif

int Mp4_CreateFile(const char *pFileName);
int Mp4_CloseFile();
int Mp4_FlushFile();
int Mp4_Setbuf(UINT);
int Mp4_SeekFile(file_off_t offset, UINT whence);
int Mp4_WriteFile(const void *pBuffer, UINT size);


#endif