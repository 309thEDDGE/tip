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

FROM frolvlad/alpine-glibc

COPY --from=build /usr/local/bin/tip* /bin/
COPY --from=build /usr/local/bin/pqcompare /bin/
COPY --from=build /usr/local/bin/bincompare /bin/
COPY --from=build /usr/local/bin/tests /bin/
COPY --from=build /usr/local/bin/parquet_video_extractor /bin/
COPY --from=build /usr/local/bin/validate_yaml /bin/

RUN ls -l /bin && /bin/tests

ENTRYPOINT ["/bin/sh"]

