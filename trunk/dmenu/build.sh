#!/bin/bash
make -f Makefile.host rebuild 0> /dev/null 1> /dev/null 2> host_err
make -f Makefile.dingoo rebuild 0> /dev/null 1> /dev/null 2> dingoo_err
HOST_ERRS=`cat host_err | grep "\(warning\|error\)" | wc -l`
DINGOO_ERRS=`cat dingoo_err | grep "\(warning\|error\)" | wc -l`

if [ $HOST_ERRS -gt 0 ]; then
  echo 
  echo "Host Errors"
  echo "-----------------------------------------"
  cat host_err | grep -v make
  echo 
fi

if [ $DINGOO_ERRS -gt 0 ]; then
  echo 
  echo "Dingoo Errors"
  echo "-----------------------------------------"
  cat dingoo_err | grep -v make
  echo 
fi

rm -f host_err dingoo_err

if [ $((HOST_ERRS + DINGOO_ERRS)) -gt 0 ]; then
   exit 1 
fi
