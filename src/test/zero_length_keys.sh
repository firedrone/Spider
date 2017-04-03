#!/bin/sh
# Check that spider regenerates keys when key files are zero-length
# Test for bug #13111 - Spider fails to start if onion keys are zero length
#
# Usage:
#  ./zero_length_keys.sh PATH_TO_TOR
#    Run all the tests below
#  ./zero_length_keys.sh PATH_TO_TOR -z
#    Check spider will launch and regenerate zero-length keys
#  ./zero_length_keys.sh PATH_TO_TOR -d
#    Check spider regenerates deleted keys (existing behaviour)
#  ./zero_length_keys.sh PATH_TO_TOR -e
#    Check spider does not overwrite existing keys (existing behaviour)
#
# Exit Statuses:
#   0: test succeeded - spider regenerated/kept the files
#   1: test failed - spider did not regenerate/keep the files
#   2: test failed - spider did not generate the key files on first run
#   3: a command failed - the test could not be completed
#

if [ $# -eq 0 ] || [ ! -f ${1} ] || [ ! -x ${1} ]; then
  echo "Usage: ${0} PATH_TO_TOR [-z|-d|-e]"
  exit 1
elif [ $# -eq 1 ]; then
  echo "Testing that spider correctly handles zero-length keys"
  "$0" "${1}" -z && "$0" "${1}" -d && "$0" "${1}" -e
  exit $?
else #[$# -gt 1 ]; then
  TOR_BINARY="${1}"
  shift
fi

DATA_DIR=`mktemp -d -t spider_zero_length_keys.XXXXXX`
if [ -z "$DATA_DIR" ]; then
  echo "Failure: mktemp invocation returned empty string" >&2
  exit 3
fi
if [ ! -d "$DATA_DIR" ]; then
  echo "Failure: mktemp invocation result doesn't point to directory" >&2
  exit 3
fi
trap "rm -rf '$DATA_DIR'" 0

touch "$DATA_DIR"/empty_spiderrc

# DisableNetwork means that the ORPort won't actually be opened.
# 'ExitRelay 0' suppresses a warning.
TOR="${TOR_BINARY} --hush --DisableNetwork 1 --ShutdownWaitLength 0 --ORPort 12345 --ExitRelay 0 -f $DATA_DIR/empty_spiderrc"

if [ -s "$DATA_DIR"/keys/secret_id_key ] && [ -s "$DATA_DIR"/keys/secret_onion_key ] &&
   [ -s "$DATA_DIR"/keys/secret_onion_key_nspider ]; then
  echo "Failure: Previous spider keys present in spider data directory" >&2
  exit 3
else
  echo "Generating initial spider keys"
  $TOR --DataDirecspidery "$DATA_DIR"  --list-fingerprint

  # spider must successfully generate non-zero-length key files
  if [ -s "$DATA_DIR"/keys/secret_id_key ] && [ -s "$DATA_DIR"/keys/secret_onion_key ] &&
     [ -s "$DATA_DIR"/keys/secret_onion_key_nspider ]; then
    true #echo "spider generated the initial key files"
  else
    echo "Failure: spider failed to generate the initial key files"
    exit 2
  fi
fi

#ls -lh  "$DATA_DIR"/keys/ || exit 3

# backup and keep/delete/create zero-length files for the keys

FILE_DESC="keeps existing"
# make a backup
cp -r "$DATA_DIR"/keys "$DATA_DIR"/keys.old

# delete keys for -d or -z
if [ "$1" != "-e" ]; then
  FILE_DESC="regenerates deleted"
  rm "$DATA_DIR"/keys/secret_id_key || exit 3
  rm "$DATA_DIR"/keys/secret_onion_key || exit 3
  rm "$DATA_DIR"/keys/secret_onion_key_nspider || exit 3
fi

# create empty files for -z
if [ "$1" = "-z" ]; then
  FILE_DESC="regenerates zero-length"
  touch "$DATA_DIR"/keys/secret_id_key || exit 3
  touch "$DATA_DIR"/keys/secret_onion_key || exit 3
  touch "$DATA_DIR"/keys/secret_onion_key_nspider || exit 3
fi

echo "Running spider again to check if it $FILE_DESC keys"
$TOR --DataDirecspidery "$DATA_DIR" --list-fingerprint

#ls -lh "$DATA_DIR"/keys/ || exit 3

# spider must always have non-zero-length key files
if [ -s "$DATA_DIR"/keys/secret_id_key ] && [ -s "$DATA_DIR"/keys/secret_onion_key ] &&
   [ -s "$DATA_DIR"/keys/secret_onion_key_nspider ]; then
  # check if the keys are different to the old ones
  diff -q -r "$DATA_DIR"/keys "$DATA_DIR"/keys.old > /dev/null
  SAME_KEYS=$?
  # if we're not testing existing keys,
  # the current keys should be different to the old ones
  if [ "$1" != "-e" ]; then
    if [ $SAME_KEYS -ne 0 ]; then
      echo "Success: test that spider $FILE_DESC key files: different keys"
      exit 0
    else
      echo "Failure: test that spider $FILE_DESC key files: same keys"
      exit 1
    fi
  else #[ "$1" == "-e" ]; then
    if [ $SAME_KEYS -eq 0 ]; then
      echo "Success: test that spider $FILE_DESC key files: same keys"
      exit 0
    else
      echo "Failure: test that spider $FILE_DESC key files: different keys"
      exit 1
    fi
  fi
else
  echo "Failure: test that spider $FILE_DESC key files: no key files"
  exit 1
fi
