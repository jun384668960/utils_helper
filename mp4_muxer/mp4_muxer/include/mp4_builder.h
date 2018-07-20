#ifndef __MP4_BUILDER_H__
#define __MP4_BUILDER_H__

#include "mp4_types.h"
#include "adts.h"

#define GOKE_AUDIODURATION_SYNCHRONIZATION   0 //国科的muxers方式
#define	MAX_FRAME_NUM_FOR_SINGLE_FILE	245760		// can buffer 8192 sec video data, 4GB for 0.5MBps

#define VIDEO_FPS(fps)			(512000000 / fps)
#define VIDEO_FPS_AUTO			0			//0
#define VIDEO_FPS_1				512000000		//(512000000 / 1)
#define VIDEO_FPS_2				256000000		//(512000000 / 2)
#define VIDEO_FPS_3				170666667		//(512000000 / 3)
#define VIDEO_FPS_4				128000000		//(512000000 / 4)
#define VIDEO_FPS_5				102400000		//(512000000 / 5)
#define VIDEO_FPS_6				85333333		//(512000000 / 6)
#define VIDEO_FPS_10			51200000		//(512000000 / 10)
#define VIDEO_FPS_12			42666667		//(512000000 / 12)
#define VIDEO_FPS_13			39384615		//(512000000 / 13)
#define VIDEO_FPS_14			36571428		//(512000000 / 14)
#define VIDEO_FPS_15			34133333		//(512000000 / 15)
#define VIDEO_FPS_20			25600000		//(512000000 / 20)
#define VIDEO_FPS_24			21333333		//(512000000 / 24)
#define VIDEO_FPS_25			20480000		//(512000000 / 25)
#define VIDEO_FPS_30			17066667		//(512000000 / 30)
#define VIDEO_FPS_50			10240000		//(512000000 / 50)
#define VIDEO_FPS_60			8533333			//(512000000 / 60)
#define VIDEO_FPS_120			4266667			//(512000000 / 120)

#define VideoMediaHeaderBox_SIZE          20
#define DataReferenceBox_SIZE             28
#define DataInformationBox_SIZE           (8+DataReferenceBox_SIZE)
#define AvcConfigurationBox_SIZE          (19+_sps_size+_pps_size)
#define BitrateBox_SIZE                   20
#define VisualSampleEntry_SIZE \
	(86+AvcConfigurationBox_SIZE+BitrateBox_SIZE)
#define VideoSampleDescriptionBox_SIZE    (16+VisualSampleEntry_SIZE)
#define VideoDecodingTimeToSampleBox_SIZE  24//(_decimal_pts? 16+(mVideoCnt<<3):16 + (entries<<3))
#define CompositionTimeToSampleBox_SIZE   0//(16 + (entries<<3))
#define SampleToChunkBox_SIZE             28
#define VideoSampleSizeBox_SIZE           (20+(mVideoCnt<<2))
#define VideoChunkOffsetBox_SIZE          (16+(mVideoCnt<<2))
#define SyncSampleBox_SIZE                (16+(mStssCnt<<2))
#define VideoSampleTableBox_SIZE \
	(8+VideoSampleDescriptionBox_SIZE  + \
	VideoDecodingTimeToSampleBox_SIZE + \
	CompositionTimeToSampleBox_SIZE   + \
	SampleToChunkBox_SIZE             + \
	VideoSampleSizeBox_SIZE           + \
	VideoChunkOffsetBox_SIZE          + \
	SyncSampleBox_SIZE)
#define VideoMediaInformationBox_SIZE \
	(8+VideoMediaHeaderBox_SIZE + \
	DataInformationBox_SIZE + \
	VideoSampleTableBox_SIZE)

//--------------------------------------------------

#define VIDEO_HANDLER_NAME            ("Ambarella AVC")
#define VIDEO_HANDLER_NAME_LEN        strlen(VIDEO_HANDLER_NAME)+1
#define MediaHeaderBox_SIZE           32
#define VideoHandlerReferenceBox_SIZE (32+VIDEO_HANDLER_NAME_LEN) // 46
#define VideoMediaBox_SIZE \
	(8+MediaHeaderBox_SIZE            + \
	VideoHandlerReferenceBox_SIZE + \
	VideoMediaInformationBox_SIZE) // 487+_spsSize+_ppsSize+V*17+IDR*4
//--------------------------------------------------

#define ElementaryStreamDescriptorBox_SIZE  50
#define AudioSampleEntry_SIZE     (36+ElementaryStreamDescriptorBox_SIZE) // 86
#define AudioSampleDescriptionBox_SIZE        (16+AudioSampleEntry_SIZE) // 102
#define AudioDecodingTimeToSampleBox_SIZE   24
#define AudioSampleSizeBox_SIZE             (20+(mAudioCnt<<2))
#define AudioChunkOffsetBox_SIZE            (16+(mAudioCnt<<2))
#define AudioSampleTableBox_SIZE            (8+AudioSampleDescriptionBox_SIZE+\
    AudioDecodingTimeToSampleBox_SIZE+\
    SampleToChunkBox_SIZE+\
    AudioSampleSizeBox_SIZE+\
    AudioChunkOffsetBox_SIZE) // 198+A*8
#define SoundMediaHeaderBox_SIZE            16
#define AudioMediaInformationBox_SIZE       (8+SoundMediaHeaderBox_SIZE+\
    DataInformationBox_SIZE+\
    AudioSampleTableBox_SIZE) // 258+A*8

