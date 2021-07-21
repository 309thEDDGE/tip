#!/usr/bin/bash

# If tip environment does not already exist:
conda env list | grep tip &> /dev/null
if [ $? = 1 ]; then
    printf "'tip' Conda env not found\n"
    # Create and activate conda environment with local channel
    conda create -n tip tip jupyterlab pandas matplotlib pyarrow \
     -c /home/user/local-channels/singleuser-channel/local_conda-forge \
     -c /home/user/local-channels/tip-package-channel \
     -c /home/user/local-channels/tip-dependencies-channel/local_conda-forge \
     --offline -y > /dev/null &
    printf "Generating Conda env 'tip'\n"
    PID=$!
    sp="/-\|"
    echo -n ' '
    while [ -d /proc/$PID ]
    do
        printf "\b${sp:i++%${#sp}:1}"
    done
fi

# Activate tip conda environment and run jupyterlab server
printf "Launching Jupyterlab\n"
cd /home/user/ && source activate tip && jupyter lab --ip 0.0.0.0 --no-browser
