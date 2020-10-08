FROM registry.il2.dsop.io/platform-one/devops/pipeline-templates/ubi8-gcc-bundle:1.0

#
# Import tip dependencies
#
WORKDIR /deps
COPY . /
