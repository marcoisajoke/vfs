/*
* Copyright (C) 2012-2014 www.56.com email: jingchun.zhang AT renren-inc.com; jczhang AT 126.com ; danezhang77 AT gmail.com
* 
* 56VFS may be copied only under the terms of the GNU General Public License V3
*/

#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/syscall.h>
#include "common.h"
#include "global.h"
#include "vfs_so.h"
#include "myepoll.h"
#include "ftn_foss.h"
#include "util.h"
#include "acl.h"
#include "vfs_task.h"
#include "vfs_localfile.h"

typedef struct {
	int fd;
	char file[256];
	time_t last;
} t_client_stat;
t_client_stat c_stat;

extern const char *s_server_stat[STAT_MAX];
int vfs_voss_log = -1;

int voss_stat_interval = 120;

/* online list */
static list_head_t activelist;  //用来检测超时
static list_head_t online_list[256]; //用来快速定位查找
static list_head_t cfg_list[256]; //配置定位查找

#include "ftn_foss_sub.c"


int svc_init() 
{
	char *logname = myconfig_get_value("log_voss_logname");
	if (!logname)
		logname = "./voss_log.log";

	char *cloglevel = myconfig_get_value("log_voss_loglevel");
	int loglevel = LOG_NORMAL;
	if (cloglevel)
		loglevel = getloglevel(cloglevel);
	int logsize = myconfig_get_intval("log_voss_logsize", 100);
	int logintval = myconfig_get_intval("log_voss_logtime", 3600);
	int lognum = myconfig_get_intval("log_voss_lognum", 10);
	vfs_voss_log = registerlog(logname, loglevel, logsize, logintval, lognum);
	if (vfs_voss_log < 0)
		return -1;
	LOG(vfs_voss_log, LOG_NORMAL, "svc_init init log ok!\n");
	INIT_LIST_HEAD(&activelist);
	int i = 0;
	for (i = 0; i < 256; i++)
	{
		INIT_LIST_HEAD(&online_list[i]);
		INIT_LIST_HEAD(&cfg_list[i]);
	}

	memset(&c_stat, 0, sizeof(c_stat));
	c_stat.fd = -1;
	return check_stat_file();
}

