 
### build a custom base image for the project

- from the root of the project
- `docker build --build-arg RHEL_SUB_USERNAME={username} --build-arg RHEL_SUB_PASSWORD={password} -t tip_base_image:0.1 -f pipeline/pipeline.Dockerfile .` 

- this container should have required tools and libraries needed to build the project

- tag and push the container to artifactory for use in the pipeline
