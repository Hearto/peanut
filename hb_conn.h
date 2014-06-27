#ifndef HB_CONN_H
#define HB_CONN_H

#include "list.h"

#define HB_SERIAL_NAME  "serial"
#define HB_UCAST_NAME   "ucast"

#define PORT 1111  //udp port
#define ST_DIR "usr/local/dualctrl/etc"
#define ST_CFG ST_DIR "/hb.cf"

#define HB_DATA    "abc"
#define CTRL_MSG_HEAD  '$'
#define MAX_HB_MSG   5
#define HB_MSG      100
#define CTRL_MSG    101

#define LOCAL_MSG_HEAD  '#'
#define MAX_LOCAL_MSG    5

enum HB_STATE {NORMAL = 0, SEND_ERR,RECV_ERR};

struct hb_setup {
	char  * priv1;
	char  * priv2;
	char  * serial;
	int     baud;
	int     re_send_time ;
	int     send_interval ;
};

struct hb_stinfo{
	struct    list_head list;
	char *    name;
	enum HB_STATE  status;
	int       (*write_hbmsg)();
	int       (*write_ctrlmsg)(char *);
	int       (*read_data)(char *,int);
};

extern struct hb_setup * st;
extern struct list_head st_list;


#endif
