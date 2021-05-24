FROM registry.il2.dso.mil/platform-one/devops/pipeline-templates/base-image/harden-python38:3.8.9

ARG CMAKE_BUILD_DIR="build"

WORKDIR /app/headers
COPY cpp ./

WORKDIR /app/bin
COPY ${CMAKE_BUILD_DIR}/cpp ./

WORKDIR /app/conf
COPY conf ./

CMD ["/bin/bash"]
