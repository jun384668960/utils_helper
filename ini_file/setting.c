#include "disp_client.h"
#include "gss_setting.h"
#include "inifile.h"
#include <stdlib.h>
#include <stdio.h>
#include "log.h"


gss_setting setting;

#define DEFAULT_PORT (6000)

#define MIN_UNKNOWN_TCP_TIMEOUT (5)

#define ACCEPT_PERIOD_LIMIT (1000)

#define MAX_AV_CONN_COUNT (2048)

#define DEFAULT_DB_THREAD_COUNT (16)

#define DEFAULT_MW_TIME 350

#define DEFAULT_MAX_PUSH_BITRATE (4096) //kbps, bitrate 

 int load_gss_setting(const char* ini_file)
{
	int ret;

	strcpy(setting.ini_file_name, ini_file);

	ret = read_profile_string("common", "log_file", "", setting.log_file_name, sizeof(setting.log_file_name), ini_file);
	if(ret == 0 || setting.log_file_name[0]=='\0')
		sprintf(setting.log_file_name, "%s/gss.log", get_exe_path());
	set_log_file_name(setting.log_file_name);

	setting.log_level = read_profile_int("common", "log_level", LOG_LEVEL_INFO, ini_file);
	set_log_level(setting.log_level);

	ret = read_profile_string("common", "listen_ip", "", setting.listen_ip, sizeof(setting.listen_ip), ini_file);
	if(ret == 0)
		setting.listen_ip[0] = '\0';

	ret = read_profile_string("common", "external_ip", "", setting.external_ip, sizeof(setting.external_ip), ini_file);
	if(ret == 0)
		setting.external_ip[0] = '\0';


	setting.listen_port = read_profile_int("common", "listen_port", DEFAULT_PORT, ini_file);
	if(setting.listen_port<=0 || setting.listen_port>65535)
	{
		LOG(LOG_LEVEL_ERROR, "ini %s, listen port is invalid.", ini_file);  
		return -1;
	}

	setting.unknown_tcp_timeout = read_profile_int("common", "unknown_tcp_timeout", MIN_UNKNOWN_TCP_TIMEOUT, ini_file);
	if(setting.unknown_tcp_timeout < MIN_UNKNOWN_TCP_TIMEOUT)
		setting.unknown_tcp_timeout = MIN_UNKNOWN_TCP_TIMEOUT;

	setting.accept_period_limit = read_profile_int("common", "accept_period_limit", ACCEPT_PERIOD_LIMIT, ini_file);
	if(setting.accept_period_limit < ACCEPT_PERIOD_LIMIT)
		setting.accept_period_limit = ACCEPT_PERIOD_LIMIT;

	setting.max_av_conn_count = read_profile_int("common", "max_av_conn_count", MAX_AV_CONN_COUNT, ini_file);
	if(setting.max_av_conn_count < MAX_AV_CONN_COUNT)
		setting.max_av_conn_count = MAX_AV_CONN_COUNT;	

	setting.enable_gop_cache = read_profile_int("common", "enable_gop_cache", 1, ini_file) == 1;

	setting.mw_time = read_profile_int("common", "mw_time", DEFAULT_MW_TIME, ini_file);

	setting.server_id = read_profile_int("common", "server_id", 0, ini_file);

	setting.async_db_thread_count = read_profile_int("db", "async_db_thread_count", DEFAULT_DB_THREAD_COUNT, ini_file);

	setting.max_push_bitrate = read_profile_int("common", "max_push_bitrate", DEFAULT_MAX_PUSH_BITRATE, ini_file);
	if(setting.max_push_bitrate < DEFAULT_MAX_PUSH_BITRATE)
		setting.max_push_bitrate = DEFAULT_MAX_PUSH_BITRATE;	

	setting.so_recvbuf = read_profile_int("common", "so_recvbuf", -1, ini_file);

	//ai
	setting.ai_enable = 0;
	memset(setting.ai_server,0,MAX_AI_SERVER_COUTN*IP_ADDR_LEN*sizeof(char));
	memset(setting.ai_port,0,MAX_AI_SERVER_COUTN*sizeof(int));
	memset(setting.ai_serveruseed,0,MAX_AI_SERVER_COUTN*sizeof(int));
	char pserver[2048] = {0};
	ret = read_profile_string("common", "ai_server", "", pserver, sizeof(pserver), ini_file);
	if(ret == 0)
	{
		LOG(LOG_LEVEL_ERROR, "ini %s, ai server is invalid.", ini_file);  
	}
	else
	{
		int nCount = 0;
		char* ptr = strtok(pserver,";");
		while (ptr != NULL && nCount < MAX_AI_SERVER_COUTN)
		{
			strcpy(setting.ai_server[nCount++],ptr);
			ptr = strtok(NULL,";");
		}
		char pport[128] = {0};
		ret = read_profile_string("common", "ai_port", "", pport, sizeof(pport), ini_file);
		if(ret == 0)
		{
			LOG(LOG_LEVEL_ERROR, "ini %s, ai port is invalid.", ini_file);  
			setting.ai_enable = 0;
		}
		else
		{
			char* ptr = strtok(pport,";");
			int tmpnCount = 0;
			while (ptr != NULL && tmpnCount < nCount)
			{
				setting.ai_port[tmpnCount++] = atoi(ptr);
				ptr = strtok(NULL,";");
			}
			nCount = tmpnCount;
		}
		if(nCount > 0)
		{
			setting.ai_enable = 1;
			for (int i = 0; i < nCount; i++)
			{
				LOG(LOG_LEVEL_INFO, "AI Server information is ok, %s:%d",setting.ai_server[i],setting.ai_port[i]);  
			}
		}
	}
	
	return 0;
}


void reload_disp_svr_info(struct disp_svr_info* disp_svr, int* count)
{
	int i=0;

	if(count)
		*count = read_profile_int("dispatch", "count", 0, setting.ini_file_name);

	for(i=0; i<*count; i++)
	{
		char key[64];
		sprintf(key, "addr_%d", i+1);
		read_profile_string("dispatch", key, "", disp_svr[i].addr, sizeof(disp_svr->addr), setting.ini_file_name);

		sprintf(key, "port_%d", i+1);
		disp_svr[i].port = read_profile_int("dispatch", key, 0, setting.ini_file_name);
	}
}