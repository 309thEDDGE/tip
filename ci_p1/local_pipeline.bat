set REGISTRY=registry.il2.dsop.io
set IMAGE=%REGISTRY%/skicamp/project-opal/tip:tip-deps


docker login %REGISTRY%

@rem Run build and test scripts in a container based on the docker image used for the pipeline
docker run -it --rm -v %cd%:/app %IMAGE% bash /app/ci_p1/build.sh
if %errorlevel% neq 0 exit /b %errorlevel%
docker run -it --rm -v %cd%:/app %IMAGE% bash /app/ci_p1/unit-test.sh
