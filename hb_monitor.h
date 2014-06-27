#ifndef HB_MONITOR_H
#define HB_MONITOR_H

#define MONITOR_INTERVAL 5


#define _10G_NORMAL  0 
#define _10G_ERR     1  
#define LRM_NORMAL   2 
#define LRM_ERR      3

#define HB_NORMAL      4 
#define HB_ERR         5

extern int node_num;

void  hb_monitor_start(void);

#endif
