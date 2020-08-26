FROM registry.access.redhat.com/ubi8/ubi

ENV YUMOPT="dnf install -y"
ARG RHEL_SUB_USERNAME
ARG RHEL_SUB_PASSWORD
# 
# Setup repos and update package manager.
#
RUN dnf update -y \
    && $YUMOPT dnf-plugins-core \
    && subscription-manager register --username isaac.myers --password A10ContainerTest --auto-attach \
    && subscription-manager repos --enable "codeready-builder-for-rhel-8-x86_64-rpms" \
    && $YUMOPT https://dl.fedoraproject.org/pub/epel/epel-release-latest-8.noarch.rpm \
    && dnf update -y && dnf clean all && rm -r /var/cache/dnf && dnf upgrade -y

#
# Configure and install Apache Arrow libs. Currently, only the latest release rpm
# completes the dependency installation process.
#
RUN rpm --import https://apache.bintray.com/arrow/centos/RPM-GPG-KEY-apache-arrow \
        && $YUMOPT https://apache.bintray.com/arrow/centos/8/apache-arrow-release-latest.rpm \
	&& dnf update -y \
	&& $YUMOPT arrow-devel \
	&& $YUMOPT parquet-devel

#
# Build tools, misc deps
#
RUN $YUMOPT python36 \
	&& $YUMOPT python3-numpy \
	&& $YUMOPT yaml-cpp-devel \
	&& $YUMOPT cmake \
	&& $YUMOPT gcc-toolset-9 \
	&& $YUMOPT gcc-c++

RUN $YUMOPT gtest-devel \
    && $YUMOPT gmock-devel 

RUN yum clean all

#
# Import tip with build system, source, scripts, etc.
#
# ADD . /usr/local/tip

#
# Build tip and install
#
# RUN cd /usr/local/tip/build && cmake .. -DCONTAINER=ON \
#     && make -j8 && make install \
#     && cd .. && rm -rf /usr/local/tip/build

#
# Setup default entrypoint
#
#ENTRYPOINT ["python3.6", "/usr/local/tip/parse_and_translate.py"]
