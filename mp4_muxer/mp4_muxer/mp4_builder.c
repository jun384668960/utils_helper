//#include <sys/ioctl.h> //for ioctl
//#include <fcntl.h>     //for open O_* flags
//#include <unistd.h>    //for read/write/lseek
#include <stdlib.h>    //for malloc/free
#include <string.h>    //for strlen/memset
#include <stdio.h>     //for printf
#ifdef WIN32
#else
#include <sys/time.h>
#endif
#include <time.h>
#include <assert.h>

#include "mp4_builder.h"
#include "am_utils.h"
#include "file_sink.h"

#ifdef WIN32
#include <Windows.h>
#if 0
int gettimeofday(struct timeval *tp, void *tzp)
{
	time_t clock;
	struct tm tm;
	SYSTEMTIME wtm;
	GetLocalTime(&wtm);
	tm.tm_year   = wtm.wYear - 1900;
	tm.tm_mon   = wtm.wMonth - 1;
	tm.tm_mday   = wtm.wDay;
	tm.tm_hour   = wtm.wHour;
	tm.tm_min   = wtm.wMinute;
	tm.tm_sec   = wtm.wSecond;
	tm. tm_isdst  = -1;
	clock = mktime(&tm);
	tp->tv_sec = clock;
	tp->tv_usec = wtm.wMilliseconds * 1000;
	return (0);
}
#endif
#endif
                     
U32 mVideoDuration;
U32 mAudioDuration;
U32 mVideoCnt;
U32 mAudioCnt;
U32 mStssCnt;

unsigned int _v_stsz[MAX_FRAME_NUM_FOR_SINGLE_FILE]; //sample size
unsigned int _v_stco[MAX_FRAME_NUM_FOR_SINGLE_FILE]; //chunk_offset
unsigned int _a_stsz[MAX_FRAME_NUM_FOR_SINGLE_FILE]; //sample size
unsigned int _a_stco[MAX_FRAME_NUM_FOR_SINGLE_FILE]; //chunk_offset
unsigned int _ctts[2*MAX_FRAME_NUM_FOR_SINGLE_FILE]; //composition time
unsigned int _stts[2*MAX_FRAME_NUM_FOR_SINGLE_FILE]; //decoding time
U32  _stss[MAX_FRAME_NUM_FOR_SINGLE_FILE];   //sync sample

int _last_ctts;

U32 _create_time;
U32 _decimal_pts;

U32 mCurPos;
U32 _v_stts_pos;
U32 _v_ctts_pos;
U32 _v_stsz_pos;
U32 _v_stco_pos;
U32 _v_stss_pos;
U32 _a_stsz_pos;
U32 _a_stco_pos;

U32 _mdat_begin_pos;
U32 _mdat_end_pos;

PTS _last_video_pts;
U16 _audio_spec_config;
AM_VIDEO_INFO mH264Info;
AM_AUDIO_INFO _audio_info;
//CMP4MUX_RECORD_INFO _record_info;

U32 _last_idr_num;
int _last_p_num;
int _last_i_num;
int _last_pic_order_cnt;
U64 _last_idr_pts;

char* _pps;
char* _sps;
U32 _pps_size;
U32 _sps_size;
sps_info_t mSpsInfo;

ADTS mAdts[max_adts];
U32 mCurrAdtsIndex;
U32 mFrameCount;

int entries;
typedef struct MOVStts {
    int count;
    int duration;
} MOVStts;
MOVStts stts_entries[MAX_FRAME_NUM_FOR_SINGLE_FILE];


int Adts_IsSyncWordOk(AdtsHeader *adtsHeader)
{
	return (0x0FFF == (0x0000 | (adtsHeader->syncword_12to5 << 4) | adtsHeader->syncword_4to1));
}

uint16_t Adts_FrameLength(AdtsHeader *adtsHeader)
{
	return (uint16_t)(0x0000 | (adtsHeader->framelength_13to12 << 11)
							 | (adtsHeader->framelength_11to4 << 3)   
                             | (adtsHeader->framelength_3to1));
}

uint16_t Adts_BufferFullness(AdtsHeader *adtsHeader)
{
	return (uint16_t)(0x0000 | (adtsHeader->buffer_fullness_11to7 << 6)
							 | (adtsHeader->buffer_fullness_6to1));
}

uint8_t Adts_ProtectionAbsent(AdtsHeader *adtsHeader)
{
	return (uint8_t)(0x00 | adtsHeader->protection);
}

uint8_t Adts_AacFrameNumber(AdtsHeader *adtsHeader)
{
	return (uint8_t)(0x00 | adtsHeader->number_of_aac_frame);
}

uint8_t Adts_AacAudioObjectType(AdtsHeader *adtsHeader)
{
	return (uint8_t)((0x00 | adtsHeader->profile) + 1);
}

uint8_t Adts_AacFrequencyIndex(AdtsHeader *adtsHeader)
{
	return (uint8_t)(0x00 | adtsHeader->sample_freqency_index);
}

uint8_t Adts_AacChannelConf(AdtsHeader *adtsHeader)
{
	return (uint8_t)(0x00 | ((adtsHeader->channel_conf_3 << 2) | adtsHeader->channel_conf_2to1));
} 
    
void Init()
{
	mVideoDuration = 0;
	mAudioDuration = 0;
	mVideoCnt = 0;
	mAudioCnt = 0;
	mStssCnt = 0;
	_last_ctts = 0;
	_create_time = 0;
	_decimal_pts = 0;
	mCurPos = 0;
	_v_stts_pos = 0;
	_v_ctts_pos = 0;
	_v_stsz_pos = 0;
	_v_stco_pos = 0;
	_v_stss_pos = 0;
	_a_stsz_pos = 0;
	_a_stco_pos = 0;
	_mdat_begin_pos = 0;
	_mdat_end_pos = 0;
	_last_video_pts = 0;
	_audio_spec_config = 0xffff;
	_last_idr_num = 0;
	_last_p_num = 0;
	_last_i_num = 0;
	_last_pic_order_cnt = 0;
	_last_idr_pts = 0;
	_pps = NULL;
	_sps = NULL;
	_pps_size = 0;
	_sps_size = 0;
	mCurrAdtsIndex = 0;
	mFrameCount = 0;
	entries = -1;
	
	memset(&mH264Info, 0, sizeof(mH264Info));
	memset(&_audio_info, 0, sizeof(_audio_info));
	//memset(&_record_info, 0, sizeof(_record_info));
	memset(&mSpsInfo, 0, sizeof(mSpsInfo));

	//_record_info.max_filesize = 1<<31;  // 2G
	//_record_info.max_videocnt = -1;
}

void freeSPSPPS()
{
	free(_pps);
	free(_sps);
	_pps = NULL;
	_sps = NULL;
}

void InitH264(AM_VIDEO_INFO* h264_info)
{
	if (h264_info) 
	{
		memcpy(&mH264Info, h264_info, sizeof(AM_VIDEO_INFO));
	} 
	else 
	{
		mH264Info.fps = 0;
		mH264Info.width = 1280;
		mH264Info.height = 720;
		mH264Info.M = 3;
		mH264Info.N = 30;
		mH264Info.rate = 1001;
		mH264Info.scale = 30000;
	}
}

void InitAudio(AM_AUDIO_INFO* audio_info)
{
	if (audio_info) 
	{
		memcpy(&_audio_info, audio_info, sizeof(AM_AUDIO_INFO));
	} 
	else 
	{ //default value
		_audio_info.sampleRate = 16000;//48000;//8000
		_audio_info.chunkSize = 1024;//320
		_audio_info.sampleSize = 16;//2
		_audio_info.channels = 2;//1
		_audio_info.pktPtsIncr = 1800;
	}
}

ERR InitProcess()
{
	put_FileTypeBox();
	put_MediaDataBox();

	return ME_OK;
}

