
#ifndef __FTN_CMD__H_
#define __FTN_CMD__H_

enum FTN_CMD_TYPE {
	FTN_CLIENT_HB = 0,
	FTN_CLIENT_GET_RANDOM,
	FTN_CLIENT_LOGIN,
	FTN_BROADCAST_NEW_MACHINE,
	FTN_CS_LOGIN_FCS,
	FTN_ASK_SYNC_DIR,
	FTN_SYNC_TO_CS_ASK,
	FTN_SYNC_FILE_TO_CS,
	FTN_NOTIFY_FOSS_OPER,
	
	FTN_CLIENT_ERR_REQ
};

extern const char *ftn_cmd_str[FTN_CLIENT_ERR_REQ];

#pragma pack(1)
#define FTN_PROTOCOL_HEAD_LENGTH 12

typedef struct {
	uint32_t cmd;  //M  rsp = req + 0x80000000
	uint32_t seq;  //M  rsp = req
	uint32_t len;  //M body length
} t_ftn_protocol_head;

#pragma pack()

#endif
