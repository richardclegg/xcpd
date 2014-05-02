MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = io processing 

#Shared stuff
SHARED_SRC=\
	of1x_cmm_mockup.c \
	../../../../../../../src/afa-imp/forwarding_module.cc\
	../../../../../../../src/io/iface_utils.cc \
	../../../../../../../src/pipeline-imp/memory.c \
	../../../../../../../src/pipeline-imp/pthread_lock.c \
	../../../../../../../src/pipeline-imp/pthread_atomic_operations.c \
	../../../../../../../src/io/pktin_dispatcher.cc \
	../../../../../../../src/io/iomanager.cc \
	../../../../../../../src/io/bufferpool.cc \
	../../../../../../../src/io/datapacketx86.cc \
	../../../../../../../src/io/datapacket_storage.cc \
	../../../../../../../src/io/packet_classifiers/packetclassifier.cc \
	../../../../../../../src/io/packet_classifiers/rofl_pktclassifier.cc \
	../../../../../../../src/io/packet_classifiers/static_pktclassifier.cc \
	../../../../../../../src/io/ports/ioport.cc \
	../../../../../../../src/io/ports/mockup/ioport_mockup.cc \
	../../../../../../../src/io/ports/mmap/ioport_mmapv2.cc \
	../../../../../../../src/io/ports/mmap/mmap_rx.cc \
	../../../../../../../src/io/ports/mmap/mmap_tx.cc \
	../../../../../../../src/io/ports/mmap/ioport_mmap.cc \
	../../../../../../../src/io/ports/mmap/mmap_int.cc \
	../../../../../../../src/io/ports/vlink/ioport_vlink.cc \
	../../../../../../../src/io/scheduler/epoll_ioscheduler.cc \
	../../../../../../../src/processing/processingmanager.cc \
	../../../../../../../src/util/time_utils.c \
	../../../../../../../src/bg_taskmanager.cc \
	../../../../../../../src/bg_taskmanager.h \
	platform_hooks_of1x_mockup.cc 
		
SHARED_LIBS=\
	-lrofl \
	-lrofl_pipeline \
	-lcppunit \
	-lpthread
	

#Simple portmockup with mockup matches	
test_portmockup_matchesmockup_SOURCES = \
	$(SHARED_SRC) \
	pipeline_packet_mockup.c \
	test_portmockup.cc
	
test_portmockup_matchesmockup_LDADD = \
	$(SHARED_LIBS)

#Simple portmockup with non-mockup matches	
test_portmockup_SOURCES = \
	$(SHARED_SRC) \
	../../../../../../../src/pipeline-imp/packet.cc \
	test_portmockup.cc
test_portmockup_LDADD = \
	$(SHARED_LIBS)
	

#Multiple portmockup with mockup matches	
test_portmockup_matchesmockup_multiport_SOURCES = \
	$(SHARED_SRC) \
	pipeline_packet_mockup.c \
	test_portmockup_multiport.cc
test_portmockup_matchesmockup_multiport_LDADD = \
	$(SHARED_LIBS)
	
#Multiple portmockup with non-mockup matches	
test_portmockup_multiport_SOURCES = \
	$(SHARED_SRC) \
	../../../../../../../src/pipeline-imp/packet.cc \
	test_portmockup_multiport.cc
test_portmockup_multiport_LDADD = \
	$(SHARED_LIBS)
	
#MMAP
test_portmmap_SOURCES = \
	$(SHARED_SRC) \
	../../../../../../../src/pipeline-imp/packet.cc \
	test_mmapport.cc
test_portmmap_LDADD = \
	$(SHARED_LIBS)

if DEBUG
#test for the expiration of buffers in the datapacket_storage
test_storage_packets_expiration_SOURCES = \
	$(SHARED_SRC) \
	../../../../../../../src/pipeline-imp/packet.cc \
	test_storage_packets_expiration.cc
test_storage_packets_expiration_LDADD = \
	$(SHARED_LIBS)
endif
	
#test for the hcl notifications for port events (add, delete & status change)
test_port_status_SOURCES = \
	$(SHARED_SRC) \
	../../../../../../../src/pipeline-imp/packet.cc \
	test_port_status.cc
test_port_status_LDADD = \
	$(SHARED_LIBS)
	
	
TESTS = \
	test_launcher.sh

check_PROGRAMS = \
	test_portmockup_matchesmockup\
	test_portmockup\
	test_portmockup_matchesmockup_multiport\
	test_portmockup_multiport\
	test_portmmap \
	test_port_status
	
if DEBUG
check_PROGRAMS +=test_storage_packets_expiration
endif


