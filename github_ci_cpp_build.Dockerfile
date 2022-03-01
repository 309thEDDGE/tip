FROM ubuntu:jammy-20220130

WORKDIR /home/tip

ARG DEBIAN_FRONTEND=noninteractive
ENV TZ=Etc/UTC
RUN apt update && apt install -y \
    make cmake gcc-9 g++-9 ninja-build python3 gcovr git

RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 90 --slave /usr/bin/g++ g++ /usr/bin/g++-9 --slave /usr/bin/gcov gcov /usr/bin/gcov-9 \
    && apt clean

COPY ./deps deps/

ENTRYPOINT ["/usr/bin/bash"]
