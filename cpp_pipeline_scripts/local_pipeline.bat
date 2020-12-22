set REGISTRY=registry.il2.dso.mil
set BUILD_IMAGE=%REGISTRY%/skicamp/project-opal/tip/build:MR57
set TEST_IMAGE=%REGISTRY%/skicamp/project-opal/tip/test:1.0


docker login %REGISTRY%

@rem Run build and test scripts in a container based on the docker image used for the pipeline
docker run -it --rm --env ALKEMIST_LICENSE_KEY -v %cd%:/app %BUILD_IMAGE% bash /app/cpp_pipeline_scripts/build.sh
if %errorlevel% neq 0 exit /b %errorlevel%
docker run -it --rm -v %cd%:/app %TEST_IMAGE% bash /app/cpp_pipeline_scripts/unit-test.sh
