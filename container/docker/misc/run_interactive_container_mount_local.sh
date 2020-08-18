#!/bin/bash

docker run \
-it --rm --name tipcontainer \
-v /home/hdp/Documents/repos/tip:/app \
tip \
#/bin/bash
