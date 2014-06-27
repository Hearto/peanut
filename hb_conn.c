#include <stdio.h>
#include "hb_conn.h"
#include <string.h>
#include <malloc.h>
#include <stdlib.h>

#define MAX_LINE 512

struct hb_setup * st;
LIST_HEAD(st_list);

void st_parse(char * str);
void hb_list_init(void);
int parse_recv_data(int len ,char * data);
int serial_write_hbmsg(void);
int ucast_write_hbmsg(void);
int serial_write_ctrlmsg(char * msg);
int ucast_write_ctrlmsg(char * msg);

/*
 * return  <=0 : return no message or ivalid msg
 *           1 : hb msg
 *           2 : ctrl msg
 */

int serial_read_data(char * data, int data_len);
int ucast_read_data(char * data,int data_len);

void hb_conn_init(void)
{
	char buf[MAX_LINE];
	FILE * stfd;

	st=(struct hb_setup *)malloc(sizeof(struct hb_setup));
	memset(st,0,sizeof(struct hb_setup));
	if (!st) {
		log_error("malloc st info : %m");
		exit(-1);
	}
	if((stfd = fopen(ST_CFG,"r")) == NULL){
			log_error("read hb.cf file: %m");
			exit(-1);
	}
	for (;;) {
		if(fgets(buf,MAX_LINE,stfd) == NULL) {
			if(ferror(stfd)){
				log_error("cf fgets: %m ");
				exit(-1);
			}
			break;
		}
		if(buf[0] == '#' || buf[0] == '\n'){
			continue;
		}

		if(buf[strlen(buf) - 1] == '\n')
			buf[strlen(buf) - 1] = '\0';
		st_parse(buf);
	}
	fclose(stfd);
	hb_list_init();
}

void st_parse(char * str)
{
	char name[MAX_LINE] = {'\0'};
	char para[MAX_LINE] = {'\0'};
	sscanf(str,"%[^ ]%*[ ]%[^ ]",name,para);
	if(strcmp(name,"priv1")==0)
		st->priv1 = strdup(para);
	else if(strcmp(name,"priv2") == 0)
		st->priv2 = strdup(para);	
	else if(strcmp(name,"serial") == 0)
		st->serial = strdup(para);
	else if(strcmp(name,"baud") == 0)
		st->baud = atoi(para);
	else if(strcmp(name,"re_send_time") == 0)
		st->re_send_time = atoi(para);
	else if(strcmp(name,"send_interval") == 0)
		st->send_interval = atoi(para);	
	else 
		log_debug(0,"invalid cf"); 
}

void hb_list_init(void)
{
	struct hb_stinfo * hb_st;
	if(strlen(st->serial) > 0){
		hb_st=(struct hb_stinfo *)malloc(sizeof(struct hb_stinfo));
		hb_st->name = HB_SERIAL_NAME;
		hb_st->status = NORMAL;
		hb_st->write_hbmsg = serial_write_hbmsg;
		hb_st->write_ctrlmsg = serial_write_ctrlmsg;
		hb_st->read_data = serial_read_data;
		list_add_tail(&hb_st->list,&st_list);
	}
	if(strlen(st->priv1) > 0 && strlen(st->priv2) > 0){
		hb_st=(struct hb_stinfo *)malloc(sizeof(struct hb_stinfo));
		hb_st->name = HB_UCAST_NAME;
		hb_st->status = NORMAL;
		hb_st->write_hbmsg = ucast_write_hbmsg;
		hb_st->write_ctrlmsg = ucast_write_ctrlmsg;
		hb_st->read_data = ucast_read_data;
		list_add_tail(&hb_st->list,&st_list);
	}
	if(list_empty(&st_list)){
		log_error("find no hb connect line");
		exit(-1);
	}
}


int serial_write_hbmsg(void)
{
	return serial_write(HB_DATA,sizeof(HB_DATA));
}

int ucast_write_hbmsg(void)
{
	return ucast_send(HB_DATA,sizeof(HB_DATA));
}

int serial_write_ctrlmsg(char * msg)
{
	char c_data[MAX_HB_MSG];
	memset(c_data,0,sizeof(c_data));
	sprintf(c_data,"%c%s",CTRL_MSG_HEAD,msg);
	return serial_write(c_data,strlen(c_data));
}

int ucast_write_ctrlmsg(char * msg)
{
	char c_data[MAX_HB_MSG];
	memset(c_data,0,sizeof(c_data));
	sprintf(c_data,"%c%s",CTRL_MSG_HEAD,msg);
	return ucast_send(c_data,strlen(c_data)); 
}

int serial_read_data(char * data, int data_len)
{
	int len = serial_read(data,data_len);
	return parse_recv_data(len,data);
}

int ucast_read_data(char * data,int data_len)
{
	int len = ucast_recv(data,data_len);
	return parse_recv_data(len,data);
}

int parse_recv_data(int len ,char * data)
{
	if( len <= 1) // no msg or invalid msg
		return len;
	if( data[0] == CTRL_MSG_HEAD ){
		if( data[1] >= '0' && data[1] <= '9')
			return CTRL_MSG;
	}
	return HB_MSG;
}
