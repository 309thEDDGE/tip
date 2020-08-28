# In the pipeline the working directory is the root of the project repository.
# When running from docker, the tip folder is mounted as /app
[ -d /app/cpp ] && cd /app # if /app/cpp exists cd to /app

BUILD_DIR=build

mkdir -p $BUILD_DIR \
    && cd $BUILD_DIR \
    && cmake .. -DCONTAINER=ON \
    && make -j2 VERBOSE=1
