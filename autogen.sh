#!/bin/sh
aclocal
automake --add-missing
libtoolize --automake
autoconf
