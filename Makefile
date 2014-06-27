# Makefile for DualContrl -- monitor and heartbeat
# 

DualCtrl_PATH := /usr/local/dualctrl

objects = main.o \
	  log.o \
	  hb_conn.o \
	  lrm.o \
	  kernel_sock.o \
	  hb_monitor.o \
	  dc_alarm.o \
	  serial.o \
	  ucast.o \


DualCtrl : $(objects)
	$(CC)  -o DualCtrl $(objects) -lpthread

$(objects) : hb_conn.h
lrm.o : lrm.h
kernel_sock.o : kernel_sock.h
hb_monitor.o : hb_monitor.h  lrm.h kernel_sock.h

install : install_sbin install_scripts install_etc install_initd

install_sbin : ./DualCtrl
	@install -vD ./DualCtrl $(DESTDIR)/$(DualCtrl_PATH)/sbin/DualCtrl

install_scripts: scripts/ipaddr scripts/iscsi-target
	@install -vD scripts/ipaddr $(DESTDIR)/$(DualCtrl_PATH)/scripts/ipaddr
	@install -vD scripts/iscsi-target $(DESTDIR)/$(DualCtrl_PATH)/scripts/iscsi-target

install_etc: etc/hb.cf etc/monitor.cf
	@install -vD etc/hb.cf  $(DESTDIR)/$(DualCtrl_PATH)/etc/hb.cf
	@install -vD etc/monitor.cf  $(DESTDIR)/$(DualCtrl_PATH)/etc/monitor.cf

install_initd : scripts/initd/dualctrld
	@install -vD scripts/initd/dualctrld $(DESTDIR)/etc/init.d/dualctrld
     
.PHONY : clean
clean :
	rm DualCtrl $(objects)
