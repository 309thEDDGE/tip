FROM registry.il2.dso.mil/platform-one/devops/pipeline-templates/centos8-gcc-bundle:1.0
ENV PATH="/root/miniconda3/bin:${PATH}"
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

RUN dnf install wget bzip2 -y
RUN wget -qO- https://conda.anaconda.org/conda-forge/linux-64/micromamba-0.13.1-0.tar.bz2  | tar -xvj bin/micromamba --strip-components=1
#RUN ./micromamba shell init -s bash -p ~/micromamba
#
# Import tip dependencies
#
# Install Conda from the official rpm
# RUN rpm --import https://repo.anaconda.com/pkgs/misc/gpgkeys/anaconda.asc
# # 
# COPY conda.repo /etc/yum/repos.d/conda.repo
#RUN dnf install conda -y
#ENV PATH=$PATH:/opt/conda/bin
#RUN dnf install python-pip -y
#RUN pip install conda 
#RUN conda info
RUN mkdir /tip
ADD . /tip

#RUN	mkdir -p /deps/alkemist-lfr/lib \
#	&& mv ${LFR_ROOT_PATH}/lib/run/liblfr.a /deps/alkemist-lfr/lib

# set-up a local channel
RUN ./micromamba create -p /opt/conda/ conda conda-mirror -c main -c conda-forge
ENV PATH=/opt/conda/bin:$PATH
WORKDIR /tip
RUN ./cpp_pipeline_scripts/build.sh
#RUN /root/micromamba --version
RUN conda create -n tip_dev tip -c /local-channel -c /local-mirror
CMD ["/bin/bash"]