ERR FinishProcess()
{
	struct timeval start, end, diff;
	gettimeofday(&start, NULL);
	_mdat_end_pos = mCurPos;
	
	//mVideoDuration = mVideoDuration*58/100;
	put_MovieBox();

	/* reset internal varibles, except sps/pps related */
	mCurPos = 0;
	mVideoDuration = 0;
	mAudioDuration = 0;
	mVideoCnt = 0;
	mAudioCnt = 0;
	mStssCnt = 0;
	_last_ctts = 0;

	_decimal_pts = 0;
	_v_stts_pos = 0;
	_v_ctts_pos = 0;
	_v_stsz_pos = 0;
	_v_stco_pos = 0;
	_v_stss_pos = 0;
	_a_stsz_pos = 0;
	_a_stco_pos = 0;

	_mdat_begin_pos = 0;
	_mdat_end_pos = 0;

	_last_video_pts = 0;
	_last_idr_num = 0;
	_last_p_num = 0;
	_last_i_num = 0;
	_last_pic_order_cnt = 0;
	_last_idr_pts = 0;
	entries = -1;
	
	gettimeofday(&end, NULL);
	timersub(&end, &start, &diff);

	return ME_OK;
}

ERR get_time(unsigned int* time_since1904, char * time_str,  int len)
{
	time_t  t;
	struct tm * utc;
	t= time(NULL);
	utc = localtime(&t);
	
	if (strftime(time_str, len, "%Y%m%d%H%M%S",utc) == 0) 
	{
	return ME_ERROR;
	}
	
	t = mktime(utc);//seconds since 1970-01-01 00:00:00 UTC
	//1904-01-01 00:00:00 UTC -> 1970-01-01 00:00:00 UTC
	//66 years plus 17 days of the 17 leap years [1904, 1908, ..., 1968]
	*time_since1904 = t+66*365*24*60*60+17*24*60*60;
	
	return ME_OK;
}

//--------------------------------------------------
//big-endian format
__inline ERR put_byte(unsigned int data)
{
	char w[1];
	w[0] = data;      //(data&0xFF);

	return put_buffer(w, 1);
}

__inline ERR put_be16(unsigned int data)
{
	char w[2];
	w[1] = data;      //(data&0x00FF);
	w[0] = data>>8;   //(data&0xFF00)>>8;

	return put_buffer(w, 2);
}

__inline ERR put_be24(unsigned int data)
{
	char w[3];
	w[2] = data;     //(data&0x0000FF);
	w[1] = data>>8;  //(data&0x00FF00)>>8;
	w[0] = data>>16; //(data&0xFF0000)>>16;

	return put_buffer(w, 3);
}

__inline ERR put_be32(unsigned int data)
{
	unsigned int dataBe = LeToBe32(data);
	
	return put_buffer((char*)&dataBe, sizeof(data));
}

__inline ERR put_buffer(char *pdata,unsigned int size)
{
	mCurPos += size;
	
//  return mpMuxedFile->WriteData(pdata, size);
	if (pdata == NULL || size <= 0) 
	{
    	return ME_BAD_PARAM;
    }
	return Mp4_WriteFile(pdata, size);
}

ERR put_boxtype(const char* pdata)
{
	mCurPos += 4;
//	return mpMuxedFile->WriteData((char *)pdata, 4);

	return Mp4_WriteFile((char *)pdata, 4);
}

// get the type and length of a nal unit
unsigned int GetOneNalUnit(char *pNaluType,char *pBuffer,unsigned int size)
{
	unsigned int code, tmp, pos=0;
	  
	for (code=0xffffffff, pos = 0; pos <4; pos++) 
	{
		tmp = pBuffer[pos];
		code = (code<<8)|tmp;
	}

	// AM_ASSERT(code == 0x00000001); // check start code 0x00000001 (BE)

	*pNaluType = pBuffer[pos++] & 0x1F;
	for (code=0xffffffff; pos < size; pos++) 
	{
		tmp = pBuffer[pos];
		if ((code=(code<<8)|tmp) == 0x00000001) 
		{
			break; //next start code is found
		}
	}

	if (pos == size ) 
	{
	// next start code is not found, this must be the last nalu
		return size;
	} 
	else 
	{
		return pos-4+1;
	}
}

unsigned int read_bit(char* pBuffer, int* _value,char* bit_pos, unsigned int num)
{
	int  i=0;
	int j =0 ;
	*_value = 0;
	
	for (j =0 ; j<num; j++) 
	{
		if (*bit_pos == 8) 
		{
			*bit_pos = 0;
			i++;
		}
		if (*bit_pos == 0) 
		{
			if ((pBuffer[i] == 0x3) &&(*(pBuffer+i-1) == 0) &&(*(pBuffer+i-2) == 0)) 
			{
				i++;
			}
		}
		*_value  <<= 1;
		*_value  += pBuffer[i] >> (7 -(*bit_pos)++) & 0x1;
	}
	
	return i;
}

//unsigned int parse_exp_codes(char* pBuffer, int* value,char* bit_pos=0, char type=0)
unsigned int parse_exp_codes(char* pBuffer, int* value,char* bit_pos, char type)
{
	int leadingZeroBits = -1;
	unsigned int i=0;
	char j=*bit_pos;
	char b = 0;
	int codeNum = 0;
	for (b = 0; !b; leadingZeroBits ++, j ++) 
	{
		if(j == 8) 
		{
		  	i++;
		  	j=0;
		}
		if (j == 0) 
		{
		  	if ((pBuffer[i] == 0x3) &&(*(pBuffer+i-1) == 0) &&(*(pBuffer+i-2) == 0)) 
		  	{
		   		i++;
		  	}
		}
		b = pBuffer[i] >> (7 -j) & 0x1;
	}
	
	
	i += read_bit(pBuffer+i,  &codeNum, &j, leadingZeroBits);
	codeNum += (1 << leadingZeroBits) -1;
	
	if (type == 0) 
	{ //ue(v)
		*value = codeNum;
	} 
	else if (type == 1) 
	{//se(v)
		*value = (codeNum/2+1);
		if (codeNum %2 == 0) 
		{
		  	*value *= -1;
		}
	}
	*bit_pos = j;
	
	return i;
}

#define parse_ue(x,y,z) parse_exp_codes((x),(y),(z),0)
#define parse_se(x,y,z) parse_exp_codes((x),(y),(z),1)

int parse_scaling_list(char* pBuffer,unsigned int sizeofScalingList,char* bit_pos)
{
	int* scalingList = (int*)malloc(sizeofScalingList*sizeof(int));

	int lastScale = 8;
	int nextScale = 8;
	int delta_scale;
	int i = 0;
	unsigned int j = 0;
	for (j = 0; j < sizeofScalingList; ++ j) 
	{
	    if (nextScale != 0) 
		{
			i += parse_se(pBuffer, &delta_scale,bit_pos);
	     	nextScale = (lastScale+ delta_scale + 256)%256;
	    }
	    scalingList[j] = ( nextScale == 0 ) ? lastScale : nextScale;
	    lastScale = scalingList[j];
  	}

	free(scalingList);

	return i;
};

