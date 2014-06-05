MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = test_flowmod

test_flowmod_SOURCES= \
    test_flowmod.cc
    
test_flowmod_LDADD = \
	../src/xcpd/libxcpd_wrap.la \
	-lpthread \
	-lrofl \
	-ldl
