dnl $Id$

AC_INIT(src/ydpdict.c)
AM_INIT_AUTOMAKE(ydpdict, 0.99.2)
AC_PREREQ(2.50)
AC_CONFIG_HEADERS(config.h)

AC_PROG_CC
AC_PROG_MAKE_SET
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_WORDS_BIGENDIAN

AC_CHECK_HEADERS([linux/soundcard.h])
AC_CHECK_HEADERS([locale.h])
AM_GNU_GETTEXT([external])

AC_CHECK_FUNCS([getopt_long])

AC_ARG_WITH(ncurses,
    [  --with-ncurses[=dir]    Compile with ncurses base directory],
    [CFLAGS="-I$dir/include" LIBS="-L$dir/lib"])

AC_CHECK_HEADER([ncursesw/ncurses.h], [have_ncursesw_ncurses_h=yes])
AC_CHECK_HEADER([curses.h], [have_curses_h=yes])
AC_CHECK_LIB([ncursesw], [initscr], [have_libncursesw=yes])

AC_MSG_CHECKING([for Unicode-aware ncurses])

if test "x$have_libncursesw" = "xyes" ; then
	if test "x$have_ncursesw_ncurses_h" = "xyes" ; then
		AC_DEFINE([HAVE_NCURSESW_NCURSES_H], [], [Define to 1 if you have the <ncursesw/ncurses.h> header file.])
		AC_MSG_RESULT([yes])
		LIBS="$LIBS -lncursesw"
	else
		if test "x$have_curses_h" = "xyes" ; then
			AC_DEFINE([HAVE_CURSES_H], [], [Define to 1 if you have the <curses.h> header file.])
			AC_MSG_RESULT([maybe])
			LIBS="$LIBS -lncursesw"
			AC_MSG_WARN([<ncursesw.h> not found, but some Linux distributions provide unicode-away <ncurses.h>, so let's try])
			ncurses_warning=yes
		else
			AC_MSG_ERROR([no])
		fi
	fi
else
	AC_MSG_ERROR([no])
fi

export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig"

PKG_CHECK_MODULES([YDPDICT], [libydpdict >= 0.99.0])
AC_SUBST([YDPDICT_CFLAGS])
AC_SUBST([YDPDICT_LIBS])

AC_ARG_WITH(dictdir,
    [  --with-dictdir=DIR      Databases are in DIR])

if test "x$with_dictdir" != "x"; then
	dictdir="$with_dictdir"
else
	# Substitute ${datarootdir}, ${prefix} etc.

	dictdir="${datadir}/ydpdict"

	save_prefix="$prefix"
	if test "X$prefix" = "XNONE"; then
		prefix="$ac_default_prefix"
	fi
	eval dictdir="$dictdir"
	eval dictdir="$dictdir"
	eval dictdir="$dictdir"
	eval dictdir="$dictdir"
	prefix="$save_prefix"
fi

AC_SUBST([dictdir])

AC_ARG_WITH(libao,
    [  --without-libao         Compile without libao])

if test "x$with_libao" != "xno"; then
	PKG_CHECK_MODULES([AO], [ao], [AC_DEFINE(HAVE_LIBAO, [], [Define to 1 if libao is available.])])
fi

AC_SUBST([AO_CFLAGS])
AC_SUBST([AO_LIBS])
AC_SUBST([HAVE_LIBAO])

CFLAGS="$CFLAGS -DHAVE_CONFIG_H"

AC_SUBST([VERSION])
AC_OUTPUT([Makefile src/Makefile contrib/Makefile ydpdict.conf po/Makefile.in])

if test "x$ncurses_warning" = "xyes" ; then
	AC_MSG_WARN([Build may fail because of ncurses headers ambiguity in some Linux distributions.])
fi
