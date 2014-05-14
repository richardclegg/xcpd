AUTOMAKE_OPTIONS = subdir-objects 1.5 foreign

MAINTAINERCLEANFILES = $(top_srcdir)/aclocal.m4 \
	src/ar-lib \
	src/config.guess \
	src/config.sub \
	src/configure \
	src/depcomp \
	src/install-sh \
	src/ltmain.sh \
	src/Makefile.in \
	src/missing

SUBDIRS = src doc test

install-exec-hook:
	@echo "Install"
uninstall-hook:
	@echo "Uninstall"

maintainer-clean-local:
	rm -rf src/m4

#AM_CPPFLAGS=-I/home/andreas/local/rofl
#AM_CFLAGS=-I/home/andreas/local/rofl
#AM_LDFLAGS=-L/home/andreas/local/lib
