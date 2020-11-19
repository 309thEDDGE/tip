FROM registry.il2.dsop.io/platform-one/devops/pipeline-templates/ubi8-gcc-bundle:1.0

ENV LFR_ROOT_PATH=/opt/alkemist/lfr
# Install alkemist-lfr
RUN echo $'[AppStream]\n\
name=AppStream\n\
baseurl=http://mirror.centos.org/centos/8/AppStream/x86_64/os\n\
enabled=1\n\
gpgcheck=0\n\
repo_gpgcheck=0\n' >> /etc/yum.repos.d/appstream.repo \
	# Add RunSafe repo to list of those yum will check for packages \
	&& echo $'[RunSafeSecurity]\n\
name=RunSafeSecurity\n\
baseurl=https://runsafesecurity.jfrog.io/artifactory/rpm-alkemist-lfr\n\
enabled=1\n\
gpgcheck=0\n\
gpgkey=https://runsafesecurity.jfrog.io/artifactory/rpm-alkemist-lfr/repodata/repomd.xml.key\n\
repo_gpgcheck=1\n' >> /etc/yum.repos.d/runsafesecurity.repo \
	&& yum -y install alkemist-lfr
#
# Import tip dependencies
#
RUN mkdir /deps
ADD . /deps

RUN	mkdir -p /deps/alkemist-lfr/lib \
	&& mv ${LFR_ROOT_PATH}/lib/run/liblfr.a /deps/alkemist-lfr/lib
