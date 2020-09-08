ARTIFACTORY=artifactory.spacecamp.ninja
# IMAGE=$ARTIFACTORY/docker/platform/tip-rhel-cpp-build:0.1
IMAGE=$ARTIFACTORY/docker/platform/ubi8-gcc-gcov-cmake

docker login $ARTIFACTORY

# Start bash in a container based on the docker image used for the pipeline
docker run -it --rm -v $PWD:/app $IMAGE bash /app/pipeline/build.sh \
	&& docker run -it --rm -v $PWD:/app $IMAGE /app/build/cpp/tests
