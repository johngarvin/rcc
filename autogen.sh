#!/bin/sh
aclocal
libtoolize -f -c
automake -a -f -c
aclocal && autoconf && automake

