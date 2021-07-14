FROM registry.il2.dso.mil/platform-one/devops/pipeline-templates/centos8-gcc-bundle:1.0 AS builder

RUN mkdir /tip
# Tip source 
COPY cpp_pipeline_scripts /tip/cpp_pipeline_scripts
COPY tip_scripts /tip/tip_scripts/
#COPY cpp /tip/cpp
# CMake files explicity
#COPY CMake_conda.txt /tip/CMake_conda.txt
#COPY CMakeLists.txt  /tip/CMakeLists.txt
# Add README for tip Licence meta.yaml requirement in conda-recepies 
COPY README.md /tip/README.md

WORKDIR /tip
ENV MINICONDA3_PATH="/home/user/miniconda3"
ENV CONDA_CHANNEL_DIR="/local-channel"
RUN mkdir ${CONDA_CHANNEL_DIR} && \
dnf install wget-1.19.5-10.el8 -y && \
dnf clean all && \
wget --progress=dot:giga https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh && \
bash Miniconda3-latest-Linux-x86_64.sh -b -p ${MINICONDA3_PATH}
# RUN ./cpp_pipeline_scripts/build.sh
ENV PATH="${MINICONDA3_PATH}/bin:${PATH}"
COPY ${CMAKE_BUILD_DIR}/ ${CONDA_CHANNEL_DIR}/ 
RUN echo "${CMAKE_BUILD_DIR}" && ls ${CONDA_CHANNEL_DIR} && ls build

RUN pip install --no-cache-dir conda-mirror==0.8.2
ENV CONDA_MIRROR_DIR="/local-mirror"
ENV MIRROR_CONFIG="tip_scripts/conda-mirror/mirror_config.yaml"
RUN ./tip_scripts/conda-mirror/clone.sh

WORKDIR /tip/tip_scripts/singleuser
RUN pip install --no-cache-dir conda-lock==0.10.0
ENV SINGLEUSER_CHANNEL_DIR = "/tip/tip_scripts/singleuser/singleuser-channel"
RUN python -m conda_vendor local_channels -f singleuser.yml -c
WORKDIR /

RUN conda clean -afy

FROM registry.il2.dso.mil/platform-one/devops/pipeline-templates/centos8-gcc-bundle:1.0 
#Twistlock: image should be created with non-root user

RUN groupadd -r user && useradd -r -g user user && mkdir /home/user && \ 
chown -R user:user /home/user 
USER user
RUN mkdir /home/user/miniconda3 && mkdir /home/user/miniconda3/notebooks

COPY --from=builder --chown=user:user /home/user/miniconda3 /home/user/miniconda3
# tip built artifacts + conda packages that are not available in main channel
COPY --from=builder --chown=user:user /local-channel /home/user/local-channel
# tip dependencies e.g arrow-cpp (available in main channel repo.anaconda.com/main/)
COPY --from=builder --chown=user:user /local-mirror /home/user/local-mirror
# singleuser channel built with conda-vendor
COPY --from=builder --chown=user:user /tip/tip_scripts/singleuser/local_channel /home/user/singleuser-channel
# Copy default conf directory for tip
COPY --chown=user:user conf /home/user/miniconda3/conf
# Nice user facing step so that users don't have to copy default conf from the
# conf directory in /root/miniconda3/
COPY --chown=user:user conf/default_conf/*.yaml /home/user/miniconda3/conf/
# Copy Jupyterlab config
RUN mkdir /home/user/.jupyter/
COPY --chown=user:user tip_scripts/singleuser/jupyter_notebook_config.py /home/user/.jupyter/

COPY --chown=user:user tip_scripts/single_env/ /home/user/user_scripts
RUN chmod 700 /home/user/user_scripts/jupyter_conda.sh

# Twistlock: private key stored in image
USER root
RUN rm -rf /usr/share/doc/perl-IO-Socket-SSL/certs/*.enc && \
rm -rf /usr/share/doc/perl-IO-Socket-SSL/certs/*.pem && \
rm -r /usr/share/doc/perl-Net-SSLeay/examples/*.pem && \
rm  /usr/lib/python3.6/site-packages/pip/_vendor/requests/cacert.pem && \
rm  /usr/share/gnupg/sks-keyservers.netCA.pem && \
rm -rf /home/user/miniconda3/conda-meta && \
rm -rf /home/user/miniconda3/include

USER user
ENV PATH=/home/user/miniconda3/bin:$PATH
WORKDIR /home/user/

# This is to validate the environment solves via local channels
RUN conda create -n tip tip jupyterlab pandas matplotlib pyarrow -c file:///home/user/local-channel -c /home/user/local-mirror -c /home/user/singleuser-channel --offline --dry-run
EXPOSE 8888
