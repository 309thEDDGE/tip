FROM registry.il2.dsop.io/platform-one/devops/pipeline-templates/hardened-ubi8-stigd-python3.8:8.2

WORKDIR /app 

ARG CMAKE_BUILD_DIR="build"

COPY ${CMAKE_BUILD_DIR}/cpp ./

USER appuser

CMD ["bash"]
