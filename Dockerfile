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
WORKDIR /

FROM registry.il2.dso.mil/platform-one/devops/pipeline-templates/centos8-gcc-bundle:1.0 
#Twistlock: image should be created with non-root user

RUN groupadd -r user && useradd -r -g user user && mkdir /home/user
RUN chown -R user:user /home/user 
USER user
RUN mkdir /home/user/miniconda3

COPY --from=builder --chown=user:user /home/user/miniconda3 /home/user/miniconda3
COPY --from=builder --chown=user:user /local-channel /home/user/local-channel
COPY --from=builder --chown=user:user /local-mirror /home/user/local-mirror
# Copy default conf directory for tip
COPY --chown=user:user conf /home/user/miniconda3/conf
# Nice user facing step so that users don't have to copy default conf from the
# conf directory in /root/miniconda3/
COPY --chown=user:user conf/default_conf/*.yaml /home/user/miniconda3/conf/

# Twistlock: private key stored in image
USER root
RUN rm -rf /usr/share/doc/perl-IO-Socket-SSL/certs/*.enc
RUN rm -rf /usr/share/doc/perl-IO-Socket-SSL/certs/*.pem
RUN rm -r /usr/share/doc/perl-Net-SSLeay/examples/*.pem
RUN rm  /usr/lib/python3.6/site-packages/pip/_vendor/requests/cacert.pem
RUN rm  /usr/share/gnupg/sks-keyservers.netCA.pem

USER user
ENV PATH=/home/user/miniconda3/bin:$PATH

# This is to validate the environment solves via local channels
RUN conda create -n tip_dev tip -c file:///home/user/local-channel -c /home/user/local-mirror --dry-run --offline
