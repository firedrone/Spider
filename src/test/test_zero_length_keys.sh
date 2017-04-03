#!/bin/sh
# Check that spider regenerates keys when key files are zero-length

exitcode=0

"${SHELL:-sh}" "${abs_top_srcdir:-.}/src/test/zero_length_keys.sh" "${builddir:-.}/src/or/spider" -z || exitcode=1
"${SHELL:-sh}" "${abs_top_srcdir:-.}/src/test/zero_length_keys.sh" "${builddir:-.}/src/or/spider" -d || exitcode=1
"${SHELL:-sh}" "${abs_top_srcdir:-.}/src/test/zero_length_keys.sh" "${builddir:-.}/src/or/spider" -e || exitcode=1

exit ${exitcode}
