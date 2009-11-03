#! /bin/bash
./build.sh && svn ci "$1" -m "$2"
