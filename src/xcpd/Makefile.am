MAINTAINERCLEANFILES = Makefile.in

SUBDIRS= xcpd_config

#Main program
sbin_PROGRAMS = xcpd 
xcpd_SOURCES = xcpd.cc control_manager.cc
xcpd_LDADD = \
	xcpd_config/libxdpd_xcpd_cfg.la\
	../xdpd/management/plugins/config/libxdpd_mgmt_config.la \
	../xdpd/management/libxdpd_mgmt.la \
	-lpthread \
	-lrofl \
	-ldl

