#include <stdio.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include "hb_conn.h"

void ucast_init(void);
int ucast_send(const char * data,int data_len);
int ucast_recv(char * data ,int data_len);

int sockfd;
struct sockaddr_in addr;

extern int node_num;

void ucast_init(void)
{
	int addr_len = sizeof(struct sockaddr_in);  

	struct timeval s_tout = {st->send_interval,0};
	struct timeval r_tout = {st->send_interval * st->re_send_time,0};

	/*建立socket*/  
	if((sockfd=socket(AF_INET,SOCK_DGRAM,0))<0){  
		log_error("create socket: %m"); 
		exit(-1);
	}  
	/*填写sockaddr_in 结构*/  
	memset(&addr,0,sizeof(addr));
	addr.sin_family=AF_INET;  
	addr.sin_port=htons(PORT);  
	if(node_num)	
		addr.sin_addr.s_addr = inet_addr(st->priv2); /* slave */
        else
		addr.sin_addr.s_addr = inet_addr(st->priv1); /* master */	
	if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr))<0){  
		perror("bind socket: %m");  
		exit(-1);
	}  
	
	if(node_num == 0)  //use for send	
		addr.sin_addr.s_addr = inet_addr(st->priv2); /* slave */
        else
		addr.sin_addr.s_addr = inet_addr(st->priv1); /* master */	

	if(setsockopt(sockfd,SOL_SOCKET,SO_SNDTIMEO,(char *)&s_tout,sizeof(s_tout)) < 0){
		log_error("setsockopt send: %m");
		exit(-1);
	}
	if(setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(char *)&r_tout,sizeof(r_tout)) < 0){
		log_error("setsockopt recv: %m");
		exit(-1);
	}
}

int ucast_send(const char * data,int data_len)
{
	int len;
	int addr_len = sizeof(struct sockaddr_in);
	len = sendto(sockfd,data,data_len,0,(struct sockaddr *)&addr,addr_len);

	if(len < 0 ){
		log_error("ucast send: %m");
	}
	else if(len < data_len){
		log_debug(0,"ucast send data incompletely");
	}
	else {
		log_debug(1,"ucast send data: %s\n",data);
	}
		
	return len;
}

int ucast_recv(char * data ,int data_len)
{
	int len = 0;
	int addr_len = sizeof(struct sockaddr_in);
	memset(data,0,data_len);
	len=recvfrom(sockfd,data,data_len, 0 , (struct sockaddr *)&addr ,&addr_len);
	if(len > 0) {
		log_debug(1,"                        ucast recv data: %s\n",data);
	}
	else {
		log_debug(1,"ucast recv: %m");
	}
	return len;
}