#define AUDIO_HANDLER_NAME             ("Ambarella AAC")
#define AUDIO_HANDLER_NAME_LEN         strlen(AUDIO_HANDLER_NAME)+1
#define AudioHandlerReferenceBox_SIZE  (32+AUDIO_HANDLER_NAME_LEN) // 46
#define AudioMediaBox_SIZE             (8+MediaHeaderBox_SIZE+\
    AudioHandlerReferenceBox_SIZE+\
    AudioMediaInformationBox_SIZE) // 344+A*8

//--------------------------------------------------
#define MovieHeaderBox_SIZE 108
//#define ObjDescrpBox_SIZE 24//33
#define AMBABox_SIZE        32
#define UserDataBox_SIZE    0//(8+AMBABox_SIZE) // 48

#define TrackHeaderBox_SIZE 92//+12
#define VideoTrackBox_SIZE (8+TrackHeaderBox_SIZE+VideoMediaBox_SIZE)
#define AudioTrackBox_SIZE (8+TrackHeaderBox_SIZE+AudioMediaBox_SIZE) // 444+A*8
#define MovieBox_SIZE (8+MovieHeaderBox_SIZE + \
                       UserDataBox_SIZE      + \
                       VideoTrackBox_SIZE    + \
                       ((mAudioDuration == 0) ? 0 : AudioTrackBox_SIZE))



struct CMP4MUX_RECORD_INFO 
{
    char*	dest_name;
    U32 max_filesize;
    U32 max_videocnt;
};

typedef struct 
{
    int pic_order_cnt_type;
    int log2_max_frame_num_minus4;
    int log2_max_pic_order_cnt_lsb_minus4;
    int frame_mbs_only_flag;
    int num_units_in_tick;
    int time_scale;
} sps_info_t;

enum 
{
  NAL_UNSPECIFIED = 0,
  NAL_NON_IDR,
  NAL_IDR=5,
  NAL_SEI,
  NAL_SPS,
  NAL_PPS,
  NAL_AUD,
};

enum
{
  SLICE_P_0,
  SLICE_B_0,
  SLICE_I_0,
  SLICE_SP_0,
  SLICE_SI_0,

  SLICE_P_1,
  SLICE_B_1,
  SLICE_I_1,
  SLICE_SP_1,
  SLICE_SI_1,
};

enum {max_adts = 8};

#if 1
	void Init();
	void freeSPSPPS();
    ERR InitProcess();
    void InitH264(AM_VIDEO_INFO *pH264Info);
    void InitAudio(AM_AUDIO_INFO *pAudioInfo);
    ERR put_VideoData(char* pData, unsigned int size,PTS pts);
    ERR put_AudioData(char* pData, unsigned int size, U32 frameCount);
    ERR FinishProcess();
#endif

    ERR get_time(unsigned int* time_since1904, char * time_str,  int len);
    ERR put_byte(unsigned int data);
    ERR put_be16(unsigned int data);
    ERR put_be24(unsigned int data);
    ERR put_be32(unsigned int data);
    ERR put_be64(U64 data);
    ERR put_buffer(char *pdata, unsigned int size);
    ERR put_boxtype(const char* pdata);
    unsigned int get_byte();
    void get_buffer(char *pdata, unsigned int size);
    void debug_sei();

		unsigned int read_bit(char* pBuffer,  int* _value,char* bit_pos, unsigned int num);
    //unsigned int read_bit(char* pBuffer,  int* value,char* bit_pos = NULL, unsigned int num = 1);
    int parse_scaling_list(char* pBuffer,unsigned int sizeofScalingList, char* bit_pos);
    unsigned int parse_exp_codes(char* pBuffer, int* value,char* bit_pos, char type);

    unsigned int	GetOneNalUnit(char *pNaluType, char *pBuffer , unsigned int size);
    ERR get_h264_info(unsigned char* pBuffer, int size,
                         AM_VIDEO_INFO* pH264Info);
    ERR get_pic_order(unsigned char* pBuffer, int size,
                         int nal_unit_type, int* pic_order_cnt_lsb);

    ERR UpdateVideoIdx(int pts, unsigned int sample_size,
                          unsigned int chunk_offset, char sync_point);
    ERR UpdateAudioIdx(unsigned int sample_size,
                          unsigned int chunk_offset);

    ERR put_FileTypeBox();
    ERR put_MediaDataBox();
    ERR put_MovieBox();
    void put_VideoTrackBox(unsigned int TrackId, unsigned int Duration);
    void put_VideoMediaBox(unsigned int Duration);
    void put_VideoMediaInformationBox();
    void put_AudioTrackBox(unsigned int TrackId, unsigned int Duration);
    void put_AudioMediaBox();
    void put_AudioMediaInformationBox();
    ERR UpdateIdxBox();

    void FeedStreamData(char* inputBuf,
                                U32 inputBufSize,
                                U32 frameCount);
    int GetOneFrame(char** ppFrameStart,
                                U32* pFrameSize);

	void FindAdts (ADTS *adts, char *bs);

int put_VideoInfo(char* pData, unsigned int size, unsigned int framerate, AM_VIDEO_INFO* h264_info);

extern U32 mVideoDuration;
extern U32 mAudioDuration;


#endif	//__MP4_BUILDER_H__
