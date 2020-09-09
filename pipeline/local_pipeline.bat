set ARTIFACTORY=artifactory.spacecamp.ninja
set IMAGE=%ARTIFACTORY%/docker/platform/ubi8-gcc-gcov-cmake:1.0


docker login %ARTIFACTORY%

@rem Start bash in a container based on the docker image used for the pipeline
docker run -it --rm -v %cd%:/app %IMAGE% bash /app/pipeline/build.sh
if %errorlevel% neq 0 exit /b %errorlevel%
docker run -it --rm -v %cd%:/app %IMAGE% /app/build/cpp/tests
