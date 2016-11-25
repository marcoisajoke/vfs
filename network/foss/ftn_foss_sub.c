/*
* Copyright (C) 2012-2014 www.56.com email: jingchun.zhang AT renren-inc.com; jczhang AT 126.com ; danezhang77 AT gmail.com
* 
* 56VFS may be copied only under the terms of the GNU General Public License V3
*/


#define SYNCFILESTR "vfs_cmd=M_SYNCFILE"
#define SYNCFILESTRLEN 18
#include "ftn_foss.h"

int process_req(t_kshow_protocol_head *tkc, char *body, int fd)
{
	struct conn *curcon = &acon[fd];
	t_kshow_peer *peer = curcon->user;
	if (tkc->cmd > KSHOW_CLIENT_LOGIN && peer->s_stat < S_RUN && tkc->cmd < KSHOW_WAR_2_SERVER_INIT_REQ)
	{
		if (peer->s_type != S_TYPE_KSHOW_ACCESS_PEER)
		{
			LOG(kshow_access_log, LOG_ERROR, "error stat in %s:%08X!\n", __func__, tkc->cmd);
			return -1;
		}
	}
	if (tkc->cmd & 0x80000000)
		return 0;
	if (peer->seq >= tkc->seq)
	{
		if (peer->s_type != S_TYPE_KSHOW_ACCESS_PEER)
		{
			if (WHITE_LIST_SEQ_ZERO[tkc->cmd%KSHOW_CLIENT_ERR_REQ] == 0)
				LOG(kshow_access_log, LOG_ERROR, "seq error %d %x %x\n", tkc->cmd, peer->seq, tkc->seq);
		}
	}
	peer->seq = tkc->seq;
	if (peer->s_type != S_TYPE_KSHOW_ACCESS_PEER && tkc->cmd < KSHOW_WAR_2_SERVER_INIT_REQ)
		LOG(kshow_access_log, LOG_DEBUG, "%s %d %s %d\n", __func__, __LINE__, kshow_cmd_str[tkc->cmd%KSHOW_CLIENT_ERR_REQ], peer->mbase.mid);
	return cball[tkc->cmd%KSHOW_CLIENT_ERR_REQ](tkc, body, fd);
}

