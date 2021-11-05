#!/usr/bin/bash


############################################################
# Main                                                     #
############################################################
echo "JUPYTERHUB_API_TOKEN ${JUPYTERHUB_API_TOKEN}"
# Checking to see if running from jupyterhub
conda env list | grep tip &> /dev/null
if [[ -n "${JUPYTERHUB_API_TOKEN}" ]]; then
    printf "Launching JupyterHub\n"
    cd /home/${NB_USER}/
    source /home/${NB_USER}/.bashrc
    conda activate singleuser
    jupyter labhub

else
    printf "Launching Jupyterlab\n"
    cd /home/${NB_USER}/
    source /home/${NB_USER}/.bashrc
    conda activate singleuser
    jupyter lab
fi
