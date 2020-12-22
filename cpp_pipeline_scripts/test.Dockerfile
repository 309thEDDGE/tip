# Build with a context of the truth data directory
# docker build -f <tip>/cpp_pipeline_scripts/test.Dockerfile <my_truth_directory>
# Example:
# docker build -t registry.il2.dso.mil/skicamp/project-opal/tip:tip-test -f cpp_pipeline_scripts/test.Dockerfile <truth_dir_path>
# docker run -it --rm -v $PWD:/app registry.il2.dso.mil/skicamp/project-opal/tip:tip-test bash

FROM registry.il2.dso.mil/platform-one/devops/pipeline-templates/centos8-gcc-bundle:1.0

ENV YUMOPT="dnf install -y"

RUN dnf install python3 ; alternatives --set python /usr/bin/python3

RUN pip3 install numpy
RUN pip3 install pyyaml
RUN pip3 install deepdiff

# Use the WORKDIR to create our destination directory
RUN mkdir -p /test/truth /test/test /test/log
COPY . /test/truth/
