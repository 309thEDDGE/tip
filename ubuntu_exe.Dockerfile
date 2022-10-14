FROM ubuntu:22.04 AS build

# Prepare base requirements
RUN apt update -y && apt install -y build-essential ninja-build cmake git

# Build dependencies
RUN apt install -y googletest libgtest-dev libgmock-dev && \
    apt install -y libyaml-cpp-dev libspdlog-dev

WORKDIR /libpcap
RUN apt install -y libpcap-dev libssl-dev && \
    git clone https://github.com/mfontanini/libtins.git && \
    cd libtins && mkdir build && cd build && \
    cmake .. -DLIBTINS_ENABLE_WPA2=0 -DLIBTINS_ENABLE_CXX11=1 && \
    make install -j4

WORKDIR /arrow-cpp
RUN rm -rf /libpcap && \
    apt install -y -V ca-certificates lsb-release wget && \
    wget https://apache.jfrog.io/artifactory/arrow/$(lsb_release --id --short | tr 'A-Z' 'a-z')/apache-arrow-apt-source-latest-$(lsb_release --codename --short).deb && \
    apt install -y -V ./apache-arrow-apt-source-latest-$(lsb_release --codename --short).deb && \
    apt update && \
    apt install -y -V libarrow-dev && \
    apt install -y -V libparquet-dev 

WORKDIR /tip
COPY CMakeLists.txt .
COPY cmake cmake/ 
COPY cpp cpp/
COPY .git .git/

RUN rm -rf /arrow-cpp && \
    mkdir -p build && ls && cd build && pwd && \
    cmake .. -GNinja && \
    ninja install -j 8

WORKDIR /
RUN rm -rf /tip && \
    /usr/local/bin/tests

# We could reduce size by using a stripped down base image.
# I had trouble with alpine-glibc due to ABI incompatiblities.
# Not sure if it's worth the time--I estimate maybe 30 MB
# reduction. 
#FROM frolvlad/alpine-glibc
FROM ubuntu:22.04

# RUN apt update -y && apt install -y build-essential
COPY --from=build ["/usr/local/bin/tip*", "/usr/local/bin/pqcompare", \
    "/usr/local/bin/bincompare", "/usr/local/bin/tests", \
    "/usr/local/bin/parquet_video_extractor", "/usr/local/bin/validate_yaml", \
    "/usr/local/bin/"]

# COPY follows symlinks and replaces links at the destination
# with the name of the original content and data represented
# the target. This duplicates data in the destination (inefficient)
# and causes ldconfig to throw warnings.
# 
# Also, make this a single COPY layer, as above.
COPY --from=build /usr/lib/x86_64-linux-gnu/libarrow.so.* /usr/lib/x86_64-linux-gnu/ 
COPY --from=build /usr/lib/x86_64-linux-gnu/libbrotli*.so* /usr/lib/x86_64-linux-gnu/ 
COPY --from=build /usr/lib/x86_64-linux-gnu/libutf8proc*.so* /usr/lib/x86_64-linux-gnu/ 
COPY --from=build /usr/lib/x86_64-linux-gnu/libre2*.so* /usr/lib/x86_64-linux-gnu/ 
COPY --from=build /usr/lib/x86_64-linux-gnu/libcurl*.so* /usr/lib/x86_64-linux-gnu/ 
COPY --from=build /usr/lib/x86_64-linux-gnu/libnghttp2*.so* /usr/lib/x86_64-linux-gnu/ 
COPY --from=build /usr/lib/x86_64-linux-gnu/librtmp*.so* /usr/lib/x86_64-linux-gnu/ 
COPY --from=build /usr/lib/x86_64-linux-gnu/libssh*.so* /usr/lib/x86_64-linux-gnu/ 
COPY --from=build /usr/lib/x86_64-linux-gnu/libpsl*.so* /usr/lib/x86_64-linux-gnu/ 
COPY --from=build /usr/lib/x86_64-linux-gnu/libdbus*.so* /usr/lib/x86_64-linux-gnu/ 
COPY --from=build /usr/lib/x86_64-linux-gnu/liblz4*.so* /usr/lib/x86_64-linux-gnu/ 
COPY --from=build /usr/lib/x86_64-linux-gnu/libbz2*.so* /usr/lib/x86_64-linux-gnu/ 
COPY --from=build /usr/lib/x86_64-linux-gnu/libparquet.so.* /usr/lib/x86_64-linux-gnu/ 
COPY --from=build /usr/lib/x86_64-linux-gnu/libpcap*.so* /usr/lib/x86_64-linux-gnu/ 
COPY --from=build /usr/lib/x86_64-linux-gnu/libprotobuf*.so* /usr/lib/x86_64-linux-gnu/ 
COPY --from=build /usr/lib/x86_64-linux-gnu/libsnappy*.so* /usr/lib/x86_64-linux-gnu/ 
COPY --from=build /usr/lib/x86_64-linux-gnu/libspdlog*.so* /usr/lib/x86_64-linux-gnu/ 
COPY --from=build /usr/lib/x86_64-linux-gnu/libthrift*.so* /usr/lib/x86_64-linux-gnu/ 
COPY --from=build /usr/lib/x86_64-linux-gnu/libyaml-cpp.so.* /usr/lib/x86_64-linux-gnu/ 
COPY --from=build /usr/lib/x86_64-linux-gnu/libfmt*.so* /usr/lib/x86_64-linux-gnu/ 
COPY --from=build /usr/local/lib/libtins*.so* /usr/lib/x86_64-linux-gnu/ 
COPY --from=build /usr/lib/x86_64-linux-gnu/libzstd*.so* /usr/lib/x86_64-linux-gnu/ 
COPY --from=build /usr/lib/x86_64-linux-gnu/libldap*.so* /usr/lib/x86_64-linux-gnu/ 
COPY --from=build /usr/lib/x86_64-linux-gnu/liblber*.so* /usr/lib/x86_64-linux-gnu/ 
COPY --from=build /usr/lib/x86_64-linux-gnu/libsasl2*.so* /usr/lib/x86_64-linux-gnu/ 

RUN /usr/local/bin/tests

ENTRYPOINT ["/bin/bash"]

