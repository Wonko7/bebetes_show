#!/bin/bash


mkdir /tmp/bbstmp
tar xjf "${1}" -C "/tmp/bbstmp"
cd /tmp/bbstmp
${4} `ls -1 /tmp/bbstmp | grep "\.c" | cut -f1 -d"."`.so `ls -1 /tmp/bbstmp | grep "\.c"`
mv /tmp/bbstmp/*.so ${2}
cp -r /tmp/bbstmp/lib_data/* ${3}
rm -r /tmp/bbstmp