ERR get_h264_info(unsigned char* pBuffer,int size,AM_VIDEO_INFO* pH264Info)
{
	char* pSPS = pBuffer;
	char bit_pos = 0;
	int profile_idc; // u(8)
	int constraint_set; //u(8)
	int level_idc; //u(8)
	int seq_paramter_set_id = 0;
	int chroma_format_idc = 0;
	int residual_colour_tramsform_flag;
	int bit_depth_luma_minus8 = 0;
	int bit_depth_chroma_minus8 = 0;
	int qpprime_y_zero_transform_bypass_flag;
	int seq_scaling_matrix_present_flag;
	int seq_scaling_list_present_flag[8] ;
	//int log2_max_frame_num_minus4;
	//int pic_order_cnt_type;
	//int log2_max_pic_order_cnt_lsb_minus4;
	int num_ref_frames;
	int gaps_in_frame_num_value_allowed_flag;
	int pic_width_in_mbs_minus1;
	int pic_height_in_map_units_minus1;
	//int frame_mbs_only_flag;
	int direct_8x8_inference_flag;
	int delta_pic_order_always_zero_flag;
	int offset_for_non_ref_pic;
	int offset_for_top_to_bottom_field;
	int num_ref_frames_in_pic_order_cnt_cycle;
	int i =0;
	int* offset_for_ref_frame;
	int mb_adaptive_frame_field_flag;
	int frame_cropping_flag;
	int vui_parameters_present_flag;
	int frame_crop_left_offset;
	int frame_crop_right_offset;
	int frame_crop_top_offset;
	int frame_crop_bottom_offset;
	int aspect_ratio_info_present_flag;
	int aspect_ratio_idc;
	int video_full_range_flag;
	int overscan_info_present_flag;
	int overscan_appropriate_flag;
	int video_signal_type_present_flag;
	int video_format;
	int colour_description_present_flag;
	int chroma_loc_info_present_flag;
	int timing_info_present_flag;

	pSPS += read_bit(pSPS, &profile_idc,&bit_pos,8);
	pSPS += read_bit(pSPS, &constraint_set,&bit_pos,8);
	pSPS += read_bit(pSPS, &level_idc, &bit_pos,8);
	pSPS += parse_ue(pSPS,&seq_paramter_set_id, &bit_pos);
	
	if (profile_idc == 100 ||profile_idc == 110 
			||profile_idc == 122 ||profile_idc == 144 )
	{
		pSPS += parse_ue(pSPS,&chroma_format_idc,&bit_pos);

		if (chroma_format_idc == 3) 
		{
		  	pSPS += read_bit(pSPS,(int *)&residual_colour_tramsform_flag,&bit_pos,1);
		}
		pSPS += parse_ue(pSPS, &bit_depth_luma_minus8,&bit_pos);
		pSPS += parse_ue(pSPS, &bit_depth_chroma_minus8,&bit_pos);
		pSPS += read_bit(pSPS,
		                (int *)&qpprime_y_zero_transform_bypass_flag,
		                 &bit_pos,1);
		pSPS += read_bit(pSPS,
		                (int *)&seq_scaling_matrix_present_flag,
		                 &bit_pos,1);
		
		if (seq_scaling_matrix_present_flag ) 
		{
			unsigned int i = 0;
			for (i = 0; i < 8; ++ i) 
		  	{
		    	pSPS += read_bit(pSPS,
		                    	(int *)&seq_scaling_list_present_flag[i],
		                     	&bit_pos,1);
			    if (seq_scaling_list_present_flag[i]) 
				{
					if (i < 6) 
					{
						pSPS += parse_scaling_list(pSPS,16,&bit_pos);
			      	} 
				 	else 
				  	{
			        	pSPS += parse_scaling_list(pSPS,64,&bit_pos);
			      	}
			    }
		  	}
		}
	}
	
	pSPS += parse_ue(pSPS, &mSpsInfo.log2_max_frame_num_minus4,&bit_pos);
	pSPS += parse_ue(pSPS, &mSpsInfo.pic_order_cnt_type,&bit_pos);

	if (mSpsInfo.pic_order_cnt_type == 0) 
	{
		pSPS += parse_ue(pSPS,&mSpsInfo.log2_max_pic_order_cnt_lsb_minus4,&bit_pos);
	} 
	else if (mSpsInfo.pic_order_cnt_type == 1) 
	{
		
		pSPS += read_bit(pSPS,
		                 (int *)&delta_pic_order_always_zero_flag, &bit_pos,1);
		
		pSPS += parse_se(pSPS,
		                 &offset_for_non_ref_pic, &bit_pos);
		pSPS += parse_se(pSPS, &offset_for_top_to_bottom_field, &bit_pos);
		pSPS += parse_ue(pSPS, &num_ref_frames_in_pic_order_cnt_cycle, &bit_pos);

		offset_for_ref_frame = (int*)malloc(num_ref_frames_in_pic_order_cnt_cycle*sizeof(int));
		
		for (i =0;i < num_ref_frames_in_pic_order_cnt_cycle; i++ ) 
		{
		  	pSPS += parse_se(pSPS, offset_for_ref_frame + i, &bit_pos);
		}

		free(offset_for_ref_frame);
	}
	
	pSPS += parse_ue(pSPS,&num_ref_frames, &bit_pos);
	pSPS += read_bit(pSPS,(int *)&gaps_in_frame_num_value_allowed_flag, &bit_pos,1);
	pSPS += parse_ue(pSPS,&pic_width_in_mbs_minus1,&bit_pos);
	pSPS += parse_ue(pSPS,&pic_height_in_map_units_minus1, &bit_pos);

	pH264Info->width  = (short)(pic_width_in_mbs_minus1 + 1) << 4;
	pH264Info->height = (short)(pic_height_in_map_units_minus1 + 1) <<4;

	pSPS += read_bit(pSPS, &mSpsInfo.frame_mbs_only_flag, &bit_pos,1);
	if (!mSpsInfo.frame_mbs_only_flag) 
	{
		
		pSPS += read_bit(pSPS, (int *)&mb_adaptive_frame_field_flag, &bit_pos,1);
		pH264Info->height *= 2;
	}

	pSPS += read_bit(pSPS, (int *)&direct_8x8_inference_flag, &bit_pos,1);
	
	pSPS += read_bit(pSPS, (int *)&frame_cropping_flag, &bit_pos,1);

	if (frame_cropping_flag) 
	{
		
		pSPS += parse_ue(pSPS,&frame_crop_left_offset, &bit_pos);
		pSPS += parse_ue(pSPS,&frame_crop_right_offset, &bit_pos);
		pSPS += parse_ue(pSPS,&frame_crop_top_offset, &bit_pos);
		pSPS += parse_ue(pSPS,&frame_crop_bottom_offset, &bit_pos);
	}
	
	pSPS += read_bit(pSPS, (int *)&vui_parameters_present_flag, &bit_pos,1);

	if (vui_parameters_present_flag) 
	{
		
		pSPS += read_bit(pSPS, (int *)&aspect_ratio_info_present_flag, &bit_pos,1);
		if (aspect_ratio_info_present_flag) 
		{
			
			pSPS += read_bit(pSPS, &aspect_ratio_idc,&bit_pos,8);
			if (aspect_ratio_idc == 255) 
			{// Extended_SAR
				int sar_width;
				int sar_height;
				pSPS += read_bit(pSPS, &sar_width, &bit_pos, 16);
				pSPS += read_bit(pSPS, &sar_height, &bit_pos, 16);
			}
		}
		
		
		pSPS += read_bit(pSPS, (int *)&overscan_info_present_flag, &bit_pos,1);
		if (overscan_info_present_flag) 
		{
			
			pSPS += read_bit(pSPS, (int *)&overscan_appropriate_flag, &bit_pos,1);
		}
		
		pSPS += read_bit(pSPS, (int *)&video_signal_type_present_flag, &bit_pos,6);
		if (video_signal_type_present_flag) 
		{
			
			pSPS += read_bit(pSPS, &video_format, &bit_pos,3);
			
			pSPS += read_bit(pSPS, (int *)&video_full_range_flag, &bit_pos,1);
			
			pSPS += read_bit(pSPS,
			               (int *)&colour_description_present_flag, &bit_pos,1);
			if (colour_description_present_flag) 
			{
				int colour_primaries, transfer_characteristics,matrix_coefficients;
				pSPS += read_bit(pSPS, &colour_primaries, &bit_pos, 8);
				pSPS += read_bit(pSPS, &transfer_characteristics, &bit_pos, 8);
				pSPS += read_bit(pSPS, &matrix_coefficients, &bit_pos, 8);
			}
		}
		
		pSPS += read_bit(pSPS, (int *)&chroma_loc_info_present_flag, &bit_pos,1);

		if( chroma_loc_info_present_flag ) 
		{
			int chroma_sample_loc_type_top_field;
			int chroma_sample_loc_type_bottom_field;
			pSPS += parse_ue(pSPS,&chroma_sample_loc_type_top_field, &bit_pos);
			pSPS += parse_ue(pSPS,&chroma_sample_loc_type_bottom_field, &bit_pos);
		}
		
		pSPS += read_bit(pSPS, (int *)&timing_info_present_flag, &bit_pos,1);

		if (timing_info_present_flag) 
		{
			//int num_units_in_tick,time_scale;
			int fixed_frame_rate_flag;
			pSPS += read_bit(pSPS, &mSpsInfo.num_units_in_tick, &bit_pos, 32);
			pSPS += read_bit(pSPS, &mSpsInfo.time_scale, &bit_pos, 32);
			pSPS += read_bit(pSPS, (int *)&fixed_frame_rate_flag, &bit_pos,1);
			if (fixed_frame_rate_flag) 
			{
				char divisor; //when pic_struct_present_flag == 1 && pic_struct == 0
				//pH264Info->fps = (float)time_scale/num_units_in_tick/divisor;
				if (mSpsInfo.frame_mbs_only_flag) 
				{
					divisor = 2;
				} 
				else 
				{
					divisor = 1; // interlaced
				}
				pH264Info->fps = divisor * mSpsInfo.num_units_in_tick;
				if (mH264Info.rate == 0 && mH264Info.scale ==0) 
				{
					mH264Info.rate = mSpsInfo.num_units_in_tick;
					mH264Info.scale = mSpsInfo.time_scale/2;
				}
			} 
			else 
			{ //default value
				pH264Info->fps = 3003;
			}
		}
	} 
	else 
	{ //default value
		pH264Info->fps = 3003;
	}

	if ((pH264Info->width == 0) ||
	   (pH264Info->height == 0) ||
	   (pH264Info->fps == 0)) 
	{
		return ME_ERROR;
	} 
	else 
	{
		return ME_OK;
	}
}

