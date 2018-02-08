#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "H264Handle.h"

using namespace std;

#define MAX_NALU 128*1024

int main(int argc, char* argv[])
{
	static char startBit[4] = {0x00, 0x00, 0x00, 0x01};
	
	FILE* m264 = NULL;
	m264 = fopen("v.264","wb");
	
	CH264Handle* h264Handle = new CH264Handle("test.264");
	
	/*
	int length = 0;
	NALU_t *nalu = h264Handle->AllocNALU(MAX_NALU_SIZE);
	while((length = h264Handle->GetAnnexbNALU(nalu)) > 0)
	{
		fprintf(stderr, "h264Handle->ReadOneNalu length:%d nalu->len:%d\n", length, nalu->len);
		fwrite(startBit, 1, sizeof(startBit), m264);
		fwrite(nalu->buf, 1, nalu->len, m264);
	}

	h264Handle->FreeNALU(nalu);
	*/

	int length = 0;
	unsigned char *frame = (unsigned char*)malloc(MAX_NALU_SIZE);
	while((length = h264Handle->GetAnnexbFrame(frame, MAX_NALU_SIZE))>0)
	{
		fprintf(stderr, "h264Handle->ReadOneNalu length:%d\n", length);
		fwrite(frame, 1, length, m264);
	}
	
	free(frame);
	
	delete h264Handle;
	fclose(m264);
	
	return 0;
}
