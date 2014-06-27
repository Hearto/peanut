/*
 * kernel_sock.h - connect to cache
 */

#ifndef _KERNEL_SOCK_H
#define _KERNEL_SOCK_H

#define MAX_MSG_LEN    10

struct ctrl_msg_info {
	char msg[MAX_MSG_LEN];
};

#define CTRL_MSG_SEND 	    	 _IOWR('i', 25, struct ctrl_msg_info)


void kernel_sock_init(void);
int send_kernel_msg(char * msg);

#endif