ERR get_pic_order(unsigned char* pBuffer, int size, int nal_unit_type, int* pic_order_cnt_lsb) 
{
	char* pSlice_header = pBuffer;
	char bit_pos = 0;
	int first_mb_in_slice;
	int slice_type;
	int pic_parameter_set_id;
	int frame_num;
	int max_pic_order_cnt_lsb;
	pSlice_header += parse_ue(pSlice_header,&first_mb_in_slice,&bit_pos);
	pSlice_header += parse_ue(pSlice_header,&slice_type,&bit_pos);
	switch (slice_type) 
	{
		case SLICE_P_0:
		case SLICE_P_1:
		if (_last_p_num == -1) 
		{
		    _last_p_num = mVideoCnt;
		} 
		else 
		{
			if (mH264Info.M == 0) 
			{
		      	mH264Info.M = mVideoCnt - _last_p_num;
		    }
		}
		break;
		case SLICE_I_0:
		case SLICE_I_1:
		if (_last_i_num == -1) 
		{
		    _last_i_num = mVideoCnt;
		} 
		else 
		{
		    if (mH264Info.N == 0) 
			{
		      mH264Info.N = mVideoCnt - _last_i_num;
		    }
		}
		break;
		default:
		  	break;
	}

	pSlice_header += parse_ue(pSlice_header,&pic_parameter_set_id,&bit_pos);
	pSlice_header += read_bit(pSlice_header,&frame_num,&bit_pos,
	                        mSpsInfo.log2_max_frame_num_minus4+4);

	if (!mSpsInfo.frame_mbs_only_flag)
	{
		int field_pic_flag;
		int bottom_field_flag;
		pSlice_header += read_bit(pSlice_header, (int *)&field_pic_flag,&bit_pos,1);
		if (field_pic_flag)
			pSlice_header += read_bit(pSlice_header, (int *)&bottom_field_flag,&bit_pos,1);
	}
	
	if (nal_unit_type == NAL_IDR) 
	{
		int idr_pic_id;
		pSlice_header += parse_ue(pSlice_header,&idr_pic_id,&bit_pos);
	}
	
	if( mSpsInfo.pic_order_cnt_type == 0 ) 
	{
		pSlice_header += read_bit(pSlice_header,pic_order_cnt_lsb,&bit_pos,
		                          mSpsInfo.log2_max_pic_order_cnt_lsb_minus4+4);
		max_pic_order_cnt_lsb = 1<<(mSpsInfo.log2_max_pic_order_cnt_lsb_minus4+4);
		if (*pic_order_cnt_lsb > max_pic_order_cnt_lsb/2) 
		{
		  	*pic_order_cnt_lsb -= max_pic_order_cnt_lsb;
		}
	}
	
	return ME_OK;
}

ERR UpdateVideoIdx(int deltaPts, unsigned int sampleSize,unsigned int chunk_offset, char sync_point)
{
	int i;
	
	_v_stsz[mVideoCnt] = LeToBe32(sampleSize);		//sample size
	_v_stco[mVideoCnt] = LeToBe32(chunk_offset);	//chunk_offset

	mVideoCnt++;

	mVideoDuration += deltaPts;
	//printf("deltaPts=%d\n",deltaPts);
	
	for (i = 0; i < mVideoCnt; i++)    //记录有多少项不同的时长
	{
	    if (deltaPts == stts_entries[i].duration) 
		{
	        stts_entries[i].count++; /* compress */
			break;
	    }
		if(i == mVideoCnt - 1)
		{
	        entries++;
	        stts_entries[entries].duration = deltaPts;
	        stts_entries[entries].count = 1;
	    }
	}
	
	if (sync_point)   //有多少项关键帧
	{
		_stss[mStssCnt] = LeToBe32(mVideoCnt);
		mStssCnt++;
	}

	return ME_OK;
}

__inline ERR UpdateAudioIdx(unsigned int sampleSize,unsigned int chunk_offset)
{
	_a_stsz[mAudioCnt] = LeToBe32(sampleSize);   //sample size
	_a_stco[mAudioCnt] = LeToBe32(chunk_offset); //chunk_offset

	mAudioDuration += _audio_info.pktPtsIncr;   //一帧音频时长(叠加)
	mAudioCnt ++;
	
	return ME_OK;
}

#define FileTypeBox_SIZE 32
ERR put_FileTypeBox()
{
	put_be32(FileTypeBox_SIZE); //uint32 size
	put_boxtype("ftyp");
	put_boxtype("isom");        //uint32 major_brand
	put_be32(0x00000200);                //uint32 minor_version
	put_boxtype("isom");        //uint32 compatible_brands[]
	put_boxtype("iso2");
	put_boxtype("avc1");
	put_boxtype("mp41");

	put_be32(8); //uint32 size
	put_boxtype("free");
  
  	return ME_OK;
}


//mp4 file size < 2G, no B frames, 1 chunk has only one sample
ERR put_MediaDataBox()
{
	int ret = 0;
	_mdat_begin_pos = mCurPos;
	ret += put_be32(0);            //uint32 size, will be revised in the end
	ret += put_boxtype("mdat");

	return ((ret == 0) ? ME_OK : ME_ERROR);
}

