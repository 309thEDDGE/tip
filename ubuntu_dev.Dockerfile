FROM ubuntu:22.04
#forcing a 

# Prepare base requirements
RUN apt update -y && apt install -y build-essential ninja-build cmake git

# Build dependencies
RUN apt install -y googletest libgtest-dev libgmock-dev && \
    apt install -y libyaml-cpp-dev libspdlog-dev

RUN apt install -y libpcap-dev libssl-dev && \
    git clone https://github.com/mfontanini/libtins.git && \
    cd libtins && mkdir build && cd build && \
    cmake .. -DLIBTINS_ENABLE_WPA2=0 -DLIBTINS_ENABLE_CXX11=1 && \
    make install -j4

RUN apt install -y -V ca-certificates lsb-release wget && \
    wget https://apache.jfrog.io/artifactory/arrow/$(lsb_release --id --short | tr 'A-Z' 'a-z')/apache-arrow-apt-source-latest-$(lsb_release --codename --short).deb && \
    apt install -y -V ./apache-arrow-apt-source-latest-$(lsb_release --codename --short).deb && \
    apt update && \
    apt install -y -V libarrow-dev && \
    apt install -y -V libparquet-dev 

ENTRYPOINT ["/usr/bin/bash"]




 


