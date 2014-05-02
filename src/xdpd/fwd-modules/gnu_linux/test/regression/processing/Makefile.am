MAINTAINERCLEANFILES = Makefile.in

SHARED_SRC=\
	../of1x_cmm_mockup.c \
	../../../../../../../../src/afa-imp/forwarding_module.cc\
	../../../../../../../../src/io/iface_utils.cc \
	../../../../../../../../src/pipeline-imp/memory.c \
	../../../../../../../../src/pipeline-imp/pthread_lock.c \
	../../../../../../../../src/pipeline-imp/pthread_atomic_operations.c \
	../../../../../../../../src/io/pktin_dispatcher.cc \
	../../../../../../../../src/io/iomanager.cc \
	../../../../../../../../src/io/bufferpool.cc \
	../../../../../../../../src/io/datapacketx86.cc \
	../../../../../../../../src/io/datapacket_storage.cc \
	../../../../../../../../src/io/packet_classifiers/packetclassifier.cc \
	../../../../../../../../src/io/packet_classifiers/rofl_pktclassifier.cc \
	../../../../../../../../src/io/packet_classifiers/static_pktclassifier.cc \
	../../../../../../../../src/io/ports/ioport.cc \
	../../../../../../../../src/io/ports/mockup/ioport_mockup.cc \
	../../../../../../../../src/io/ports/mmap/ioport_mmap.cc \
	../../../../../../../../src/io/ports/mmap/mmap_int.cc \
	../../../../../../../../src/io/ports/mmap/ioport_mmapv2.cc \
	../../../../../../../../src/io/ports/mmap/mmap_rx.cc \
	../../../../../../../../src/io/ports/mmap/mmap_tx.cc \
	../../../../../../../../src/io/ports/vlink/ioport_vlink.cc \
	../../../../../../../../src/io/scheduler/epoll_ioscheduler.cc \
	../../../../../../../../src/processing/processingmanager.cc \
	../../../../../../../../src/util/time_utils.c \
	../../../../../../../../src/bg_taskmanager.cc \
	../../../../../../../../src/bg_taskmanager.h \
	../../../../../../../../src/pipeline-imp/packet.cc 
	

if DEBUG
processingmanagertest_SOURCES= $(SHARED_SRC)\
			../platform_hooks_of1x_mockup.cc\
			processingmanagertest.cc 
			
processingmanagertest_LDADD= -lrofl -lcppunit -lpthread -lrofl_pipeline


check_PROGRAMS = processingmanagertest
TESTS = processingmanagertest 

endif
