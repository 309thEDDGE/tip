#!/usr/bin/env bash

ARTIFACTORY=artifactory.spacecamp.ninja
IMAGE=$ARTIFACTORY/docker/platform/ubi8-gcc-gcov-cmake:1.0

# Skip login if running under WSL
# This means that docker run will fail if there is no local image
# To get the local image, first run local_pipeline.bat in DOS or Powershell
if ! grep --silent -i microsoft /proc/version ; then
	docker login $ARTIFACTORY
fi

# Run build and test scripts in a container based on the docker image used for the pipeline
docker run -it --rm -v $PWD:/app $IMAGE bash /app/ci_p1/build.sh \
	&& docker run -it --rm -v $PWD:/app $IMAGE bash /app/ci_p1/unit-test.sh
