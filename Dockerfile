FROM registry.il2.dso.mil/platform-one/devops/pipeline-templates/base-image/harden-python38:3.8.5

ARG CMAKE_BUILD_DIR="build"

WORKDIR /app/bin
COPY ${CMAKE_BUILD_DIR}/cpp ./

WORKDIR /app/conf
COPY conf ./

WORKDIR /app/tip_scripts
COPY tip_scripts/run_cl_process.py ./
COPY tip_scripts/exec.py ./
COPY tip_scripts/__init__.py ./

WORKDIR /app
COPY parse_and_translate.py ./

CMD ["/bin/bash"]
