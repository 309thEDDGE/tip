FROM registry.il2.dso.mil/platform-one/devops/pipeline-templates/centos8-gcc-bundle:1.0 AS builder

RUN mkdir /tip
# Tip source 
ADD cpp_pipeline_scripts /tip/cpp_pipeline_scripts
ADD tip_scripts /tip/tip_scripts/
ADD cpp /tip/cpp
# CMake files explicity
ADD CMake_conda.txt /tip/CMake_conda.txt
ADD CMakeLists.txt  /tip/CMakeLists.txt
# Add README for tip Licence meta.yaml requirement in conda-recepies 
ADD README.md /tip/README.md

WORKDIR /tip
RUN mkdir /home/user
RUN ./cpp_pipeline_scripts/build.sh
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
RUN find /usr/ -type f -name "*.pem" | xargs rm

USER user
ENV PATH=/home/user/miniconda3/bin:$PATH
RUN conda create -n tip_dev tip -c file:///home/user/local-channel -c /home/user/local-mirror --dry-run --offline
