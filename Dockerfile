FROM registry1.dso.mil/ironbank/opensource/metrostar/tip-dependencies:0.0.5 AS tipdependencies
FROM registry1.dso.mil/ironbank/opensource/metrostar/singleuser:torch_1.10.0_v4 AS pytorch
FROM registry1.dso.mil/ironbank/opensource/metrostar/singleuser:singleuser_v4 AS singleuser

COPY --chown=jovyan:jovyan --from=tipdependencies /local_channel /home/jovyan/tip_deps_channel
COPY --chown=jovyan:jovyan --from=pytorch /home/jovyan/local-channel /home/jovyan/pytorch_channel

ENV ARTIFACT_DIR=".ci_artifacts/build-metadata/build-artifacts"

WORKDIR /home/jovyan
CMD []

COPY --chown=jovyan:jovyan $ARTIFACT_DIR/local_channel.tar .
COPY --chown=jovyan:jovyan ./conf /home/jovyan/

RUN sed '/local-channel/s/.*/  - .\/pytorch_channel\n/' /home/jovyan/pytorch_channel/local_channel_env.yaml > /home/jovyan/pytorch_env.yaml \
    && conda env create -f /home/jovyan/pytorch_env.yaml \
    && rm -rf /home/jovyan/pytorch_channel

# Delete this block and uncomment the following block upon next update to singleuser
RUN mkdir /home/jovyan/tip_channel \
    && tar xvf local_channel.tar --strip-components=3 --directory=/home/jovyan/tip_channel \
    && sed '/conda-forge/s/.*/  - .\/tip_channel\n  - .\/tip_deps_channel\n  - .\/local-channel/' /home/jovyan/local-channel/local_channel_env.yaml > /home/jovyan/singleuser_env.yaml \
    && printf "\n  - tip" >> /home/jovyan/singleuser_env.yaml \
    && conda env create -f /home/jovyan/singleuser_env.yaml \
    && rm -rf /home/jovyan/tip_deps_channel /home/jovyan/local-channel

# # Sed replaces the "  - local-channel" entry in the conda env file and replaces it with the three local channels: tip_channel, tip_deps_channel, and local-channel
# RUN mkdir /home/jovyan/tip_channel \
#     && tar xvf local_channel.tar --strip-components=3 --directory=/home/jovyan/tip_channel \
#     && sed '/local-channel/s/.*/  - .\/tip_channel\n  - .\/tip_deps_channel\n  - .\/local-channel/' /home/jovyan/local-channel/local_channel_env.yaml > /home/jovyan/singleuser_env.yaml \
#     && printf "\n  - tip" >> /home/jovyan/singleuser_env.yaml \
#     && conda env create -f /home/jovyan/singleuser_env.yaml \
#     && rm -rf /home/jovyan/tip_deps_channel /home/jovyan/local-channel

ENV PATH="/opt/conda/envs/singleuser/bin:$PATH"

RUN source /opt/conda/bin/activate \
    && conda activate singleuser

RUN rm -rf /opt/conda/pkgs/future-0.18.2-py39hf3d152e_4/lib/python3.9/site-packages/future/backports/test/badcert.pem \
    && rm -rf /opt/conda/pkgs/future-0.18.2-py39hf3d152e_4/lib/python3.9/site-packages/future/backports/test/badkey.pem \
    && rm -rf /opt/conda/pkgs/future-0.18.2-py39hf3d152e_4/lib/python3.9/site-packages/future/backports/test/keycert.passwd.pem \
    && rm -rf /opt/conda/pkgs/future-0.18.2-py39hf3d152e_4/lib/python3.9/site-packages/future/backports/test/keycert.pem \
    && rm -rf /opt/conda/pkgs/future-0.18.2-py39hf3d152e_4/lib/python3.9/site-packages/future/backports/test/keycert2.pem \
    && rm -rf /opt/conda/pkgs/future-0.18.2-py39hf3d152e_4/lib/python3.9/site-packages/future/backports/test/ssl_key.passwd.pem \
    && rm -rf /opt/conda/pkgs/future-0.18.2-py39hf3d152e_4/lib/python3.9/site-packages/future/backports/test/ssl_key.pem \
    && rm -rf /opt/conda/pkgs/tornado-6.1-py39h3811e60_2/lib/python3.9/site-packages/tornado/test/test.key \
    && rm -rf /opt/conda/envs/singleuser/lib/python3.9/site-packages/tornado/test/test.key \
    && rm -rf /opt/conda/envs/torch/lib/python3.9/site-packages/future/backports/test/badcert.pem \
    && rm -rf /opt/conda/envs/torch/lib/python3.9/site-packages/future/backports/test/badkey.pem \
    && rm -rf /opt/conda/envs/torch/lib/python3.9/site-packages/future/backports/test/keycert.passwd.pem \
    && rm -rf /opt/conda/envs/torch/lib/python3.9/site-packages/future/backports/test/keycert.pem \
    && rm -rf /opt/conda/envs/torch/lib/python3.9/site-packages/future/backports/test/keycert2.pem \
    && rm -rf /opt/conda/envs/torch/lib/python3.9/site-packages/future/backports/test/ssl_key.passwd.pem \
    && rm -rf /opt/conda/envs/torch/lib/python3.9/site-packages/future/backports/test/ssl_key.pem \
    && rm -rf /opt/conda/envs/torch/lib/python3.9/site-packages/tornado/test/test.key
