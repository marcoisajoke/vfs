#include "kshow_cmd.h"
#include "kshow_pk.h"

const char *ftn_cmd_str[FTN_CLIENT_ERR_REQ] = {
	"FTN_CLIENT_HB",
	"FTN_CLIENT_GET_RANDOM",
	"FTN_CLIENT_LOGIN",
	"FTN_BROADCAST_NEW_MACHINE",
	"FTN_CS_LOGIN_FCS",
	"FTN_ASK_SYNC_DIR",
	"FTN_SYNC_TO_CS_ASK",
	"FTN_SYNC_FILE_TO_CS",
	"FTN_NOTIFY_FOSS_OPER"
};

const char *ftn_protocol_errmsg[FTN_PROTOCL_ERRTYPE_MAX] = {
	"ok",
	"error body len",
	"error option",
	"user must login first",
	"FTN_PROTOCL_ERR_UNDEF"
};
