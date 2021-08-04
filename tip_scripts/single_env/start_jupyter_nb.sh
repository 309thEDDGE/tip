#!/usr/bin/bash



############################################################
# Main                                                     #
############################################################
Main()
{
    # Checking to see if running from jupyterhub
    conda env list | grep tip &> /dev/null
    if [ -z "$JUPTYERHUB_API_TOKEN"  ]; then
    printf "Launching Jupyterlab\n"
    cd /home/${NB_USER}/
    source /home/${NB_USER}/.bashrc
    conda activate tip
    jupyter lab

    else 
    printf "Launching Jupyterlab\n"
    cd /home/${NB_USER}/
    source /home/${NB_USER}/.bashrc
    conda activate tip
    jupyter labhub
    fi
}

Main
