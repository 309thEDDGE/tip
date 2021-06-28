FROM registry.il2.dso.mil/platform-one/devops/pipeline-templates/centos8-gcc-bundle:1.0 AS builder

ENV PATH=/opt/conda/bin:$PATH

RUN mkdir /tip
# Tip source 
COPY cpp_pipeline_scripts /tip/cpp_pipeline_scripts
COPY tip_scripts /tip/tip_scripts/
COPY cpp /tip/cpp
# CMake files explicity
COPY CMake_conda.txt /tip/CMake_conda.txt
COPY CMake_linux.txt /tip/CMake_linux.txt
COPY CMakeLists.txt  /tip/CMakeLists.txt
COPY CMake_windows.txt /tip/CMake_windows.txt
# Add README for tip Licence meta.yaml requirement in conda-recepies 
COPY README.md /tip/README.md

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
