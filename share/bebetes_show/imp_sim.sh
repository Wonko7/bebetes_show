#!/bin/bash

#mkdir "~/.bebetes_show/save/${1}"
mkdir /tmp/tmpbbs
tar xjf "${1}" -C /tmp/tmpbbs
cp ${2}`ls /tmp/tmpbbs/*/lib` /tmp/tmpbbs/*/lib/
cp -r /tmp/tmpbbs/* ~/.bebetes_show/save/
rm -r /tmp/tmpbbs