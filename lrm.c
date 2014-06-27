#include "lrm.h"
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <sys/wait.h>

static LIST_HEAD(rs_list);

void set_rs_start(struct hb_rsinfo * rs);
void set_rs_stop(struct hb_rsinfo * rs);
int  get_rs_status(struct hb_rsinfo * rs);
void resource_parse(char * res,struct hb_rsinfo * rs);
void get_cmd_str(struct hb_rsinfo * rs,char * cmd,char * str);

void resource_table_init(void)
{
	struct hb_rsinfo * rs;
	FILE * rsfd;
	char buf[MAXLINE];


	if ((rsfd=fopen(RS_CFG,"r")) == NULL) {
		log_error("read rs file : %m");
		exit(-1);
	}	
	for (;;) {
		if(fgets(buf,MAXLINE,rsfd) == NULL) {
			if(ferror(rsfd))
				log_error("fgets : %m");
			break;
		}
		if(buf[0] == '#'){
			continue;
		}

		if(buf[strlen(buf) - 1] == '\n')
			buf[strlen(buf) - 1] = '\0';
		rs=(struct hb_rsinfo *)malloc(sizeof(struct hb_rsinfo));
		if (!rs) {
			log_error("malloc resource info: %m");
			fclose(rsfd);
			exit(-1);
		}
		resource_parse(buf,rs);	
		list_add(&rs->list,&rs_list);
	}
	fclose(rsfd);
}

void resource_parse(char * res,struct hb_rsinfo *rs)
{
	char name[MAXNAME]={'\0'};
	char para[MAXPARA]={'\0'};
	sscanf(res,"%[^ ]%*[ ]%[^ ]",name,para);
	strcpy(rs->name,name);
	strcpy(rs->para,para);
}

void  get_cmd_str(struct hb_rsinfo * rs,char * cmd, char * str)
{
	if(strcmp(rs->name,RES_IPAddr)==0)
		sprintf(str,"%s/%s %s %s",RS_PATH,rs->name,rs->para,cmd);
	else if (strcmp(rs->name,RES_iSCSITarget)==0)
		sprintf(str,"%s/%s %s",RS_PATH,rs->name,cmd);
	else
		*str='\0';
}

void set_rs_start(struct hb_rsinfo * rs)
{
	int status;
	char str[MAXLINE];
	get_cmd_str(rs,"start",str);
	status=system(str);
	if(WIFEXITED(status)){
		rs->status = WEXITSTATUS(status);
	}
}
void set_rs_stop(struct hb_rsinfo * rs)
{
	int status;
	char str[MAXLINE];
	get_cmd_str(rs,"stop",str);
	status=system(str);
	if(WIFEXITED(status)){
		rs->status = WEXITSTATUS(status);
		if(rs->status)
			rs->status = 1;
	}
}

int get_rs_status(struct hb_rsinfo * rs)
{
	int status;
	char str[MAXLINE];
	get_cmd_str(rs,"status",str);
	status=system(str);
	if(WIFEXITED(status)){
		if(WEXITSTATUS(status) == 0){
			rs->status = 0;
			log_debug(1,"resource %s is running\n",rs->name);
		}
		else {
			rs->status = 1;
			log_debug(1,"resource %s is stoped\n",rs->name);
			return 1;
		}
	}
	return 0;
}
int resource_monitor(int no_restart)
{
	struct hb_rsinfo * rs=NULL;
	struct list_head * p;
	list_for_each(p,&rs_list){
		rs=list_entry(p,struct hb_rsinfo,list);
		if( get_rs_status(rs) ){
			if( no_restart == 0 ){
				log_debug(0,"lrm resource %s is stoped,try to restart...",rs->name);
				set_rs_start(rs);
				if(get_rs_status(rs))
					log_debug(0,"lrm resource %s restart failed",rs->name);
			}
			if( rs->status )
				return 1;
		}
	}
	return 0;
}

void print_rs_status(int lrm_status)
{
	struct hb_rsinfo * rs=NULL;
	struct list_head * p;
	list_for_each(p,&rs_list)
	{
		rs=list_entry(p,struct hb_rsinfo,list);
		if (rs->status)
			rs->status = 1;
		if( rs->status == lrm_status ) {
			if(lrm_status == 0)
				log_debug(0,"lrm resoures %s is running",rs->name);
			else
				log_debug(0,"lrm resoures %s is stoped",rs->name);
		}
	}
}

