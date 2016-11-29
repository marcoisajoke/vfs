/*
* Copyright (C) 2012-2014 www.56.com email: jingchun.zhang AT renren-inc.com; jczhang AT 126.com ; danezhang77 AT gmail.com
* 
* 56VFS may be copied only under the terms of the GNU General Public License V3
*/

#ifndef __FTN_FOSS_PROTOCOL_H_
#define __FTN_FOSS_PROTOCOL_H_

#pragma pack(1)
#define FTN_PROTOCOL_HEAD_LENGTH 12

typedef struct {
	uint32_t cmd;  //M  rsp = req + 0x80000000
	uint32_t seq;  //M  rsp = req
	uint32_t len;  //M body length
} t_ftn_protocol_head;

typedef struct {
	uint32_t errcmd;
	uint32_t errseq;
	uint8_t errcode;
	char errmsg[64];
} t_ftn_protocol_err_ret;

//FTN_CLIENT_GET_RANDOM begin
typedef struct {
	uint16_t retcode;
	uint32_t random;
}t_ftn_client_get_random_rsp;
//FTN_CLIENT_GET_RANDOM end

//FTN_CLIENT_LOGIN begin
typedef struct {
	char mid[128];
	uint64_t localip;
	uint32_t port;
}t_ftn_client_login_req;

typedef struct{
	uint32_t mid;
	uint64_t outip; //外ip
	uint64_t localip; //本地ip
	uint32_t port; //服务端口
} t_machine_info;

typedef struct{
	uint16_t retcode;
	uint16_t itemcount;
	t_machine_info item[0];
}t_ftn_client_login_rsp;

//FTN_CLIENT_LOGIN end
#pragma pack()

#endif
