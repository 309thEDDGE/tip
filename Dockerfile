FROM registry.il2.dso.mil/platform-one/devops/pipeline-templates/centos8-gcc-bundle:1.0 AS builder
ENV PATH=/opt/conda/bin:$PATH

RUN mkdir /tip
COPY . /tip

WORKDIR /tip
RUN ./cpp_pipeline_scripts/build.sh
WORKDIR /

FROM registry1.dso.mil/ironbank/opensource/dask-gateway/miniconda:4.9.2
USER root    
COPY --from=builder /local-channel /local-channel
COPY --from=builder /local-mirror /local-mirror
ENV PATH=/opt/conda/bin:$PATH
RUN conda create -n tip_dev tip -c /local-channel -c /local-mirror --dry-run --offline
CMD ["/bin/bash"]
