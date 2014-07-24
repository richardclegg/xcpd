MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = test_flowmod \
			test_datapath

test_flowmod_SOURCES= \
    test_flowmod.cc
    
test_flowmod_LDADD = \
	../src/xcpd/libxcpd_wrap.la \
	-lpthread \
	-lrofl \
	-ldl
	
test_datapath_SOURCES= \
    test_datapath.cc
    
test_datapath_LDADD = \
	../src/xcpd/libxcpd_wrap.la \
	-lpthread \
	-lrofl \
	-ldl
