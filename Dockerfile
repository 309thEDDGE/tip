FROM registry1.dso.mil/ironbank/opensource/metrostar/tip-dependencies:0.0.3 AS tipdependencies
FROM registry1.dso.mil/ironbank/opensource/metrostar/singleuser:0.0.1 AS singleuser

COPY --from=tipdependencies /local_channel /home/jovyan/local_channel

ENV ARTIFACT_DIR=".ci_artifacts/build-metadata/build-artifacts"

WORKDIR /home/jovyan

COPY $ARTIFACT_DIR/local_channel.tar .

RUN tar xvf local_channel.tar && \
    source /opt/conda/bin/activate && \
    conda activate singleuser && \
    conda install -c /home/jovyan/local_channel/ -c /home/jovyan/local-channel tip --offline

WORKDIR /home/jovyan/user_scripts

COPY --chown=jovyan:jovyan tip_scripts/single_env/start_jupyter_nb.sh .

EXPOSE 8888

ENTRYPOINT ["/usr/bin/bash", "/home/jovyan/user_scripts/start_jupyter_nb.sh"]
