SUBDIRS = src contrib po
DIST_SUBDIRS = src contrib po
EXTRA_DIST = autogen.sh autoclean.sh configure.ac ydpdict.1 ydpdict.pl.1 ydpdict.conf.in
man_MANS = ydpdict.1
ACLOCAL_AMFLAGS = -I m4
sysconf_DATA = ydpdict.conf

install-data-local:
	$(MKINSTALLDIRS) $(DESTDIR)$(dictdir)
	test -d $(DESTDIR)$(mandir)/pl/man1 || mkdir -p $(DESTDIR)$(mandir)/pl/man1
	$(INSTALL_DATA) ydpdict.pl.1 $(DESTDIR)$(mandir)/pl/man1/ydpdict.1

