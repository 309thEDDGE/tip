FROM registry.il2.dso.mil/skicamp/project-opal/opal-operations:vendor-whl AS wheel

FROM registry.il2.dso.mil/platform-one/devops/pipeline-templates/centos8-gcc-bundle:1.0 AS builder

RUN mkdir /whl

COPY --from=wheel /whl /whl

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

# ARG GITLAB_TOKEN

# RUN git clone https://__token__@code.il2.dso.mil/skicamp/project-opal/opal-operations.git

ENV MINICONDA3_PATH="/home/user/miniconda3"
ENV CONDA_CHANNEL_DIR="/local-channels"
ENV ARTIFACT_CHANNEL_DIR=".ci_artifacts/build-metadata/build-artifacts"

# COPY $ARTIFACT_CHANNEL_DIR/test.txt $CONDA_CHANNEL_DIR/test.txt

COPY $ARTIFACT_CHANNEL_DIR/local_channel.tar $CONDA_CHANNEL_DIR/local_channel.tar
RUN tar -xvf $CONDA_CHANNEL_DIR/local_channel.tar -C $CONDA_CHANNEL_DIR && \
   mv $CONDA_CHANNEL_DIR/local-channel $CONDA_CHANNEL_DIR/tip-package-channel


RUN dnf install wget-1.19.5-10.el8 -y && \
    dnf clean all && \
    wget --progress=dot:giga https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh && \
    bash Miniconda3-latest-Linux-x86_64.sh -b -p ${MINICONDA3_PATH}


ENV PATH="${MINICONDA3_PATH}/bin:${PATH}"

COPY ${ARTIFACT_CHANNEL_DIR}/ ${CONDA_CHANNEL_DIR}/

ARG GITLAB_TOKEN

RUN echo "CONDA_CHANNEL_DIR = ${CONDA_CHANNEL_DIR}" && \
    ls ${CONDA_CHANNEL_DIR} && \
    pip install --no-cache-dir conda-mirror==0.8.2 && \
    pip install /whl/conda_vendor-0.1-py3-none-any.whl && \
    pip install --no-cache-dir conda-lock==0.10.0


RUN conda clean -afy && \
    mkdir "${CONDA_CHANNEL_DIR}/singleuser-channel" && \
    python -m conda_vendor local-channels -f /tip/tip_scripts/singleuser/singleuser.yml --channel-root "${CONDA_CHANNEL_DIR}/singleuser-channel" && \
    mkdir "${CONDA_CHANNEL_DIR}/tip-dependencies-channel" && \
    python -m conda_vendor local-channels -f /tip/tip_scripts/conda-mirror/tip_dependency_env.yml --channel-root "${CONDA_CHANNEL_DIR}/tip-dependencies-channel"
    

FROM registry.il2.dso.mil/platform-one/devops/pipeline-templates/centos8-gcc-bundle:1.0
#Twistlock: image should be created with non-root user

ENV CONDA_ADD_PIP_AS_PYTHON_DEPENDENCY=False

RUN groupadd -r user && useradd -r -g user user && mkdir /home/user && \
chown -R user:user /home/user
USER user
RUN mkdir /home/user/miniconda3 && \
    mkdir /home/user/miniconda3/notebooks && \
    mkdir /home/user/.jupyter/

COPY --from=builder --chown=user:user /home/user/miniconda3 /home/user/miniconda3
# Copies the local channels:
# singleuser-channel, tip-dependencies-channel, tip-package-channel
COPY --from=builder --chown=user:user /local-channels /home/user/local-channels
# Copy default conf directory for tip
COPY --chown=user:user conf /home/user/miniconda3/conf
# Nice user facing step so that users don't have to copy default conf from the
# conf directory in /root/miniconda3/
COPY --chown=user:user conf/default_conf/*.yaml /home/user/miniconda3/conf/
# Copy Jupyterlab config
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
# NOTE: Currently the mix of main and conda-forge isn't allowing an environment to solve
# RUN conda create -n tip tip jupyterlab pandas matplotlib pyarrow \
#     -c /home/user/local-channels/singleuser-channel/local_conda-forge \
#     -c /home/user/local-channels/tip-package-channel \
#     --offline --dry-run

# This is to validate the environment solves via local channels
# NOTE: Currently the mix of main and conda-forge isn't allowing an environment to solve
RUN conda create -n tip tip jupyterlab pandas matplotlib pyarrow \
     -c /home/user/local-channels/singleuser-channel/local_conda-forge \
     -c /home/user/local-channels/tip-package-channel \
     -c /home/user/local-channels/tip-dependencies-channel/local_conda-forge \
     --offline --dry-run

# installs a lightweight init system called tini
RUN conda install --quiet --yes tini

EXPOSE 8888

# Uses tini for init, runs the jupyterlab scripts for proper envvars
ENTRYPOINT ["tini","-g","--"]
CMD ["start-notebook.sh"]

COPY tip_scripts/start.sh tip_scripts/start-notebook.sh tip_scripts/start-singleuser.sh /usr/local/bin

#RUN conda env list && source ~/.bashrc && conda activate tip && conda env list
