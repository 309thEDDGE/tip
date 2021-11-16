FROM registry1.dso.mil/ironbank/opensource/metrostar/tip-dependencies:0.0.3 AS tipdependencies
FROM registry1.dso.mil/ironbank/opensource/metrostar/singleuser:0.0.1 AS singleuser

COPY --from=tipdependencies /local_channel /home/jovyan/local_channel

ENV ARTIFACT_DIR=".ci_artifacts/build-metadata/build-artifacts"

WORKDIR /home/jovyan

COPY --chown=jovyan:jovyan $ARTIFACT_DIR/local_channel.tar .

RUN tar xvf local_channel.tar --strip-components=2 && \
    ls -la && \
    pwd && \
    source /opt/conda/bin/activate && \
    conda activate singleuser && \
    conda install -c file:///home/jovyan/local_channel/ -c file:///home/jovyan/local-channel tip --offline

# [OPAL-242] Install upstream dependencies from internet until these are in the singleuser ironbank image.
RUN source /opt/conda/bin/activate && \
    conda activate singleuser && \
    pip install --no-cache-dir \
      s3fs=2021.11.0 \
#      pandas \
#      matplotlib \
      intake=0.6.4 \
      pyarrow=5.0.0 \
      pyyaml=5.4.1 \
      dask=2021.09.01 \
      intake-parquet=0.2.3 

RUN source /opt/conda/bin/activate && \
    jupyter kernelspec list