int put_VideoInfo(char* pData, unsigned int size, unsigned int framerate, AM_VIDEO_INFO* h264_info)
{
	unsigned int nal_unit_length;
	char nal_unit_type;
	unsigned int cursor = 0;

	do
	{
		nal_unit_length = GetOneNalUnit(&nal_unit_type, pData+cursor, size-cursor);
		assert(nal_unit_length > 0 && nal_unit_type > 0);

		if (nal_unit_type == NAL_SPS)     	
		{ 
			if (_sps == NULL) 
			{
				_sps_size = nal_unit_length -4;
				_sps = (char*)malloc(_sps_size*sizeof(char));
				memcpy(_sps, pData+cursor+4, _sps_size); //exclude start code 0x00000001
			}
			
			if ((mH264Info.width == 0) || (mH264Info.height == 0)
			  	|| (mH264Info.fps == 0)) 
			{
				get_h264_info(pData+cursor+5,nal_unit_length-5, h264_info) ;

				h264_info->fps = (U32)(512000000/framerate); //++VIDEO_FPS_10;//17083750;
         		h264_info->M = 1;
   				h264_info->N = 30;
    			h264_info->scale = 90000;    //时间刻度
    			h264_info->rate  = (U32)(h264_info->scale / framerate);
				memcpy(&mH264Info, h264_info, sizeof(AM_VIDEO_INFO));
     		}
      		break;
    	} 

		cursor += nal_unit_length;
	} while (cursor < size);

	return 1;
}

ERR put_VideoData(char* pData, unsigned int size,PTS pts)
{
	unsigned int nal_unit_length;
	char nal_unit_type;
	unsigned int cursor = 0;
	unsigned int chunk_offset = mCurPos;
	char sync_point = 0;
	int ret = 0;
	int delta_pts;
	do
	{
		nal_unit_length = GetOneNalUnit(&nal_unit_type, pData+cursor, size-cursor);
		//  AM_ASSERT(nal_unit_length > 0 && nal_unit_type > 0);

		if (nal_unit_type == NAL_SPS) 
		{ // write sps
			if (_sps == NULL) 
			{
				_sps_size = nal_unit_length -4;
				//  _sps = new char[_sps_size];
				_sps = (char*)malloc(_sps_size*sizeof(char));
				memcpy(_sps, pData+cursor+4, _sps_size); //exclude start code 0x00000001
		 	}
		  
		 	if ((mH264Info.width == 0) || (mH264Info.height == 0)
		      || (mH264Info.fps == 0)) 
		  	{
		    	get_h264_info(pData+cursor+5,nal_unit_length-5, &mH264Info) ;
		  	}
		  
		} 
		else if (nal_unit_type == NAL_PPS) 
		{ // write pps
			if (_pps == NULL) 
		  	{
				_pps_size = nal_unit_length -4;
				//  _pps = new char[_pps_size];
				_pps = (char*)malloc(_pps_size*sizeof(char));
				memcpy(_pps, pData+cursor+4, _pps_size); //exclude start code 0x00000001
		  	}
		} 
		else if (nal_unit_type == NAL_IDR) 
		{
				sync_point = 1;
		}

		put_be32(nal_unit_length - 4); // exclude start code length
		ret = put_buffer(pData + cursor + 4, nal_unit_length - 4);// save ES data
		if(ret == ME_ERROR)
		{
			return -1;
		}
		cursor += nal_unit_length;
	} while (cursor < size);

 	 delta_pts = ((mVideoCnt == 0) ? 0 : (pts - _last_video_pts)*90000/1000);	  //一帧的时长
 	_last_video_pts = pts;

	return UpdateVideoIdx(delta_pts, (mCurPos - chunk_offset),
                        chunk_offset, sync_point);
}


ERR put_AudioData(char* pData,unsigned int dataSize,U32 frameCount)
{
#if 1
	char *pFrameStart;
	U32 frameSize;
	int ret;

	FeedStreamData(pData, dataSize, frameCount);
	
	
	while (GetOneFrame (&pFrameStart, &frameSize)) 
	{
		unsigned int   chunk_offset = mCurPos;
		AdtsHeader *adtsHeader = (AdtsHeader*)pFrameStart;
		unsigned int  header_length = sizeof(AdtsHeader);
		/*
		if ((mCurPos + frameSize + MovieBox_SIZE) > _record_info.max_filesize)
		{
		  	return ME_TOO_MANY;
		}
		*/
		if (frameSize < 7)
		{
		  	return ME_ERROR;
		}
		// if (false == adtsHeader->IsSyncWordOk()) 
		if (false == Adts_IsSyncWordOk(adtsHeader)) 
		{
		  	return ME_ERROR;
		}
		
		if (_audio_spec_config == 0xffff) 
		{
		  	_audio_spec_config = (((/*adtsHeader->AacAudioObjectType()*/Adts_AacAudioObjectType(adtsHeader) << 11) |
		                         (/*adtsHeader->AacFrequencyIndex()*/Adts_AacFrequencyIndex(adtsHeader) << 7)   |
		                         (/*adtsHeader->AacChannelConf()*/Adts_AacChannelConf(adtsHeader) << 3)) & 0xFFF8);
		}

		if (Adts_AacFrameNumber(adtsHeader) == 0)
		{
			//   if (adtsHeader->ProtectionAbsent() == 0) 
			if (Adts_ProtectionAbsent(adtsHeader) == 0) 
			{ //adts_error_check
				header_length += 2;
			}
			//adts_fixed_header + adts_variable_header
			ret = put_buffer(pFrameStart + header_length, frameSize - header_length);
			if(ret == ME_ERROR)
			{
				return ME_ERROR;
			}
		} 
		else 
		{
		  //Todo
		}
		UpdateAudioIdx((frameSize - header_length), chunk_offset);
	}
#endif

#if 0
	AdtsHeader *adtsHeader = (AdtsHeader*)pData;
	unsigned int  header_length = sizeof(AdtsHeader);
	   _audio_spec_config = (((/*adtsHeader->AacAudioObjectType()*/Adts_AacAudioObjectType(adtsHeader) << 11) |
	                         (/*adtsHeader->AacFrequencyIndex()*/Adts_AacFrequencyIndex(adtsHeader) << 7)   |
	                         (/*adtsHeader->AacChannelConf()*/Adts_AacChannelConf(adtsHeader) << 3)) & 0xFFF8);

	printf("-------------Audio Spec Info: 0x%04x\n",_audio_spec_config);  

	printf("Adts_FrameLength %d,Adts_AacAudioObjectType %d,Adts_AacFrequencyIndex %d,Adts_AacChannelConf %d,Adts_ProtectionAbsent %d\n",Adts_FrameLength(adtsHeader),Adts_AacAudioObjectType(adtsHeader),Adts_AacFrequencyIndex(adtsHeader),Adts_AacChannelConf(adtsHeader),Adts_ProtectionAbsent(adtsHeader));
	//   if (_audio_spec_config == 0xffff) 
	{
	//     _audio_spec_config = 4128;
	}
	unsigned int   chunk_offset = mCurPos;
	put_buffer(pData, dataSize);
	UpdateAudioIdx(dataSize, chunk_offset);

	printf("put_AudioData  chunk_offset %d,dataSize %d\n",chunk_offset,dataSize);
#endif

	return ME_OK;
}

