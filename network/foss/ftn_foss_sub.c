/*
* Copyright (C) 2012-2014 www.56.com email: jingchun.zhang AT renren-inc.com; jczhang AT 126.com ; danezhang77 AT gmail.com
* 
* 56VFS may be copied only under the terms of the GNU General Public License V3
*/


#define SYNCFILESTR "vfs_cmd=M_SYNCFILE"
#define SYNCFILESTRLEN 18
#include "ftn_foss.h"

static int cb_error_req(t_ftn_protocol_head *tkc, char *body, int fd)
{
	LOG(ftn_foss_log, LOG_ERROR, "error stat cmd  %s:%08X:%s!\n", __func__, tkc->cmd, ftn_cmd_str[tkc->cmd%FTN_CLIENT_ERR_REQ]);
	return 0;
}

static int cb_ftn_client_hb(t_ftn_protocol_head *tkc, char *body, int fd)
{
	tkc->cmd += 0x80000000;
	set_client_data(fd, (char *)tkc, sizeof(t_kshow_protocol_head));
	return 0;
}

static int cb_ftn_client_get_random(t_ftn_protocol_head *tkc, char *body, int fd)
{
	struct conn *curcon = &acon[fd];
	ftn_foss_peer *peer = curcon->user;
	t_foss_data_info *mbase = &(peer->mbase);
	mbase->identify = random()&0x1F;

	t_ftn_client_get_random_rsp resp;
	memset(&resp, 0, sizeof(resp));
	resp.retcode = RET_FTN_OK;
	resp.random = mbase->identify;
	
	tkc->cmd += 0x80000000;
	tkc->len = sizeof(resp);
	set_client_data(fd, (char *)tkc, sizeof(t_ftn_protocol_head));
	set_client_data(fd, (char *)&resp, sizeof(resp));
	
	return 0;
}

static cb_ftn_client_login(t_ftn_protocol_head *tkc, char *body, int fd)
{
	struct conn *curcon = &acon[fd];
	ftn_foss_peer *peer = curcon->user;
	t_foss_data_info *mbase = &(peer->mbase);
	
	t_ftn_client_login_rsp rsp;
	rsp.retcode = RET_FTN_OK;
	t_ftn_client_login_req* req = (t_ftn_client_login_req*)body;
	
	LOG(ftn_foss_log, LOG_DEBUG, "%s %d %s %d\n", __func__, __LINE__, req->mid, mbase->identify);
	
}

const ftn_foss_request_cb cball[FTN_CLIENT_ERR_REQ] = {
	cb_ftn_client_hb, //0
	cb_ftn_client_get_random,  //2
	cb_error_req,
	cb_error_req,
	cb_error_req,
	cb_error_req,
	cb_error_req,
	cb_error_req,
	cb_error_req,
	cb_error_req
};
int process_req(t_ftn_protocol_head *tkc, char *body, int fd)
{
	struct conn *curcon = &acon[fd];
	ftn_foss_peer *peer = curcon->user;
	if (tkc->cmd > FTN_CLIENT_LOGIN && tkc->cmd < FTN_CLIENT_ERR_REQ)
	{
		if (peer->s_type != S_TYPE_KSHOW_ACCESS_PEER)
		{
			LOG(ftn_foss_log, LOG_ERROR, "error stat in %s:%08X!\n", __func__, tkc->cmd);
			return -1;
		}
	}
	if (tkc->cmd & 0x80000000)
		return 0;
	if (peer->seq >= tkc->seq)
	{
		if (peer->s_type != S_TYPE_KSHOW_ACCESS_PEER)
		{
			if (WHITE_LIST_SEQ_ZERO[tkc->cmd%FTN_CLIENT_ERR_REQ] == 0)
				LOG(ftn_foss_log, LOG_ERROR, "seq error %d %x %x\n", tkc->cmd, peer->seq, tkc->seq);
		}
	}
	peer->seq = tkc->seq;
	if (peer->s_type != S_TYPE_KSHOW_ACCESS_PEER && tkc->cmd < FTN_CLIENT_ERR_REQ)
		LOG(ftn_foss_log, LOG_DEBUG, "%s %d %s\n", __func__, __LINE__, kshow_cmd_str[tkc->cmd%FTN_CLIENT_ERR_REQ]);
	return cball[tkc->cmd%FTN_CLIENT_ERR_REQ](tkc, body, fd);
}

