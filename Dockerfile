FROM registry1.dso.mil/ironbank/opensource/metrostar/tip-dependencies:0.0.3 AS tipdependencies
FROM registry1.dso.mil/ironbank/opensource/metrostar/singleuser:0.0.1 AS singleuser

COPY --from=tipdependencies /local_channel /home/jovyan/local_channel

ENV ARTIFACT_DIR=".ci_artifacts/build-metadata/build-artifacts"

WORKDIR /home/jovyan

COPY --chown=jovyan:jovyan $ARTIFACT_DIR/local_channel.tar .
COPY --chown=jovyan:jovyan ./conf /home/jovyan/

RUN tar xvf local_channel.tar --strip-components=2 && \
    ls -la && \
    pwd && \
    source /opt/conda/bin/activate && \
    conda activate singleuser && \
    conda install -c file:///home/jovyan/local_channel/ -c file:///home/jovyan/local-channel tip --offline && \
    pip install --no-cache-dir  s3fs==2021.11.0 \
      pandas==1.3.5 \
      matplotlib==3.5.1 \
      intake==0.6.5 \
      pyarrow==6.0.1 \
      pyyaml==5.4.1 \
      dask==2022.1.0 \
      intake-parquet==0.2.3

RUN rm -rf /opt/conda/pkgs/future-0.18.2-py39hf3d152e_4/lib/python3.9/site-packages/future/backports/test/badcert.pem && \
    rm -rf /opt/conda/pkgs/future-0.18.2-py39hf3d152e_4/lib/python3.9/site-packages/future/backports/test/badkey.pem && \
    rm -rf /opt/conda/pkgs/future-0.18.2-py39hf3d152e_4/lib/python3.9/site-packages/future/backports/test/keycert.passwd.pem && \
    rm -rf /opt/conda/pkgs/future-0.18.2-py39hf3d152e_4/lib/python3.9/site-packages/future/backports/test/keycert.pem && \
    rm -rf /opt/conda/pkgs/future-0.18.2-py39hf3d152e_4/lib/python3.9/site-packages/future/backports/test/keycert2.pem && \
    rm -rf /opt/conda/pkgs/future-0.18.2-py39hf3d152e_4/lib/python3.9/site-packages/future/backports/test/ssl_key.passwd.pem && \
    rm -rf /opt/conda/pkgs/future-0.18.2-py39hf3d152e_4/lib/python3.9/site-packages/future/backports/test/ssl_key.pem && \
    rm -rf /opt/conda/pkgs/tornado-6.1-py39h3811e60_2/lib/python3.9/site-packages/tornado/test/test.key

