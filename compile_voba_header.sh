#!/bin/bash -x
h_file=$1
so_file=${h_file%.h}.i.so
echo $h_file $so_file
VOBA_BUILD=../build

cat <<EOF | cpp -E -I ../exec_once -I ../voba_str -I ../../vhash -I ~/d/other-working/GC/bdwgc/include -I$VOBA_BUILD -x c - | sed 's/^#.*//g'| indent > /tmp/a.c
#include <stdio.h>
#include <voba/include/value.h>
#include "$h_file"
EOF


$CC -std=c99 -c /tmp/a.c
