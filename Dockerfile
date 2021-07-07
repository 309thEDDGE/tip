FROM registry.il2.dso.mil/platform-one/devops/pipeline-templates/centos8-gcc-bundle:1.0 AS builder

RUN mkdir /tip
# Tip source 
COPY cpp_pipeline_scripts /tip/cpp_pipeline_scripts
COPY tip_scripts /tip/tip_scripts/
COPY cpp /tip/cpp
# CMake files explicity
COPY CMake_conda.txt /tip/CMake_conda.txt
COPY CMakeLists.txt  /tip/CMakeLists.txt
# Add README for tip Licence meta.yaml requirement in conda-recepies 
COPY README.md /tip/README.md

WORKDIR /tip
RUN ./cpp_pipeline_scripts/build.sh
ENV PATH="/home/user/miniconda3/bin:${PATH}"

RUN pip install --no-cache-dir conda-mirror==0.8.2
ENV CONDA_MIRROR_DIR="/local-mirror"
ENV MIRROR_CONFIG="tip_scripts/conda-mirror/mirror_config.yaml"
RUN ./tip_scripts/conda-mirror/clone.sh


WORKDIR /tip/tip_scripts/singleuser
RUN pip install --no-cache-dir conda-lock==0.10.0
ENV SINGLEUSER_CHANNEL_DIR = "singleuser-channel"
RUN python -m conda_vendor local_channels -f /tip/tip_scripts/singleuser/singleuser.yml -l $SINGLEUSER_CHANNEL_DIR
WORKDIR /

RUN conda clean -afy

FROM registry.il2.dso.mil/platform-one/devops/pipeline-templates/centos8-gcc-bundle:1.0 
#Twistlock: image should be created with non-root user

RUN groupadd -r user && useradd -r -g user user && mkdir /home/user && \ 
chown -R user:user /home/user 
USER user
RUN mkdir /home/user/miniconda3

COPY --from=builder --chown=user:user /home/user/miniconda3 /home/user/miniconda3
COPY --from=builder --chown=user:user /local-channel /home/user/local-channel
COPY --from=builder --chown=user:user /local-mirror /home/user/local-mirror
COPY --from=builder --chown=user:user /tip/tip_scripts/singleuser/singleuser-channel /home/user/singleuser-channel
# Copy default conf directory for tip
COPY --chown=user:user conf /home/user/miniconda3/conf
# Nice user facing step so that users don't have to copy default conf from the
# conf directory in /root/miniconda3/
COPY --chown=user:user conf/default_conf/*.yaml /home/user/miniconda3/conf/

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

# This is to validate the environment solves via local channels
RUN conda create -n tip tip jupyterlab pandas matplotlib pyarrow -c file:///home/user/local-channel -c /home/user/local-mirror -c /home/user/singleuser-channel --offline

RUN source activate tip
