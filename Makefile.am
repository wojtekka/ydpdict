SUBDIRS = src contrib po
DIST_SUBDIRS = src contrib po
EXTRA_DIST = autogen.sh autoclean.sh configure.ac ydpdict.1 ydpdict.pl.1 ydpdict.conf.in
man_MANS = ydpdict.1
ACLOCAL_AMFLAGS = -I m4

install-data-local:
	sed -s "s,\@datadir\@,$(sysconfdir)," < $(srcdir)/ydpdict.conf.in > $(srcdir)/ydpdict.conf
	$(INSTALL_DATA) $(srcdir)/ydpdict.conf $(DESTDIR)$(sysconfdir)/ydpdict.conf
	$(INSTALL_DATA) -d $(DESTDIR)$(datadir)/ydpdict