int svc_initconn(int fd) 
{
	LOG(vfs_voss_log, LOG_TRACE, "%s:%s:%d\n", ID, FUNC, LN);
	uint32_t ip = getpeerip(fd);
	struct conn *curcon = &acon[fd];
	if (curcon->user == NULL)
		curcon->user = malloc(sizeof(ftn_foss_peer));
	if (curcon->user == NULL)
	{
		LOG(vfs_voss_log, LOG_ERROR, "malloc err %m\n");
		return RET_CLOSE_MALLOC;
	}
	ftn_foss_peer *peer;
	memset(curcon->user, 0, sizeof(ftn_foss_peer));
	peer = (ftn_foss_peer *)curcon->user;
	peer->hbtime = time(NULL);
	peer->sock_stat = CONNECTED;
	peer->fd = fd;
	peer->con_ip = ip;
	peer->local_in_fd = -1;
	ip2str(peer->ip, ip);
	INIT_LIST_HEAD(&(peer->alist));
	INIT_LIST_HEAD(&(peer->hlist));
	INIT_LIST_HEAD(&(peer->cfglist));
	list_move_tail(&(peer->alist), &activelist);
	list_add_head(&(peer->hlist), &online_list[ip&ALLMASK]);
	LOG(vfs_voss_log, LOG_TRACE, "a new fd[%d] init ok!\n", fd);
	return 0;
}
static int check_request(int fd, char* data, int len) 
{
	if (len < FTN_PROTOCOL_HEAD_LENGTH)
	{
		LOG(kshow_access_log, LOG_DEBUG, "%d head not long\n", fd);
		return 0;
	}

	t_ftn_protocol_head tkc;
	memcpy(&tkc, data, FTN_PROTOCOL_HEAD_LENGTH);

	if (tkc.len > (len - FTN_PROTOCOL_HEAD_LENGTH))
	{
		LOG(kshow_access_log, LOG_DEBUG, "%d body not long %08x %08x %08x\n", fd, tkc.cmd, tkc.len, len);
		return 0;
	}

	int clen = tkc.len + FTN_PROTOCOL_HEAD_LENGTH;

	LOG(kshow_access_log, LOG_TRACE, "%d process %08x %s\n", fd, tkc.cmd, kshow_cmd_str[tkc.cmd%KSHOW_CLIENT_ERR_REQ]);
	int ret = process_req(&tkc, data + FTN_PROTOCOL_HEAD_LENGTH, fd);
	if (ret)
	{
		if (ret < 1 || ret > KSHOW_PROTOCL_ERR_UNDEF)
			ret = KSHOW_PROTOCL_ERR_UNDEF;
		struct conn *curcon = &acon[fd];
		t_kshow_peer *peer = curcon->user;

		LOG(kshow_access_log, LOG_ERROR, "%s %d %08x %s %d mid = %d uid = %u\n", __func__, __LINE__, tkc.cmd, kshow_cmd_str[tkc.cmd%KSHOW_CLIENT_ERR_REQ], ret, peer->mbase.mid, peer->uid);
		send_errmsg_to_fd(fd, &tkc, ret);
		return clen;
	}

	return clen;
}
/*校验是否有一个完整请求*/
static int check_req(int fd)
{
	char *data;
	size_t datalen;
	if (get_client_data(fd, &data, &datalen))
	{
		LOG(kshow_access_log, LOG_TRACE, "fd[%d] no data!\n", fd);
		return RECV_ADD_EPOLLIN;  /*no suffic data, need to get data more */
	}
	int clen = check_request(fd, data, datalen);
	if (clen < 0)
	{
		LOG(kshow_access_log, LOG_DEBUG, "fd[%d] data error!\n", fd);
		return RECV_CLOSE;
	}
	if (clen == 0)
	{
		LOG(kshow_access_log, LOG_DEBUG, "fd[%d] data not suffic!\n", fd);
		return RECV_ADD_EPOLLIN;
	}
	consume_client_data(fd, clen);
	return RECV_SEND;
}

int svc_recv(int fd) 
{
	int ret = RECV_ADD_EPOLLIN;;
	struct conn *curcon = &acon[fd];
	ftn_foss_peer *peer = (ftn_foss_peer *) curcon->user;
	peer->hbtime = time(NULL);
	list_move_tail(&(peer->alist), &activelist);

	ret = RECV_ADD_EPOLLIN;;
	int subret = 0;
	while (1)
	{
		subret = check_req(fd);
		if (subret == -1)
			break;
		if (subret == RECV_CLOSE)
			return RECV_CLOSE;
		if (subret == RECV_ADD_EPOLLIN)
			return RECV_ADD_EPOLLIN;
	}
	return ret;
}

int svc_send(int fd)
{
	return SEND_ADD_EPOLLIN;
}

void svc_timeout()
{
	time_t now = time(NULL);
	int to = g_config.timeout * 10;
	ftn_foss_peer *peer = NULL;
	list_head_t *l;
	list_for_each_entry_safe_l(peer, l, &activelist, alist)
	{
		if (now - peer->hbtime > to)
		{
			LOG(vfs_voss_log, LOG_DEBUG, "timeout close %d [%lu:%lu]\n", peer->fd, now, peer->hbtime);
			do_close(peer->fd);
		}
	}
}

void svc_finiconn(int fd)
{
	struct conn *curcon = &acon[fd];
	if (curcon->user == NULL)
		return;
	ftn_foss_peer *peer = (ftn_foss_peer *) curcon->user;
	LOG(vfs_voss_log, LOG_NORMAL, "close %s\n", peer->ip);
	t_voss_data_info *datainfo = &(peer->datainfo);
	datainfo->opentime = 0;
	check_close_local(fd);
	if (peer->local_in_fd > 0)
		close(peer->local_in_fd);
	peer->local_in_fd = -1;
	list_del_init(&(peer->alist));
	list_del_init(&(peer->hlist));
	list_del_init(&(peer->cfglist));
	memset(curcon->user, 0, sizeof(ftn_foss_peer));
}
