bin_PROGRAMS = ydpdict
ydpdict_SOURCES = ydpconfig.c ydpconfig.h ydpsound.c ydpsound.h ydpdict.c xmalloc.c xmalloc.h adpcm.c adpcm.h
ydpdict_CFLAGS = -Wall @CFLAGS@ @YDPDICT_CFLAGS@ @AO_CFLAGS@ -DSYSCONFDIR=\"@sysconfdir@\" -DLOCALEDIR=\"$(localedir)\"
ydpdict_LDADD = -lm @LIBS@ @YDPDICT_LIBS@ @AO_LIBS@ @LTLIBINTL@
