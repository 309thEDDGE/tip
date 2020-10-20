
FROM centos

ENV YUMOPT="dnf install -y"

# 
# Setup repos and update package manager.
#
RUN dnf update -y \
	&& rpm --import http://download.fedoraproject.org/pub/epel/RPM-GPG-KEY-EPEL-8 \
	&& $YUMOPT dnf-plugins-core \
	&& $YUMOPT https://dl.fedoraproject.org/pub/epel/epel-release-latest-8.noarch.rpm \
	&& dnf update -y \
	&& $YUMOPT epel-release \
	&& dnf config-manager --set-enabled PowerTools \
	&& dnf clean all && rm -r /var/cache/dnf && dnf upgrade -y && dnf update -y

#
# Configure and install Apache Arrow libs. Currently, only the latest release rpm
# completes the dependency installation process.
#
RUN rpm --import https://apache.bintray.com/arrow/centos/RPM-GPG-KEY-apache-arrow \
#	&& $YUMOPT https://apache.bintray.com/arrow/centos/8/x86_64/Packages/apache-arrow-release-0.16.0-1.el8.noarch.rpm \
        && $YUMOPT https://apache.bintray.com/arrow/centos/8/apache-arrow-release-latest.rpm \
	&& dnf update -y \
#	&& $YUMOPT arrow-devel-0.16.0-1.el8 \
#	&& $YUMOPT parquet-devel-0.16.0-1.el8 \
	&& $YUMOPT arrow-devel \
	&& $YUMOPT parquet-devel

#
# Build tools, misc deps
#
RUN $YUMOPT python36 \
	&& $YUMOPT python3-numpy \
	&& $YUMOPT yaml-cpp-devel \
	&& $YUMOPT cmake \
	# includes gcc, make, gdb
	&& $YUMOPT gcc-toolset-9 \
	&& $YUMOPT gcc-c++

#
# Build libtins
#

# Install pcap libs - already installed in the centos image
RUN $YUMOPT git \
    && $YUMOPT libpcap-devel

# Clone libtins and build it
WORKDIR /build_libtins
RUN git clone https://github.com/mfontanini/libtins.git
RUN cd libtins \
    && mkdir build \
    && cd build \
    && cmake .. -DLIBTINS_BUILD_SHARED=1 \
    && make -j4 \
    && make install
RUN rm -rf /build_libtins

#
# Clone libirig106 (Isaac's fork) and build/install it
#
WORKDIR /build_libirig106
RUN git clone https://github.com/ijmyers/libirig106.git
RUN cd libirig106 \
    && make \
    && cp -f libirig106.a libirig106.so /usr/lib64 \
    && mkdir -p /usr/include/libirig106 \
    && cp -f  src/*.h /usr/include/libirig106
RUN rm -rf /build_libirig106

#
# Install Google Mock and Google Test
#
RUN $YUMOPT gtest-devel \
    && $YUMOPT gmock-devel

#
# Import tip with build system, source, scripts, etc.
#
WORKDIR /usr/local/tip
COPY cpp ./cpp
COPY tip_scripts ./tip_scripts
COPY *.py .
COPY CMake*.txt .
COPY conf ./conf

#
# Build tip and install
#
# Two RUN commands: get an easy layer
# pass with the first RUN so ADD command
# doesn't have be executed repeatedly.
#
RUN mkdir -p /usr/local/tip/build 
RUN cd /usr/local/tip/build \
    && cmake .. -DCONTAINER=ON \
    && make -j8 && make install \
    && cd .. && rm -rf /usr/local/tip/build

#
# Setup default entrypoint
#
#ENTRYPOINT ["python3.6", "/usr/local/tip/parse_and_translate.py"]

