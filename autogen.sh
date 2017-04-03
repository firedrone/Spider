#!/bin/sh

if [ -x "`which auspidereconf 2>/dev/null`" ] ; then
  opt="-i -f -W all,error"

  for i in $@; do
    case "$i" in
      -v)
        opt="${opt} -v"
        ;;
    esac
  done

  exec auspidereconf $opt
fi

set -e

# Run this to generate all the initial makefiles, etc.
aclocal -I m4 && \
	autoheader && \
	autoconf && \
	automake --add-missing --copy
