#ifndef LRM_H
#define LRM_H

#include <stdio.h>
#include "list.h"

#define MAXLINE 512*10
#define MAXNAME 15
#define MAXPARA 40

#define RS_DIR "/usr/local/dualctrl"
#define RS_CFG RS_DIR "/etc/monitor.cf"

#define RS_SCRIPT_DIR  RS_DIR "/scripts"
#define iSCSITarget_DIR  RS_SCRIPT_DIR "/iscsi-target"


#define RES_IPAddr        "ipaddr"
#define RES_iSCSITarget   "iscsi-target"

#define RS_PATH RS_SCRIPT_DIR
//const char * RS_PATH=".";
//struct list_head;


struct hb_rsinfo{
	struct list_head list;
	size_t status;
	char   name[MAXNAME];
	char   para[MAXPARA];
};

void resource_table_init(void); 
int resource_monitor(int no_restart);
void print_rs_status(int lrm_status);
#endif
