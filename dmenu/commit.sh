#! /bin/bash
echo "svn ci $@"
./build.sh && svn ci $@
