#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <sys/types.h>  
#include <sys/stat.h>  
#include <sys/select.h>
#include <sys/time.h>
#include <fcntl.h> 
#include <termios.h>
#include <errno.h>  
#include "hb_conn.h"

void serial_init(void);
int serial_write(char * data,int data_len);
int serial_read(char * data,int data_len);

static int get_baud(int baud);

int serial_fd = 0; 

void serial_init(void)
{
	struct termios options;  
	serial_fd = open(st->serial, O_RDWR | O_NOCTTY | O_NDELAY);  
	if (serial_fd < 0) {  
		log_error("open serial: %m");  
		exit(-1);  
	}  

	 //判断串口的状态是否为阻塞状态                            
	if(fcntl(serial_fd, F_SETFL, 0) < 0) {
		log_error("fcntl failed: %m");
		exit(-1);
	}     

	//测试是否为终端设备    
//	if(0 == isatty(STDIN_FILENO)) {
//		log_debug(0,"standard input is not a terminal device");
//		exit(-1);
//	}   	

	tcgetattr(serial_fd, &options);

	options.c_cflag |= (CLOCAL | CREAD);//设置控制模式状态，本地连接，接收使能  
	options.c_cflag &= ~CSIZE;//字符长度，设置数据位之前一定要屏掉这个位  
	options.c_cflag &= ~CRTSCTS;//无硬件流控  
	options.c_cflag |= CS8;//8位数据长度  
	options.c_cflag &= ~CSTOPB;//1位停止位  
	options.c_iflag |= IGNPAR;//无奇偶检验位  
	options.c_oflag = 0; //输出模式  
	options.c_lflag = 0; //不激活终端模式  
	cfsetospeed(&options, get_baud(st->baud));//设置波特率  

	/**3. 设置新属性，TCSANOW：所有改变立即生效*/  
	tcflush(serial_fd, TCIFLUSH);//溢出数据可以接收，但不读  
	tcsetattr(serial_fd, TCSANOW, &options);    
}

static int get_baud(int baud)
{
	int s_baud = 0;
	switch(baud){
		case 9600:
			s_baud = B9600;
			break;
		case 19200:
			s_baud = B19200;
			break;
		case 38400:
			s_baud = B38400;
			break;
		case 115200:
			s_baud = B115200;
			break;
		default:
			s_baud = B115200;
			break;
	}
	return s_baud;
}


int serial_write(char * data,int data_len)
{
	int len = 0;  
	len = write(serial_fd,data,data_len);  

	if(len < 0) {
		log_error("serial send: %m");
	}
	else { 
		log_debug(1,"serial send data: %s\n",data);
	}

	if(len == data_len) {  
		return len;  
	}
	else {  
		tcflush(serial_fd, TCOFLUSH);//TCOFLUSH刷新写入的数据但不传送  
		//exit(-1);  
	}  
	return 0; 
}

int serial_read(char * data ,int data_len)
{	  
	int len = 0;
	int ret = 0;    
	fd_set fs_read;
	struct timeval tv_timeout;
	FD_ZERO(&fs_read);  
	FD_SET(serial_fd, &fs_read);  
	tv_timeout.tv_sec  = (st->re_send_time * st->send_interval);
	tv_timeout.tv_usec = 0;
	ret = select(serial_fd+1, &fs_read, NULL, NULL, &tv_timeout);
	if(ret < 0) {
		log_error("serial select: %m");
		exit(-1);
	}

	if (FD_ISSET(serial_fd, &fs_read)) {  
		len = read(serial_fd, data, data_len);
		if(len < 0){ 
			log_error("serial recv data: %m");
		}
		else
			log_debug(1,"serial recv data: %s\n",data);  
		return len;  
	}
	return -1;;
}

