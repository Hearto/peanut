#include <stdio.h>
#include <ifaddrs.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/stat.h>

#include "hb_conn.h"
#include "hb_monitor.h"

const char * pidfile_path = "/var/run/dualctrl.pid";

int node_num = 0; /* 0: master, 1: slave */

void create_pidfile(void);
void daemon_init(void);
int is_server(void); 

int main(int argc,char **argv)
{
	log_init();
	daemon_init();
	create_pidfile();
	hb_conn_init();
	if(is_server() < 0) {
		node_num = 1;
	}
	log_debug(0,"start dualctrl ...");
	hb_monitor_start();
	return 0;
}

void daemon_init(void)
{
	pid_t pid;
        pid = fork();
        if (pid < 0) {
        	log_error("error starting daemon : %m");
                exit(-1);
        }
	else if (pid)
                exit(0);

        close(0);
        open("/dev/null", O_RDWR);
        dup2(0, 1); 
        dup2(0, 2); 

        setsid();

	if (umask(0) < 0) {
		log_error("failed to set umask 0 : %m");
		exit(-1);
	}

        if (chdir("/") < 0) {
                log_error("failed to set working dir to / : %m");
                exit(-1);
        }   
	log_debug(1,"daemon_init success");
}

void create_pidfile(void)
{
        int pid_fd;
	char pid_buf[64];
        pid_fd = open(pidfile_path, O_WRONLY|O_CREAT, 0644);
	if (pid_fd < 0) {
		log_error("unable to create pid file : %m");
                exit(-1);
	}
        if (lockf(pid_fd, F_TLOCK, 0) < 0) {
		log_error("unable to lock pid file : %m");
                exit(-1);
        }
 
        if (ftruncate(pid_fd, 0) < 0) {
                log_error("failed to ftruncate the PID file : %m");
                exit(-1);
        }
         
	sprintf(pid_buf, "%d\n", getpid());

        if (write(pid_fd, pid_buf, strlen(pid_buf)) < strlen(pid_buf)) {
                log_error("failed to write PID to PID file : %m");
                exit(-1);
        }

	lockf(pid_fd, F_ULOCK, 0);
	close(pid_fd);

	log_debug(1,"create pidfile success");
}

int is_server()
{
	struct ifaddrs *ifAddrStruct;
	void *tmpAddrPtr=NULL;
	char ip[16];
	getifaddrs(&ifAddrStruct);
	while (ifAddrStruct != NULL) {
		if (ifAddrStruct->ifa_addr->sa_family==AF_INET) {
			tmpAddrPtr=&((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
			inet_ntop(AF_INET, tmpAddrPtr, ip, INET_ADDRSTRLEN);
			if(strcmp(ip,st->priv1) == 0)
				return 0;
		}
		ifAddrStruct=ifAddrStruct->ifa_next;
	}
	//free ifaddrs
	freeifaddrs(ifAddrStruct);
	return -1;
}
