#!/bin/sh
# Validate Spider's nspider implementation.

exitcode=0

"${PYTHON:-python}" "${abs_top_srcdir:-.}/src/test/nspider_ref.py" test-spider || exitcode=1
"${PYTHON:-python}" "${abs_top_srcdir:-.}/src/test/nspider_ref.py" self-test || exitcode=1

exit ${exitcode}
