# Test Argo Workflow #

## Setup ##

### Minikube ###
Simple recommended method for local installation of kubernetes.

* https://kubernetes.io/docs/setup/learning-environment/minikube/
* Use virtualbox on rhel 8
* virtualization enabled at hardware level
* `minikube start`
* Stop minikube before shutting down or sleeping
* To start dashboard and view in browser (run locally): 
  * `minikube dashboard`

### Argo ###
Install:

* https://github.com/argoproj/argo
* `kubectl create namespace argo`
* ~~`kubectl apply -n argo -f https://raw.githubusercontent.com/argoproj/argo/stable/manifests/install.yaml`~~
* `kubectl apply -n argo -f https://raw.githubusercontent.com/argoproj/argo/stable/manifests/quick-start-postgres.yaml`
* To view server and port-forward if using local kubernetes (i.e. minikube or Docker for desktop):
  * `kubectl -n argo port-forward deployment/argo-server 2746:2746`
* View http://localhost:2746/workkflows/ to view Argo Workflows gui

### Minio ###

**Note: Neither Minio nor an artifact store were used in the course the Argo tests that follow.**

Use minio as artifact store. Artifacts can be written to or read from store at each step (container instantiation).
Install
* Install Helm, package manager for kubernetes:
* https://helm.sh/docs/intro/install/
* `$ curl -fsSL -o get_helm.sh https://raw.githubusercontent.com/helm/helm/master/scripts/get-helm-3`
* `$ chmod 700 get_helm.sh`
* `$ ./get_helm.sh`
* Install minio pod in kubernetes cluster using Helm:
  * kubernetes must be running, kubectl is effective
  * `$ helm repo add stable https://kubernetes-charts.storage.googleapis.com/` # official Helm stable charts
  * $ `helm repo update`
  * $ `helm install argo-artifacts stable/minio --set service.type=LoadBalancer --set fullnameOverride=argo-artifacts`
* Helm indicates the chart is deprecated. Not sure which is correct. 
* View the Kubernetes Dashboard to confirm argo-artifacts is deployed
* View the minio UI using a browser:
  * To get the external IP, do `minikube service --url argo-artifacts`
  * When installed via helm, minio has the following defaults:
    * AccessKey: AKIAIOSFODNN7EXAMPLE
    * SecretKey: wJalrXUtnFEMI/K7MDENG/bPxRfiCYEXAMPLEKEY
  * Create a bucket called my-bucket

### Load Custom Images Into Minikube ###
Kubernetes runs in the VM with its own docker daemon. Even if you build a custom docker image on the machine which runs the VM, kubernetes can't see the image. It must be built or loaded with the docker daemon present in the same environment as kubernetes. 

* ~~To set the local env vars to the VM docker vars: `eval $(minikube docker-env)` (persistent only for the life of the shell in which command was issued)~~
* ~~to restore: `eval $(minikube docker-env -u)`~~
* _above command fails along with other commands noted in https://minikube.sigs.k8s.io/docs/handbook/pushing/_
* Only found success with `minikube ssh` method
* requires to mount location of Dockerfile and files using `minikube mount <source directory>:<target directory>`
* after mount, ssh into minikube vm and use docker daemon inside the vm to build/load image	

## Argo Workflow Submission ##

* start minikube
* `minikube dashboard` then go to url to view docker UI
* kubectl argo namespace port-forward to view argo UI
* ~~`argo submit <argo workflow>.yml`~~
* `argo submit -n argo <argo workflow>.yml`

## Argo Testing, Phase 1 ## 

#### Goal ####

Initiate a single container with no descendant or parallel containers. The image is the docker python base image with yaml added via pip and a python script, yaml_from_input_args.py, which reads some input arguments and writes a yaml file to /app. These files and the Docker file are located in argo/docker/faux_front_end. This faux front end container has as its entrypoint the python script. It takes arguments given to the container and writes them into the yaml file as though it were a web ui collecting configuration information from a user. If this step can be completed successfully then the next step will connect the yaml file as input to the next container, which is the tip container.

#### Issues ####

The argo submission seems to do its job, the pod is registered with the correct name and begins to run according the k8s dashboard, argo UI shows the workflow added and starting. After the expected script run time (with sleep statements), the pod fails according to kubernetes and argo. However, the output logs which include stdout show the script expected output, which also shows a cat of the yaml the file which is written. Everything in the logs indicates that the container functioned as expected. Fail modes are:

* k8s:
  * Initialized = true
  * Ready = False; Reason = ContainersNotReady; Message = containers with unready status: \[wait main\]
  * ContainersReady = False; Reason = \<same as above\>; Message = \<same as above\>
  * PodScheduled = True
