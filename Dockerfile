FROM registry.il2.dso.mil/platform-one/devops/pipeline-templates/centos8-gcc-bundle:1.0 AS builder

ENV PATH=/opt/conda/bin:$PATH

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
RUN ./cpp_pipeline_scripts/build.sh
WORKDIR /

FROM registry.il2.dso.mil/platform-one/devops/pipeline-templates/centos8-gcc-bundle:1.0 

COPY --from=builder /root/miniconda3 /root/miniconda3
COPY --from=builder /local-channel /local-channel
COPY --from=builder /local-mirror /local-mirror
# Copy default conf directory for tip
COPY conf /root/miniconda/conf
# Nice user facing step so that users don't have to copy default conf from the
# conf directory in /root/miniconda3/
COPY conf/default_conf/*.yaml /root/miniconda3/conf/
ENV PATH=/root/miniconda3/bin:$PATH
RUN conda create -n tip_dev tip -c /local-channel -c /local-mirror --dry-run --offline
# Twistlock: private key stored in image
RUN rm -rf /usr/share/doc/perl-IO-Socket-SSL/certs/*.enc
RUN rm -rf /usr/share/doc/perl-IO-Socket-SSL/certs/*.pem
RUN rm -r /usr/share/doc/perl-Net-SSLeay/examples/*.pem
RUN find /usr/ -type f -name "*.pem" | xargs rm

# Twistlock: image should be created with non-root user
RUN groupadd -r user && useradd -r -g user user 

USER user
