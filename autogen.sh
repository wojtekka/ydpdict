#!/bin/sh -e
echo "aclocal"
aclocal -I m4
echo "autoheader"
autoheader
echo "libtoolize"
libtoolize -c -f
echo "automake"
automake -a -c -f --foreign
echo "autoconf"
autoconf

test x$NOCONFIGURE = x && ./configure $*

