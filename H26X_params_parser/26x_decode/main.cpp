#include "h264_stream.h"
#include "h265_stream.h"

int main(int argc, char* argv[])
{
	uint8_t sps[] = { 	0x67, 0x64, 0x00, 0x1f, 0xad, 0x84, 0x01, 0x0c, 0x20, 0x08, 0x61, 0x00, 
						0x43, 0x08, 0x02, 0x18, 0x40, 0x10, 0xc2, 0x00, 0x84, 0x3b, 0x50, 0x28, 0x02, 0xdd, 0x37, 0x01, 
						0x01, 0x01,0x02 };
	int spsLen =  sizeof(sps) / sizeof(sps[0]);
	h264_stream_t* h4 = h264_new();
	h264_configure_parse(h4, sps, spsLen, H264_SPS);
	printf("profile_idc:%d, level_idc:%d\n", h4->info->profile_idc, h4->info->level_idc);
	printf("width:%d, height:%d framerate:%f\n", h4->info->width, h4->info->height, h4->info->max_framerate);
	h264_free(h4);


	uint8_t vps[] = { 0x40, 0x01, 0x0C, 0x01, 0xFF, 0xFF, 0x01, 0x60, 0x00, 0x00, 0x03, 0x00, 0x90, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x78, 0x92, 0x80, 0x90 };	
	int vpsLen = sizeof(vps) / sizeof(vps[0]);
		
	h265_stream_t* h5 = h265_new();
	h265_configure_parse(h5, vps, vpsLen, H265_VPS);
//	h265_configure_parse(h5, sps, spsLen, H265_SPS);
	printf("profile_idc:%d, level_idc:%d tier_idc:%d\n", h5->info->profile_idc, h5->info->level_idc, h5->info->tier_idc);
//	printf("width:%d, height:%d framerate:%f\n", h5->info->width, h5->info->height, h5->info->max_framerate);

	h265_free(h5);
	
	getchar();

	return 0;
}
