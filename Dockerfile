FROM registry.il2.dso.mil/skicamp/project-opal/opal-operations:vendor-whl AS wheel
FROM registry.il2.dso.mil/platform-one/devops/pipeline-templates/ironbank/miniconda:4.9.2 AS builder

USER root

COPY --from=wheel /whl /whl

RUN mkdir /tip
COPY tip_scripts /tip/tip_scripts/
COPY README.md /tip/README.md

WORKDIR /tip

# ARG GITLAB_TOKEN
# RUN git clone https://__token__@code.il2.dso.mil/skicamp/project-opal/opal-operations.git

ENV CONDA_PATH="/opt/conda"
ENV CONDA_CHANNEL_DIR="/local-channels"
ENV ARTIFACT_CHANNEL_DIR=".ci_artifacts/build-metadata/build-artifacts"

COPY $ARTIFACT_CHANNEL_DIR/local_channel.tar $CONDA_CHANNEL_DIR/local_channel.tar
RUN tar -xvf $CONDA_CHANNEL_DIR/local_channel.tar -C $CONDA_CHANNEL_DIR && \
    mv $CONDA_CHANNEL_DIR/local-channel $CONDA_CHANNEL_DIR/tip-package-channel

ENV PATH="${CONDA_PATH}/bin:${PATH}"
RUN conda init && source /root/.bashrc && conda activate base && \
    pip3 install --no-cache-dir /whl/conda_vendor-0.1-py3-none-any.whl && \
    echo "CONDA_CHANNEL_DIR = ${CONDA_CHANNEL_DIR}" && \
    ls ${CONDA_CHANNEL_DIR} && \
    mkdir "${CONDA_CHANNEL_DIR}/singleuser-channel" && \
    python -m conda_vendor local-channels -f /tip/tip_scripts/singleuser/singleuser.yml --channel-root "${CONDA_CHANNEL_DIR}/singleuser-channel" && \
    mkdir "${CONDA_CHANNEL_DIR}/tip-dependencies-channel" && \
    python -m conda_vendor local-channels -f /tip/tip_scripts/conda-mirror/tip_dependency_env.yml --channel-root "${CONDA_CHANNEL_DIR}/tip-dependencies-channel" && \
    ls ${CONDA_CHANNEL_DIR} 

FROM registry.il2.dso.mil/platform-one/devops/pipeline-templates/centos8-gcc-bundle:1.0

SHELL ["/usr/bin/bash", "-c"] 

ENV CONDA_ADD_PIP_AS_PYTHON_DEPENDENCY=False
ENV CONDA_PATH="/opt/conda"

RUN groupadd -r user && useradd -r -g user user && mkdir /home/user && \
    chown -R user:user /home/user

COPY --from=builder --chown=user:user $CONDA_PATH $CONDA_PATH
COPY --from=builder --chown=user:user /local-channels /home/user/local-channels
COPY --chown=user:user conf /home/user/conf
COPY --chown=user:user conf/default_conf/*.yaml /home/user/conf/
COPY --chown=user:user tip_scripts/singleuser/jupyter_notebook_config.py /home/user/.jupyter/
COPY --chown=user:user tip_scripts/single_env/start_jupyter_nb.sh /home/user/user_scripts/

RUN chmod 700 /home/user/user_scripts/start_jupyter_nb.sh && \
    rm -rf /usr/share/doc/perl-IO-Socket-SSL/certs/*.enc && \
    rm -rf /usr/share/doc/perl-IO-Socket-SSL/certs/*.pem && \
    rm -r /usr/share/doc/perl-Net-SSLeay/examples/*.pem && \
    rm  /usr/lib/python3.6/site-packages/pip/_vendor/requests/cacert.pem && \
    rm  /usr/share/gnupg/sks-keyservers.netCA.pem && \
    rm -rf ${CONDA_PATH}/conda-meta && \
    rm -rf ${CONDA_PATH}/include

USER user
ENV PATH="${CONDA_PATH}/bin:$PATH"
RUN conda init && source /home/user/.bashrc
WORKDIR /home/user

RUN conda create -n tip tip jupyterlab pandas matplotlib pyarrow \
     -c /home/user/local-channels/singleuser-channel/local_conda-forge \
     -c /home/user/local-channels/tip-package-channel \
     -c /home/user/local-channels/tip-dependencies-channel/local_conda-forge \
     --offline --dry-run

EXPOSE 8888

ENTRYPOINT ["/usr/bin/bash","/home/user/user_scripts/start_jupyter_nb.sh"]

