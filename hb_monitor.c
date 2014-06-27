#include <stdio.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "lrm.h"
#include "hb_monitor.h"
#include "hb_conn.h"
#include "kernel_sock.h"

static void * rs_monitor();
static void * hb_conn();

static void * hb_send();
static void * hb_recv();

void sys_handle(char * msg);

int use_ucast = 0;
struct hb_stinfo * hb_st = NULL;
struct list_head * list_p;

void hb_monitor_start(void)
{
	int ret;
	pthread_t monitor_id;
        pthread_t conn_id;

	use_ucast = 0;
	list_for_each(list_p,&st_list){
		hb_st = list_entry(list_p,struct hb_stinfo,list);
		if(hb_st->status == NORMAL)
			break;
	}

	resource_table_init();

	serial_init(); 

	ucast_init();

//	kernel_sock_init();

	if(pthread_create(&monitor_id, NULL , rs_monitor , NULL) != 0){
		perror("create monitor pthread");
		exit(1);
	}

	if(pthread_create(&conn_id, NULL, hb_conn, NULL) != 0){
		log_error("create conn pthread: %m");
		exit(-1);
	}

	pthread_join(monitor_id, NULL);
	pthread_join(conn_id, NULL);
}


static void * rs_monitor()
{
	int status;
	int lrm_status = 0;
	char l_data[MAX_LOCAL_MSG];
	for(;;){
		status = lrm_status;
		if( resource_monitor( lrm_status ) == 0)
			lrm_status = 0;
		else 
			lrm_status = 1;
		if( status != lrm_status ){
			print_rs_status(lrm_status);
			memset(l_data,0,MAX_LOCAL_MSG);
			if (lrm_status == 0)
				sprintf(l_data,"%c%d",LOCAL_MSG_HEAD,LRM_NORMAL);
			else
				sprintf(l_data,"%c%d",LOCAL_MSG_HEAD,LRM_ERR);
			sys_handle(l_data);
		}
		sleep(MONITOR_INTERVAL);
	}	
}

static void * hb_send()
{
	struct hb_stinfo * hs = NULL;
	struct list_head * p;
	for(;;){
		if( list_p != (&st_list)){
			if(hb_st->write_hbmsg() <= 0){
				hb_st->status = SEND_ERR;
			}
		}
		else{
			list_for_each(p,&st_list){
				hs = list_entry(p,struct hb_stinfo,list);
				hs->write_hbmsg();
			}
		}
		sleep(st->send_interval);
	}
}

static void * hb_recv()
{
	int len;
	char m_data[MAX_HB_MSG];
	int hb_status = HB_NORMAL;
	struct hb_stinfo * hs = NULL;
	struct list_head * p;
	for(;;){
		if(list_p != (&st_list)){
			if(hb_st->status == SEND_ERR){
				log_debug(0,"%s send err",hb_st->name);
				list_p = list_p->next;
				if(list_p == (&st_list))
					continue;
				hb_st = list_entry(list_p,struct hb_stinfo,list);
				log_debug(0,"use %s instead",hb_st->name);
				sleep(st->send_interval);
				continue;
			}
			len = hb_st->read_data(m_data,MAX_HB_MSG);
			if(len <= 0){
				log_debug(0,"%s receive failed",hb_st->name);
				hb_st->status = RECV_ERR;
				list_p = list_p->next;
				if(list_p == (&st_list))
					continue;
				hb_st = list_entry(list_p,struct hb_stinfo,list);
				log_debug(0,"use %s instead",hb_st->name);
				sleep(st->send_interval);
			}
			else if(len == CTRL_MSG) {
				sys_handle(m_data);
			}
			else
				;
		}
		else {
			if(hb_status == HB_NORMAL){
				hb_status = HB_ERR;
				log_debug(0,"heartbeat connect failed,check your link");
				memset(m_data,0,sizeof(m_data));
				sprintf(m_data,"%c%d",CTRL_MSG_HEAD,HB_ERR);
				sys_handle(m_data);
				dc_alarm();
			}
			else {
				list_for_each(p,&st_list){
					hs = list_entry(p,struct hb_stinfo,list);
					if(hs->read_data(m_data,MAX_HB_MSG) > 0){
						hs->status = NORMAL;
						hb_status = HB_NORMAL;
						list_p = p;
						hb_st = hs;
						log_debug(0,"%s has recovered",hs->name);
						break;
					}
				}
				if(p == (&st_list)){
					dc_alarm();
					sleep(st->send_interval);
				}
			}
		}
	}
}


static void * hb_conn()
{
	pthread_t hb_send_id;
	pthread_t hb_recv_id;

	if(pthread_create(&hb_send_id, NULL, hb_send, NULL) != 0){
		log_error("create hb send pthread: %m");
		exit(1);
	}
	if(pthread_create(&hb_recv_id, NULL, hb_recv, NULL) != 0){
		log_error("create hb recv pthread: %m");
		exit(1);
	}
	pthread_join(hb_send_id,NULL);
	pthread_join(hb_recv_id,NULL);	
}

void sys_handle(char * msg)
{
	char data[MAX_HB_MSG];
	memset(data,0,sizeof(data));
	if(NULL == msg){
		log_warning("invalid sysrem msg");
		return;
	}
	if( msg[0] == LOCAL_MSG_HEAD) {
	//	sscanf(msg,"%*c%s",data);
		hb_st->write_ctrlmsg(data);
	}
	else if ( msg[0] == CTRL_MSG_HEAD){
//		send_kernel_msg(msg);
	}
	else
		;
}

