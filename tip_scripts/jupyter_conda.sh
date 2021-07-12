#!/usr/bin/bash

# If tip environment does not already exist:
conda env list | grep tip &> /dev/null
printf "$?\n"
if [ $? = 0 ]; then
    printf "'tip' Conda env not found\n"
    # Create and activate conda environment with local channel
    conda create -n tip tip jupyterlab pandas matplotlib pyarrow -c file:///home/user/local-channel -y > /dev/null &
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
