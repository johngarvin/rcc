#!/bin/sh
aclocal-1.8
automake-1.8 --add-missing
libtoolize --automake
autoconf
