#!/bin/sh
echo "aclocal"
aclocal -I m4 || exit 1
echo "autoheader"
autoheader || exit 1
echo "libtoolize"
libtoolize -c -f || exit 1
echo "automake"
automake -a -c -f --foreign || exit 1
echo "autoconf"
autoconf || exit 1

test x$NOCONFIGURE = x && ./configure $*