ERR put_MovieBox()
{
	int ret = 0;
	printf("mAudioDuration=%d   mVideoDuration=%d\n",mAudioDuration,mVideoDuration);
	printf("mVideoCnt=%d mAudioCnt=%d\n",mVideoCnt,mAudioCnt);
	entries++;    //last entries
#if GOKE_AUDIODURATION_SYNCHRONIZATION  
	mAudioDuration = mVideoDuration;
#endif
	//printf("mAudioDuration=%d   mVideoDuration=%d\n",mAudioDuration,mVideoDuration);
	//MovieBox
	
	ret += put_be32(MovieBox_SIZE); //uint32 size
	ret += put_boxtype("moov");     //'moov'

	//MovieHeaderBox
	ret += put_be32(MovieHeaderBox_SIZE); //uint32 size
	ret += put_boxtype("mvhd");           //'mvhd'
	ret += put_byte(0);                   //uint8 version
	put_be24(0);                          //bits24 flags
	//uint32 creation_time [version==0] uint64 creation_time [version==1]
	put_be32(_create_time);
	//uint32 modification_time [version==0] uint64 modification_time [version==1]
	put_be32(_create_time);
	put_be32(mH264Info.scale);   //uint32 timescale
	//uint32 duration [version==0] uint64 duration [version==1]

	put_be32(mVideoDuration);
	put_be32(0x00010000);                 //int32 rate
	put_be16(0x0100);                     //int16 volume
	put_be16(0);                          //bits16 reserved
	put_be32(0);                          //uint32 reserved[2]
	put_be32(0);
	put_be32(0x00010000);                 //int32 matrix[9]
	put_be32(0);
	put_be32(0);
	put_be32(0);
	put_be32(0x00010000);
	put_be32(0);
	put_be32(0);
	put_be32(0);
	put_be32(0x40000000);
	put_be32(0);                          //bits32 pre_defined[6]
	put_be32(0);
	put_be32(0);
	put_be32(0);
	put_be32(0);
	put_be32(0);
	put_be32(3);                          //uint32 next_track_ID

	/*
	put_be32(ObjDescrpBox_SIZE);          //uint32 size
	put_boxtype("iods");  //'iods'
	put_byte(0);                        	       //uint8 version
	put_be24(0);                         	      //bits24 flags
	put_be32(0x10808080);
	put_be32(0x07004FFF );
	put_be32(0xFF0F7FFF);
	*/
	
	if (mVideoDuration) 
	{
		put_VideoTrackBox(1, mVideoDuration);
	}
	
	if (mAudioDuration) 
	{
		put_AudioTrackBox(2, mAudioDuration);
	}
	UpdateIdxBox();
	
	return ME_OK;
}

void put_VideoTrackBox(unsigned int TrackId, unsigned int Duration)
{
	//TrackBox
	put_be32(VideoTrackBox_SIZE);//uint32 size
	put_boxtype("trak");         //'trak'

	//TrackHeaderBox
	put_be32(TrackHeaderBox_SIZE);//uint32 size
	put_boxtype("tkhd");          //'tkhd'
	put_byte(0);                  //uint8 version
	//0x01:track_enabled, 0x02:track_in_movie, 0x04:track_in_preview
	put_be24(0x07);               //bits24 flags
	//uint32 creation_time [version==0] uint64 creation_time [version==1]
	put_be32(_create_time);
	//uint32 modification_time [version==0] uint64 modification_time [version==1]
	put_be32(_create_time);
	put_be32(TrackId);            //uint32 track_ID
	put_be32(0);                  //uint32 reserved
	//uint32 duration [version==0] uint64 duration [version==1]
	put_be32(Duration);
	put_be32(0);                  //uint32 reserved[2]
	put_be32(0);
	put_be16(0);                  //int16 layer
	put_be16(0);                  //int16 alternate_group
	put_be16(0x0000);             //int16 volume
	put_be16(0);                  //uint16 reserved
	put_be32(0x00010000);         //int32 matrix[9]
	put_be32(0);
	put_be32(0);
	put_be32(0);
	put_be32(0x00010000);
	put_be32(0);
	put_be32(0);
	put_be32(0);
	put_be32(0x40000000);
	put_be32(mH264Info.width<<16); //uint32 width  //16.16 fixed-point
	put_be32(mH264Info.height<<16);//uint32 height //16.16 fixed-point

	put_VideoMediaBox(Duration);
}

void put_VideoMediaBox(unsigned int Duration)
{
	//MediaBox
	put_be32(VideoMediaBox_SIZE); //uint32 size
	put_boxtype("mdia");          //'mdia'

	//MediaHeaderBox
	put_be32(MediaHeaderBox_SIZE); //uint32 size
	put_boxtype("mdhd");           //'mdhd'
	put_byte(0);                   //uint8 version
	put_be24(0);                   //bits24 flags
	//uint32 creation_time [version==0] uint64 creation_time [version==1]
	put_be32(_create_time);
	//uint32 modification_time [version==0] uint64 modification_time [version==1]
	put_be32(_create_time);
	put_be32(mH264Info.scale);     //uint32 timescale
	//uint32 duration [version==0] uint64 duration [version==1]
	put_be32(Duration);
	put_be16(0);                  //bits5 language[3]  //ISO-639-2/T language code
	put_be16(0);                  //uint16 pre_defined

	//HandlerReferenceBox
	put_be32(VideoHandlerReferenceBox_SIZE); //uint32 size
	put_boxtype("hdlr");                     //'hdlr'
	put_byte(0);                             //uint8 version
	put_be24(0);                             //bits24 flags
	put_be32(0);                             //uint32 pre_defined
	put_boxtype("vide");                     //'vide'
	put_be32(0);                             //uint32 reserved[3]
	put_be32(0);
	put_be32(0);
	put_byte(VIDEO_HANDLER_NAME_LEN);   //char name[], name[0] is actual length
	put_buffer((char *)VIDEO_HANDLER_NAME, VIDEO_HANDLER_NAME_LEN-1);

	put_VideoMediaInformationBox();
}

