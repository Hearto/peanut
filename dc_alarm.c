#include   <sys/io.h> 
#include   <unistd.h> 

#define ALARM_TIME  3

unsigned int freq[]={ 330,392,330,294,330,392, 
		      330,394,330,330,392,330, 
	    	      294,262,294,330,392,294, 
		      262,262,220,196,196,220, 
		      262,294,330,262,0 }; 

unsigned int time[]={ 50 ,50 ,50 ,25 ,25 ,50 , 
		      25 ,25 ,100,50 ,50 ,25 , 
		      25 ,50 ,50 ,25 ,25 ,100, 
		      50 ,25 ,25 ,50 ,25 ,25 , 
		      50 ,25 ,25 ,100 }; 

unsigned int freq_alert[]={ 2000,2400,0 }; 
unsigned int time_alert[]={ 50,60 };
 
void  play(unsigned int* freq,unsigned int* delay); 
void  Stop(void);


void dc_alarm(void) 
{ 
	int i;
	for(i = 0; i < ALARM_TIME;i++) 
	{ 
		play(freq_alert,time_alert); 
	} 
	Stop();
} 

void play(unsigned int* freq,unsigned int* time) 
{ 
	int   i; 
	for(i = 0;freq[i] != 0;i++) 
	{ 
		speaker(freq[i],time[i]); 
	} 
} 

int speaker(unsigned int freq,unsigned int delay) 
{ 
	static int flag = 0;
	static int bit; 
	if(flag == 0) 
	{ 
		flag = 1; 
		iopl(3); 
	} 
	outb(0xb6,0x43); 
	outb((freq & 0xff),0x42); 
	outb((freq >> 8),0x42); 
	bit=inb(0x61); 
	outb(3 | bit,0x61); 
	usleep(10000 * delay); 
	outb(0xfc | bit,0x61); 
}

//如果蜂鸣器在鸣叫时程序被ctrl+c或者其他情况意外终止,蜂鸣器就会一直不停的叫

//下面这个函数让蜂鸣器不发声

void Stop(void) 
{  
	static int flag=0; 
	if(flag == 0) 
	{ 
		flag = 1; 
		iopl(3); 
	} 
	outb(0xfc,0x61); 
	return;
}  
