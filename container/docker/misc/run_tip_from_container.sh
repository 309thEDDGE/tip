#!/bin/bash

docker run \
-it --rm --name tipcontainer \
-v /home/hdp/Documents/blah:/data \
tip \
/data/blah.ch10 /data/thesuite_ICD.txt
#/bin/bash
