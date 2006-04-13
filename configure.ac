AC_INIT(src/ydpdict.c)

AC_CONFIG_HEADER(src/config.h)
AC_PROG_CC
AC_PROG_MAKE_SET
AC_PROG_INSTALL
AC_WORDS_BIGENDIAN

AC_CHECK_HEADERS([linux/soundcard.h])
AC_CHECK_HEADERS([locale.h])
AC_CHECK_HEADERS([libintl.h])

AC_CHECK_FUNCS([getopt_long])

AC_ARG_WITH(ncurses,
    [  --with-ncurses[=dir]    Compile with ncurses base directory],
    [CFLAGS="-I$dir/include/ncursesw" LIBS="-L$dir/lib"])

AC_CHECK_HEADER([ncursesw/ncurses.h], [have_ncursesw_ncurses_h=yes])
AC_CHECK_LIB([ncursesw], [initscr], [have_ncursesw=yes])

AC_MSG_CHECKING([for Unicode-aware ncurses])

if test "x$have_ncursesw_ncurses_h" = "xyes" ; then
	if test "x$have_ncursesw" = "xyes" ; then
		LIBS="$LIBS -lncursesw"
		AC_MSG_RESULT([yes])
	else
		AC_MSG_ERROR([no])
	fi
else
	AC_MSG_ERROR([no])
fi

export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig"

PKG_CHECK_MODULES([YDPDICT], [libydpdict])
AC_SUBST([YDPDICT_CFLAGS])
AC_SUBST([YDPDICT_LIBS])

AC_ARG_WITH(libao,
    [  --without-libao         Compile without libao])

if test "x$with_libao" != "xno"; then
	PKG_CHECK_MODULES([AO], [ao], [AC_DEFINE(HAVE_LIBAO)])
fi

AC_SUBST([AO_CFLAGS])
AC_SUBST([AO_LIBS])
AC_SUBST([HAVE_LIBAO])

CFLAGS="$CFLAGS -DHAVE_CONFIG_H"

AC_SUBST([VERSION])
VERSION=$(grep '^#define VERSION' src/ydpconfig.h | sed -e 's/^[^"]\+"//' -e 's/".*//')
AC_OUTPUT([Makefile src/Makefile])