* argo:
  * (end) phase: Eror
  * Message:
    * failed to save outputs
    * failed to establish pod watch
    * timed out waiting for the condition
  
My assumption about the failure messages is that they are generated by k8s and not argo. 

Conclusion: The errors originate from a permissions issue which may be caused by using the wrong/deprecated Argo install template (see Argo: Install steps) and/or use of the incorrect name space. Use `argo submit -n argo \<template name\>` to submit jobs. After re-installing argo with new template and using the correct namespace during submission, the errors go away.

## Argo Testing, Phase 2 ##

#### Goal ####

Pass arg(s) to initial "front end" container, record args to yaml output, retrieve args with second container and write parsed yaml to stdout. Completed.

## Argo Testing, Phase 3 ##

#### Goal ####

Configure all args necessary to execute parse_and_translate.py in second container. Use mount (-v) to access ch10 data and ICD in second container. Parse and translate Ch10 and exit.

* __COMPLETE__ configure args to faux front end: 
  * external ch10 mount point: _ext\_ch10\_mount\_point_ - used to create mount point for second container, passed as part of yaml file to second container manager
  * external icd mount point: _ext\_icd\_mount\_point_ - used to create mount point for second container, passed as part of yaml file to second container manager
  * ch10 file name: _ch10\_file\_name_ - name of the ch10 located at _ext\_ch10\_mount\_point_, recorded in yaml file
  * icd file name: _icd\_file\_name_ - name of icd located at _ext\_icd\_mount\_point_, recorded in yaml file
  * overwrite boolean: _overwrite_ - flag passed to parse\_and\_translate.py, recorded in yaml file
  * video generation boolean: _video_ - flag passed to parse\_and\_translate.py, recorded in yaml file
* __COMPLETE__ update centos container tip build: The original plan involved modifying the pipeline-built TIP image to utilize the correct directory structure, include python files to run parse\_and\_translate.py, etc. However, the pipeline is currently broken so I updated the centos TIP image in tip/container/docker/centos to build and include libirig106, gtest/gmock, libtins, libpcap. I've done this and confirmed that parse\_and\_translate.py works as expected.
* __COMPLETE__ modify centos image to include the run\_tip.py entry point, import image into minikube cluster
* __COMPLETE__ second container, mount ch10 and ICD mount points (Note: the method of creating a mount point in the container in practice will be very similar to what I've done here. The difference is that a local mount point will never be used in production.)
* __COMPLETE__ second container, create script to interpret yaml config and execute parse\_and\_translate.py
  * successful execution of tip\_parse, apparent issue with tip\_translate, failed to parse first line of csv icd, found 26 cols instead of required 25. This may be due to subtle difference of certain functions in Linux vs Win.
* ~~modify tip build to create image with appropriate directory structure:~~ (see update centos container tip build)
  * /app/bin - all libs and binaries
  * /app/conf - yaml configuration files, fixed, also includes /app/conf/default which can be used by parse\_and\_translate.py
  * /app/parse\_and\_translate.py - script in root
  * /app/tip\_scripts/ - needed by parse\_and\_translate: run\_cl\_process.py, exec.py  

## Argo Testing, Phase 4 ##

Trigger Argo workflow: https://argoproj.github.io/argo-events/triggers/argo-workflow/. Other information received from the boxboat contractors indicate that a better option for triggering workflows is Argo CD. In this exercise Argo workflows are triggered manually via the argo submit command.

Goal: Modify faux front end to test retryStrategy and graceful termination. Introduce a new second step, which is a script template, that parses the output json generated at step one and loops over the output, creating multiple parallel containers for parsing in the 3rd step. 

* __COMPLETE__ Modify faux front end:
  * workflow: set retry strategy to Always, understand retry limit (see comments in run\_tip\_from\_faux\_front\_end.yml)
  * entry point: kill switch via input args to test retry
  * entry point: graceful termination (Already implemented via sigterm signal catching? - Yes, when terminate is sent via Argo UI, signal is caught and indicated via print statement in the logs and the next step is not initiated by the workflow.)
  * entry point: generate json output instead of yaml. Json is included by default in the python library and more relevant to Argo workflows because json is the default data mapping language for passing between templates even though the Argo workflow itself is written in yaml.
* __COMPLETE__ Modify tip_custom container to interpret json instead of yaml
* __COMPLETE__ New second step:
  * Script template, python, interpret json from first step, return json list of dictionaries specifying the mount points, ch10 file names, etc., to be executed by the third step
* __COMPLETE__ New third step:
  * loop over list of json dicts/maps created by the script template to create one or more parallel tip parse/translate containers based on the output from the front end step
  * handle different mount points for each tip container that is created


