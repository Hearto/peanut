/*
 * (C) 2004 - 2005 FUJITA Tomonori <tomof@acm.org>
 *
 * This code is licenced under the GPL.
 */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "kernel_sock.h"

#define CTL_DEVICE	"/dev/cache_ietctl"

int ctlfd;


void kernel_sock_init(void)
{
	FILE *f;
	char devname[256];
	char buf[256];
	int devn;

	if (!(f = fopen("/proc/devices", "r"))) {
		log_error("cannot open control path to the driver: %m");
		exit(-1);
	}

	devn = 0;
	while (!feof(f)) {
		if (!fgets(buf, sizeof (buf), f)) {
			break;
		}
		if (sscanf(buf, "%d %s", &devn, devname) != 2) {
			continue;
		}
		if (!strcmp(devname, "ietctl_cache")) {
			break;
		}
		devn = 0;
	}

	fclose(f);
	if (!devn) {
		log_error("cannot find ietctl_cache in /proc/devices - "
		     "make sure the kernel module is loaded");
		exit(-1);
	}

	unlink(CTL_DEVICE);
	if (mknod(CTL_DEVICE, (S_IFCHR | 0600), (devn << 8))) {
		log_error("cannot create %s: %m", CTL_DEVICE);
		exit(-1);
	}

	ctlfd = open(CTL_DEVICE, O_RDWR);
	if (ctlfd < 0) {
		log_error("cannot open %s: %m", CTL_DEVICE);
		exit(-1);
	}
}

int send_kernel_msg(char * msg)
{
	int err;	
	struct ctrl_msg_info *info;
	memset(info,0,sizeof(struct ctrl_msg_info));
	sprintf(info->msg,"%s",msg);
	err = ioctl(ctlfd, CTRL_MSG_SEND, info);
	if (err < 0 && errno == EFAULT)
		log_error("error calling ioctl CACHE_MODULE_GET: %m");
	return (err < 0) ? -errno : 0;
}
