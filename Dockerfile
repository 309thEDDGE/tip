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
ENV PATH=/root/miniconda3/bin:$PATH
RUN conda create -n tip_dev tip -c /local-channel -c /local-mirror --dry-run --offline
CMD ["/bin/bash"]
