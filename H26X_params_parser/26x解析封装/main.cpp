#include "h265_stream.h"

int main(int argc, char* argv[])
{
	uint8_t buf[] = { 0x40, 0x01, 0x0C, 0x01, 0xFF, 0xFF, 0x01, 0x60, 0x00, 0x00, 0x03, 0x00, 0x90, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x78, 0x92, 0x80, 0x90 };
	int size = sizeof(buf) / sizeof(buf[0]);
	int nal_size = size;
	int rbsp_size = size;
	uint8_t* rbsp_buf = (uint8_t*)malloc(rbsp_size);

	int rc = nal_to_rbsp(2, buf, &nal_size, rbsp_buf, &rbsp_size);

	if (rc < 0) { free(rbsp_buf); return -1; } // handle conversion error

	bs_t* b = bs_new(rbsp_buf, rbsp_size);

	h265_stream_t* m_hH265 = h265_new();
	h265_read_vps_rbsp(m_hH265, b);

	printf("profile_idc:%d level_idc:%d tier_idc:%d", m_hH265->info->profile_idc, m_hH265->info->level_idc, m_hH265->info->tier_idc);

	free(rbsp_buf);
	h265_free(m_hH265);

	getchar();

	return 0;
}