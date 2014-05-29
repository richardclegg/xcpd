MAINTAINERCLEANFILES = Makefile.in

SUBDIRS= xcpd_config hardware_management

#Main program
sbin_PROGRAMS = xcpd 
xcpd_SOURCES = xcpd.cc \
	control_manager.cc \
	cportvlan_mapper.cc \
	cportvlan_messagetranslator.cc \
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
	morpheus_nested.cc
	
xcpd_LDADD = \
	xcpd_config/libxdpd_xcpd_cfg.la\
	../xdpd/management/plugins/config/libxdpd_mgmt_config.la \
	../xdpd/management/libxdpd_mgmt.la \
	hardware_management/libxcpd_hardware.la \
	-lpthread \
	-lrofl \
	-ldl