void put_VideoMediaInformationBox()
{
	char EncoderName[32]="\012AVC Coding"; //Compressorname
	//MediaInformationBox
	put_be32(VideoMediaInformationBox_SIZE); //uint32 size
	put_boxtype("minf");                     //'minf'

	//VideoMediaHeaderBox
	put_be32(VideoMediaHeaderBox_SIZE);      //uint32 size
	put_boxtype("vmhd");                     //'vmhd'

	put_byte(0);                             //uint8 version
	//This is a compatibility flag that allows QuickTime to distinguish
	// between movies created with QuickTime 1.0 and newer movies.
	// You should always set this flag to 1, unless you are creating a movie
	// intended for playback using version 1.0 of QuickTime
	put_be24(1); //bits24 flags
	put_be16(0); //uint16 graphicsmode  //0=copy over the existing image
	put_be16(0); //uint16 opcolor[3]	  //(red, green, blue)
	put_be16(0);
	put_be16(0);

	//DataInformationBox
	put_be32(DataInformationBox_SIZE); //uint32 size
	put_boxtype("dinf"); //'dinf'

	//DataReferenceBox
	put_be32(DataReferenceBox_SIZE);   //uint32 size
	put_boxtype("dref");               //'dref'
	put_byte(0); //uint8 version
	put_be24(0); //bits24 flags
	put_be32(1); //uint32 entry_count
	put_be32(12);//uint32 size
	put_boxtype("url");//'url '
	put_byte(0); //uint8 version
	put_be24(1);//bits24 flags 1=media data is in the same file as the MediaBox

	//SampleTableBox
	put_be32(VideoSampleTableBox_SIZE); //uint32 size
	put_boxtype("stbl"); //'stbl'

	//SampleDescriptionBox
	put_be32(VideoSampleDescriptionBox_SIZE); //uint32 size
	put_boxtype("stsd"); //'stsd'
	put_byte(0); //uint8 version
	put_be24(0); //bits24 flags
	put_be32(1); //uint32 entry_count
	//VisualSampleEntry
	put_be32(VisualSampleEntry_SIZE); //uint32 size
	put_boxtype("avc1"); //'avc1'
	put_byte(0); //uint8 reserved[6]
	put_byte(0);
	put_byte(0);
	put_byte(0);
	put_byte(0);
	put_byte(0);
	put_be16(1); //uint16 data_reference_index
	put_be16(0); //uint16 pre_defined
	put_be16(0); //uint16 reserved
	put_be32(0); //uint32 pre_defined[3]
	put_be32(0);
	put_be32(0);
	put_be16(mH264Info.width); //uint16 width
	put_be16(mH264Info.height);//uint16 height
	put_be32(0x00480000); //uint32 horizresolution  72dpi
	put_be32(0x00480000); //uint32 vertresolution   72dpi
	put_be32(0); //uint32 reserved
	put_be16(1); //uint16 frame_count
	
	put_buffer(EncoderName,32);
	put_be16(0x0018); //uint16 depth   //0x0018=images are in colour with no alpha
	put_be16(-1); //int16 pre_defined
	//AvcConfigurationBox
	put_be32(AvcConfigurationBox_SIZE); //uint32 size
	put_boxtype("avcC"); //'avcC'
	put_byte(1); //uint8 configurationVersion
	put_byte(_sps[1]); //uint8 AVCProfileIndication
	put_byte(_sps[2]); //uint8 profile_compatibility
	put_byte(_sps[3]);  //uint8 level
	//uint8 nal_length  //(nal_length&0x03)+1 [reserved:6, lengthSizeMinusOne:2]
	put_byte(0xFF);
	//uint8 sps_count  //sps_count&0x1f [reserved:3, numOfSequenceParameterSets:5]
	put_byte(0xE1);
	//uint16 sps_size    //sequenceParameterSetLength
	put_be16(_sps_size);
	//uint8 sps[sps_size] //sequenceParameterSetNALUnit
	put_buffer(_sps, _sps_size);
	put_byte(1);                 //uint8 pps_count //umOfPictureParameterSets
	put_be16(_pps_size);         //uint16 pps_size //pictureParameterSetLength
	put_buffer(_pps, _pps_size);//uint8 pps[pps_size] //pictureParameterSetNALUnit

	//BitrateBox
	put_be32(BitrateBox_SIZE);				   //uint32 size
	put_boxtype("btrt");		//'btrt'
	put_be32(0);								 //uint32 buffer_size
	put_be32(0);								 //uint32 max_bitrate
	put_be32(0);								 //uint32 avg_bitrate

	//DecodingTimeToSampleBox  //bits24 flags
    put_be32(VideoDecodingTimeToSampleBox_SIZE);
    put_boxtype("stts"); 
    put_byte(0);	 //uint8 version
    put_be24(0); 	//bits24 flags
    put_be32(1);
	put_be32(mVideoCnt);
	put_be32(mVideoDuration/mVideoCnt);
	#if 0
	//DecodingTimeToSampleBox  //bits24 flags
    put_be32(VideoDecodingTimeToSampleBox_SIZE);
    put_boxtype("stts"); 
    put_byte(0);	 //uint8 version
    put_be24(0); 	//bits24 flags
    put_be32(entries);
	int i;
    for (i = 0; i < entries; i++)
	{
        put_be32(stts_entries[i].count);
        put_be32(stts_entries[i].duration);
    }	
	
	//CompositionTimeToSampleBox
    put_be32(CompositionTimeToSampleBox_SIZE);
    put_boxtype("ctts"); 
    put_byte(0);	 //uint8 version
    put_be24(0);	 //bits24 flags
    put_be32(entries); /* entry count */
    for (i = 0; i < entries; i++) 
	{
        put_be32(stts_entries[i].count);
        put_be32(stts_entries[i].duration);
    }
	#endif
		
	//SampleToChunkBox
	put_be32(SampleToChunkBox_SIZE); //uint32 size
	put_boxtype("stsc");             //'stsc'
	put_byte(0); //uint8 version
	put_be24(0); //bits24 flags
	put_be32(1); //uint32 entry_count
	put_be32(1); //uint32 first_chunk
	put_be32(1); //uint32 samples_per_chunk
	put_be32(1); //uint32 sample_description_index

	//SampleSizeBox
	put_be32(VideoSampleSizeBox_SIZE); //uint32 size
	put_boxtype("stsz");               //'stsz'
	put_byte(0);                       //uint8 version
	put_be24(0);                       //bits24 flags
	put_be32(0);                       //uint32 sampleSize
	put_be32(mVideoCnt);               //uint32 sample_count
	put_buffer((char *)_v_stsz, mVideoCnt * sizeof(_v_stsz[0]));

	//ChunkOffsetBox
	put_be32(VideoChunkOffsetBox_SIZE); //uint32 size
	put_boxtype("stco");                //'stco'
	put_byte(0);                        //uint8 version
	put_be24(0);                        //bits24 flags
	put_be32(mVideoCnt);                //uint32 entry_count
	put_buffer((char *)_v_stco, mVideoCnt * sizeof(_v_stco[0]));

	//SyncSampleBox
	put_be32(SyncSampleBox_SIZE);       //uint32 size
	put_boxtype("stss");                //'stss'
	put_byte(0);                        //uint8 version
	put_be24(0);                        //bits24 flags
	put_be32(mStssCnt);                 //uint32 entry_count
	put_buffer((char*)_stss, mStssCnt*sizeof(_stss[0]));
}

ERR UpdateIdxBox()
{
	Mp4_SeekFile(_mdat_begin_pos, SEEK_SET);
	
	mCurPos = _mdat_begin_pos;
	put_be32(_mdat_end_pos - _mdat_begin_pos);
	
	return ME_OK;
}

void put_AudioTrackBox(unsigned int TrackId, unsigned int Duration)
{
	//TrackBox
	put_be32(AudioTrackBox_SIZE);          //uint32 size
	put_boxtype("trak");                   //'trak'

	//TrackHeaderBox
	put_be32(TrackHeaderBox_SIZE);         //uint32 size
	put_boxtype("tkhd");
	put_byte(0);                           //uint8 version
	//0x01:track_enabled, 0x02:track_in_movie, 0x04:track_in_preview
	put_be24(0x07);                        //bits24 flags
	//uint32 creation_time [version==0] uint64 creation_time [version==1]
	put_be32(_create_time);
	//uint32 modification_time [version==0] uint64 modification_time [version==1]
	put_be32(_create_time);
	put_be32(TrackId);                     //uint32 track_ID
	put_be32(0);                           //uint32 reserved
	//uint32 duration [version==0] uint64 duration [version==1]
	put_be32(Duration);
	put_be32(0);                           //uint32 reserved[2]
	put_be32(0);
	put_be16(0);                           //int16 layer
	put_be16(0);                           //int16 alternate_group
	put_be16(0x0100);                      //int16 volume
	put_be16(0);                           //uint16 reserved
	put_be32(0x00010000);                  //int32 matrix[9]
	put_be32(0);
	put_be32(0);
	put_be32(0);
	put_be32(0x00010000);
	put_be32(0);
	put_be32(0);
	put_be32(0);
	put_be32(0x40000000);
	put_be32(0);                           //uint32 width  //16.16 fixed-point
	put_be32(0);                           //uint32 height //16.16 fixed-point

	put_AudioMediaBox();
}

void put_AudioMediaBox()
{
	//MediaBox
	put_be32(AudioMediaBox_SIZE);               //uint32 size
	put_boxtype("mdia");                        //'mdia'

	//MediaHeaderBox
	put_be32(MediaHeaderBox_SIZE);              //uint32 size
	put_boxtype("mdhd");                        //'mdhd'
	put_byte(0);                                //uint8 version
	put_be24(0);                                //bits24 flags
	//uint32 creation_time [version==0] uint64 creation_time [version==1]
	put_be32(_create_time);
	//uint32 modification_time [version==0] uint64 modification_time [version==1]
	put_be32(_create_time);
	//Audio's timescale is the same as Video, 90000
	put_be32(mH264Info.scale);         //uint32 timescale
	 
	//uint32 duration [version==0] uint64 duration [version==1]
	put_be32(mAudioDuration);
	put_be16(0);                  //bits5 language[3]  //ISO-639-2/T language code
	put_be16(0);                                //uint16 pre_defined

	//HandlerReferenceBox
	put_be32(AudioHandlerReferenceBox_SIZE);    //uint32 size
	put_boxtype("hdlr");	//'hdlr'
	put_byte(0);                                //uint8 version
	put_be24(0);                                //bits24 flags
	put_be32(0);                                //uint32 pre_defined
	put_boxtype("soun");	//'soun':audio track
	put_be32(0);                                //uint32 reserved[3]
	put_be32(0);
	put_be32(0);
	//char name[], name[0] is actual length
	put_byte(AUDIO_HANDLER_NAME_LEN);
	put_buffer((char *)AUDIO_HANDLER_NAME, AUDIO_HANDLER_NAME_LEN-1);

	put_AudioMediaInformationBox();
}

