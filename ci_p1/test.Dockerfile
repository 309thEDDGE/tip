# To be run with a context of the test set directory containing truth/ test/ and log/
# docker build -f <tip>/ci_p1/test.Dockerfile <my_test_directory>
# Example:
# docker build -t registry.il2.dsop.io/skicamp/project-opal/tip:tip-test -f ci_p1/test.Dockerfile <test_dir_path>
# docker run -it --rm -v $PWD:/app registry.il2.dsop.io/skicamp/project-opal/tip:tip-test bash

FROM registry.il2.dsop.io/platform-one/devops/pipeline-templates/ubi8-python3.8:8.1

# Use the WORKDIR to create our destination directory
RUN mkdir -p /test/truth /test/test /test/log
COPY ./truth /test/truth/
