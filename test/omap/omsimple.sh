#!/bin/bash

mkdir -p run
cd run

SSTOPTS=''
SDLOPTS=''

sst ../omsimple.py ${SSTOPTS} -- ${SDLOPTS};

wait

