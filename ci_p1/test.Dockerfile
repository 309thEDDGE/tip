# To be run with a context of the test set directory containing truth/ test/ and log/
# docker build -f <tip>/ci_p1/test.Dockerfile <my_test_directory>
# Example:
# docker build -t registry.il2.dsop.io/skicamp/project-opal/tip:tip-test -f ci_p1/test.Dockerfile <test_dir_path>
# docker run -it --rm -v $PWD:/app registry.il2.dsop.io/skicamp/project-opal/tip:tip-test bash

FROM registry.il2.dsop.io/platform-one/devops/pipeline-templates/centos8-gcc-bundle:1.0

ENV YUMOPT="dnf install -y"

RUN dnf install python3 ; alternatives --set python /usr/bin/python3

RUN pip3 install numpy

# Use the WORKDIR to create our destination directory
WORKDIR /test/truth
COPY ./truth /test/truth/
