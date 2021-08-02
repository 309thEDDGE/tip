#!/usr/bin/bash

# Default vars
PACKAGE_LIST="tip jupyterlab pandas matplotlib pyarrow"
LAB_TYPE="lab"

############################################################
# Help                                                     #
############################################################
Help()
{
   # Display Help
   echo "Run a singleuser jupyterlab instance"
   echo
   echo "Syntax: start_jupyterhub_nb.sh [-|h|D]"
   echo "options:"
   echo "h     Print this Help."
   echo "D     Run a (Distributed) lab instance for jupyterhub"
   echo
}

############################################################
# Main                                                     #
############################################################
Main()
{
    # If tip environment does not already exist:
    conda env list | grep tip &> /dev/null
    if [ $? = 1 ]; then
        printf "'tip' Conda env not found\n"
        # Create and activate conda environment with local channel
        conda create -n tip ${PACKAGE_LIST} \
            -c /home/${NB_USER}/local-channels/singleuser-channel/local_conda-forge \
            -c /home/${NB_USER}/local-channels/tip-package-channel \
            -c /home/${NB_USER}/local-channels/tip-dependencies-channel/local_conda-forge \
            --offline -y > /dev/null &
        printf "Generating Conda env 'tip'\n"
        PID=$!
        sp="/-\|"
        echo -n ' '
        while [ -d /proc/$PID ]
        do
            printf "\b${sp:i++%${#sp}:1}"
            sleep 0.2
        done
    fi

    # Activate tip conda environment and run jupyterlab server
    printf "Launching Jupyterlab\n"
    cd /home/${NB_USER}/ && conda activate tip && jupyter ${LAB_TYPE}
}

while getopts "hD" option; do
    case $option in
        h)  # display help
            Help
            exit;;
        D)  # set vars for hub
            PACKAGE_LIST="${PACKAGE_LIST} jupyterhub"
            LAB_TYPE="labhub"
            ;;
        \?) # Invalid option
            echo "Error: Invalid option"
            exit;;
    esac
done

Main