void put_AudioMediaInformationBox()
{
	//MediaInformationBox
	put_be32(AudioMediaInformationBox_SIZE);     //uint32 size
	put_boxtype("minf");                         //'minf'

	//SoundMediaHeaderBox
	put_be32(SoundMediaHeaderBox_SIZE);          //uint32 size
	put_boxtype("smhd");                         //'smhd'
	put_byte(0);                                 //uint8 version
	put_be24(0);                                 //bits24 flags
	put_be16(0);                                 //int16 balance
	put_be16(0);                                 //uint16 reserved

	//DataInformationBox
	put_be32(DataInformationBox_SIZE);           //uint32 size
	put_boxtype("dinf");                         //'dinf'
	//DataReferenceBox
	put_be32(DataReferenceBox_SIZE);             //uint32 size
	put_boxtype("dref");                         //'dref'
	put_byte(0);                                 //uint8 version
	put_be24(0);                                 //bits24 flags
	put_be32(1);                                 //uint32 entry_count
	put_be32(12);                                //uint32 size
	put_boxtype("url");                          //'url '
	put_byte(0);                                 //uint8 version
	//1=media data is in the same file as the MediaBox
	put_be24(1);                                 //bits24 flags

	//SampleTableBox
	put_be32(AudioSampleTableBox_SIZE);          //uint32 size
	put_boxtype("stbl");        //'stbl'

	//SampleDescriptionBox
	put_be32(AudioSampleDescriptionBox_SIZE);   //uint32 size
	put_boxtype("stsd");                        //uint32 type
	put_byte(0);                                //uint8 version
	put_be24(0);                                //bits24 flags
	put_be32(1);                                //uint32 entry_count
	//AudioSampleEntry
	put_be32(AudioSampleEntry_SIZE);            //uint32 size
	put_boxtype("mp4a");                        //'mp4a'
	put_byte(0);                                //uint8 reserved[6]
	put_byte(0);
	put_byte(0);
	put_byte(0);
	put_byte(0);
	put_byte(0);
	put_be16(1);                                 //uint16 data_reference_index
	put_be32(0);                                 //uint32 reserved[2]
	put_be32(0);
	put_be16(_audio_info.channels);              //uint16 channelcount
	put_be16(_audio_info.sampleSize /*16*8*2*/);        //uint16 samplesize
	//for QT sound
	put_be16(0xfffe);                            //uint16 pre_defined
	put_be16(0);                                 //uint16 reserved
	//= (timescale of media << 16)
	put_be32(_audio_info.sampleRate<<16);        //uint32 samplerate

	//ElementaryStreamDescriptorBox
	put_be32(ElementaryStreamDescriptorBox_SIZE);//uint32 size
	put_boxtype("esds");                         //'esds'
	put_byte(0);                                 //uint8 version
	put_be24(0);                                 //bits24 flags
	//ES descriptor takes 38 bytes
	put_byte(3);                                 //ES descriptor type tag
	put_be16(0x8080);
	put_byte(34);                                //descriptor type length
	put_be16(0);                                 //ES ID
	put_byte(0);                                 //stream priority
	//Decoder config descriptor takes 26 bytes (include decoder specific info)
	put_byte(4);              //decoder config descriptor type tag
	put_be16(0x8080);
	put_byte(22);                                //descriptor type length
	put_byte(0x40); //object type ID MPEG-4 audio=64 AAC
	//stream type:6, upstream flag:1, reserved flag:1 (audio=5)    Audio stream
	put_byte(0x15);
	put_be24(8192);                              // buffer size
	put_be32(128000);                            // max bitrate
	put_be32(128000);                            // avg bitrate
	//Decoder specific info descriptor takes 9 bytes
	//decoder specific descriptor type tag
	put_byte(5);
	put_be16(0x8080);
	put_byte(5);                                 //descriptor type length
	put_be16((unsigned int)_audio_spec_config);
	put_be16(0x0000);
	put_byte(0x00);
	//SL descriptor takes 5 bytes
	put_byte(6);                                 //SL config descriptor type tag
	put_be16(0x8080);
	put_byte(1);                                 //descriptor type length
	put_byte(2);                                 //SL value

	//DecodingTimeToSampleBox
	put_be32(AudioDecodingTimeToSampleBox_SIZE);//uint32 size
	put_boxtype("stts");                        //'stts'
	put_byte(0);                                //uint8 version
	put_be24(0);                                //bits24 flags
	put_be32(1);                                //uint32 entry_count
	put_be32(mAudioCnt);                        //uint32 sample_count
	#if GOKE_AUDIODURATION_SYNCHRONIZATION
	put_be32(mAudioDuration/mAudioCnt);  
	#else
	put_be32(_audio_info.pktPtsIncr);			//uint32 sample_delta
	#endif
	
	//SampleToChunkBox
	put_be32(SampleToChunkBox_SIZE);             //uint32 size
	put_boxtype("stsc");                         //'stsc'
	put_byte(0);                                 //uint8 version
	put_be24(0);                                 //bits24 flags
	put_be32(1);                                 //uint32 entry_count
	put_be32(1);                                 //uint32 first_chunk
	put_be32(1);                                 //uint32 samples_per_chunk
	put_be32(1);                                 //uint32 sample_description_index

	//SampleSizeBox
	put_be32(AudioSampleSizeBox_SIZE);           //uint32 size
	put_boxtype("stsz");                         //'stsz'
	put_byte(0);                                 //uint8 version
	put_be24(0);                                 //bits24 flags
	put_be32(0);                                 //uint32 sampleSize
	put_be32(mAudioCnt);                         //uint32 sample_count
	put_buffer((char *)_a_stsz, mAudioCnt*sizeof(_a_stsz[0]));

	//ChunkOffsetBox
	put_be32(AudioChunkOffsetBox_SIZE);          //uint32 size
	put_boxtype("stco");
	put_byte(0);                                 //uint8 version
	put_be24(0);                                 //bits24 flags
	put_be32(mAudioCnt);                         //uint32 entry_count
	put_buffer((char *)_a_stco, mAudioCnt * sizeof(_a_stco[0]));
}

void FindAdts (ADTS *adts, char *bs)
{
	unsigned int i = 0;
	for (i = 0; i < mFrameCount; ++ i) 
	{
		adts[i].addr = bs;
		adts[i].size = Adts_FrameLength((AdtsHeader*)bs);//((AdtsHeader*)bs)->FrameLength();
		bs += adts[i].size;
    
	}
}

void FeedStreamData(char *inputBuf,U32 inputBufSize,U32 frameCount)
{
	mFrameCount =  frameCount;
	FindAdts(mAdts, inputBuf);
	mCurrAdtsIndex = 0;
}

int GetOneFrame(char **ppFrameStart,U32 *pFrameSize)
{
	if (mCurrAdtsIndex >= mFrameCount) 
	{
		return false;
	}
	*ppFrameStart  = mAdts[mCurrAdtsIndex].addr;
	*pFrameSize    = mAdts[mCurrAdtsIndex].size;
	++ mCurrAdtsIndex;

	return true;
}
