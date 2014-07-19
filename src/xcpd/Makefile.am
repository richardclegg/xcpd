MAINTAINERCLEANFILES = Makefile.in

SUBDIRS= config hardware_management

noinst_LTLIBRARIES = libxcpd_wrap.la
libxcpd_wrap_la_SOURCES = \
	cportvlan_mapper.cc \
	control_manager.cc \
	csh_aggregate_stats.cc \
	csh_barrier.cc \
	csh_desc_stats.cc \
	csh_features_request.cc \
	csh_flow_mod.cc \
	csh_flow_stats.cc \
	csh_get_config.cc \
	csh_packet_in.cc \
	csh_packet_out.cc \
	csh_port_mod.cc \
	csh_port_stats.cc \
	csh_set_config.cc \
	csh_table_mod.cc \
	csh_table_stats.cc \
	morpheus.cc \
	morpheus_nested.cc \
	flow_entry_translate.cc
libxcpd_wrap_la_LIBADD = \
	config/libxcpd_config.la\
	hardware_management/libxcpd_hardware.la \
	-lpthread \
	-lrofl \
	-ldl
	
#Main program
sbin_PROGRAMS = xcpd 
xcpd_SOURCES = xcpd.cc 
	
xcpd_LDADD = \
	libxcpd_wrap.la \
	config/libxcpd_config.la\
	hardware_management/libxcpd_hardware.la \
	-lpthread \
	-lrofl \
	-ldl
