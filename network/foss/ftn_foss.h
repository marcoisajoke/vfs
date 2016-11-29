/*
* Copyright (C) 2012-2014 www.56.com email: jingchun.zhang AT renren-inc.com; jczhang AT 126.com ; danezhang77 AT gmail.com
* 
* 56VFS may be copied only under the terms of the GNU General Public License V3
*/

#ifndef __FTN_FOSS_SO_H
#define __FTN_FOSS_SO_H
#include "list.h"
#include "global.h"
#include "ftn_retcode.h"
#include "ftn_cmd.h"
#include "ftn_foss_protocol.h"
#include "vfs_init.h"
#include "common.h"
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <dirent.h>

enum SOCK_STAT {LOGOUT = 0, CONNECTED, LOGIN, HB_SEND, HB_RSP, IDLE, RECV_LAST, SEND_LAST, PREPARE_SYNCFILE, SYNCFILEING, SYNCFILE_POST, SYNCFILE_OK, PREPARE_SENDFILE, SENDFILEING, SENDFILE_OK};

typedef int (*ftn_foss_request_cb) (t_ftn_protocol_head *tkc, char *body, int fd);

extern const char *sock_stat_cmd[] ;

typedef struct {
	int recvlen;
	int datalen;
	time_t opentime;
	char outfile[256];
	uint32_t identify;
} t_foss_data_info;

typedef struct {
	list_head_t alist;
	list_head_t hlist;
	list_head_t cfglist;
	char ip[16];
	uint32_t cfgip;
	uint32_t con_ip;
	int fd;
	int local_in_fd; /* 接受vfs侧数据时，写到本地的文件句柄*/
	uint32_t hbtime;
	t_foss_data_info datainfo;
	uint8_t s_type;
	uint8_t server_stat;
	uint8_t role;
	uint8_t bk;
} ftn_foss_peer;

extern char *iprole[];
#endif
