#!/usr/bin/env bash

REGISTRY=registry.il2.dsop.io
BUILD_IMAGE=$REGISTRY/skicamp/project-opal/tip:tip-deps
TEST_IMAGE=$REGISTRY/skicamp/project-opal/tip/test:1.0

docker login $REGISTRY

# Run build and test scripts in a container based on the docker image used for the pipeline
docker run -it --rm --env ALKEMIST_LICENSE_KEY -v $PWD:/app $BUILD_IMAGE bash /app/cpp_pipeline_scripts/build.sh \
	&& docker run -it --rm -v $PWD:/app $TEST_IMAGE bash /app/cpp_pipeline_scripts/unit-test.sh
