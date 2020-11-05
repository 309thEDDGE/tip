 
### General instructions to build a custom base image for the project

- from the root of the project
- `docker build -t tip_base_image:0.1 -f cpp_pipeline_scripts/pipeline.Dockerfile .` 

- this container should have required tools and libraries needed to build the project

- tag and push the container to artifactory for use in the pipeline

### Specific instructions for TIP

Our dockerfile expects deps as the Docker build context.  (This builds the image faster because it only has to copy `tip/deps/` instead of all of `tip/`)

- from the root of the project
- `docker build -t tip_base_image:0.1 -f cpp_pipeline_scripts/pipeline.Dockerfile ./deps`