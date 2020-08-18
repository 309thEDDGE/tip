
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
# Import tip with build system, source, scripts, etc.
#
ADD . /usr/local/tip

#
# Build tip and install
#
RUN cd /usr/local/tip/build && cmake .. -DCONTAINER=ON \
    && make -j8 && make install \
    && cd .. && rm -rf /usr/local/tip/build

#
# Setup default entrypoint
#
ENTRYPOINT ["python3.6", "/usr/local/tip/parse_and_translate.py"]

