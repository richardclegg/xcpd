AUTOMAKE_OPTIONS = subdir-objects 1.5 foreign

ACLOCAL_AMFLAGS = -I m4

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

