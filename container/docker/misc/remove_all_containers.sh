#!/bin/bash

docker container rm $(docker container ls -aq)
