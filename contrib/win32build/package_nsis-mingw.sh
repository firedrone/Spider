#!/bin/sh
#
# ===============================================================================
# package_nsis-ming.sh is distributed under this license:

# Copyright (c) 2006-2007 Andrew Lewman
# Copyright (c) 2008 The Spider Project, Inc.

# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:

#     * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.

#     * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.

#     * Neither the names of the copyright owners nor the names of its
# contribuspiders may be used to endorse or promote products derived from
# this software without specific prior written permission.

# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# ===============================================================================

# Script to package a Spider installer on win32.  This script assumes that
# you have already built Spider, that you are running msys/mingw, and that
# you know what you are doing.

# Start in the spider source directory after you've compiled spider.exe
# This means start as ./contrib/win32build/package_nsis-mingw.sh

rm -rf win_tmp
mkdir win_tmp
mkdir win_tmp/bin
mkdir win_tmp/contrib
mkdir win_tmp/doc
mkdir win_tmp/doc/spec
mkdir win_tmp/doc/design-paper
mkdir win_tmp/doc/contrib
mkdir win_tmp/src
mkdir win_tmp/src/config
mkdir win_tmp/tmp

cp src/or/spider.exe win_tmp/bin/
cp src/tools/spider-resolve.exe win_tmp/bin/
cp contrib/win32build/spider.ico win_tmp/bin/
cp src/config/geoip win_tmp/bin/
strip win_tmp/bin/*.exe

# There is no man2html in mingw.  
# Maybe we should add this into make dist instead.
# One has to do this manually and cp it do the spider-source/doc dir
#man2html doc/spider.1.in > win_tmp/tmp/spider-reference.html
#man2html doc/spider-resolve.1 > win_tmp/tmp/spider-resolve.html

clean_newlines() {
    perl -pe 's/^\n$/\r\n/mg; s/([^\r])\n$/\1\r\n/mg;' $1 >$2
}

clean_localstatedir() {
    perl -pe 's/^\n$/\r\n/mg; s/([^\r])\n$/\1\r\n/mg; s{\@LOCALSTATEDIR\@/(lib|log)/spider/}{C:\\Documents and Settings\\Application Data\\Spider\\}' $1 >$2
}

for fn in address-spec.txt bridges-spec.txt control-spec.txt dir-spec.txt path-spec.txt rend-spec.txt socks-extensions.txt spider-spec.txt version-spec.txt; do
    clean_newlines doc/spec/$fn win_tmp/doc/spec/$fn
done

for fn in HACKING spider-gencert.html spider.html spiderify.html spider-resolve.html; do
    clean_newlines doc/$fn win_tmp/doc/$fn
done

for fn in README ChangeLog LICENSE; do
    clean_newlines $fn win_tmp/$fn
done

clean_localstatedir src/config/spiderrc.sample.in win_tmp/src/config/spiderrc.sample

cp contrib/win32build/spider-mingw.nsi.in win_tmp/contrib/

cd win_tmp
makensis.exe contrib/spider-mingw.nsi.in

