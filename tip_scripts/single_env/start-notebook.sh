#!/bin/bash
# Reference: https://github.com/jupyter/docker-stacks/blob/master/base-notebook/start-notebook.sh

#generate conda environment
conda create -n tip tip jupyterlab pandas matplotlib pyarrow \
    -c /home/user/local-channels/singleuser-channel/local_conda-forge \
    -c /home/user/local-channels/tip-package-channel \
    -c /home/user/local-channels/tip-dependencies-channel/local_conda-forge \
    --offline -y > /dev/null

#activate our 'opal-singleuser' conda env
/home/"${NB_USER}"/miniconda3/bin/conda activate tip
set -e

wrapper=""
if [[ "${RESTARTABLE}" == "yes" ]]; then
    wrapper="run-one-constantly"
fi

if [[ -n "${JUPYTERHUB_API_TOKEN}" ]]; then
    # launched by JupyterHub, use single-user entrypoint
    exec /usr/local/bin/start-singleuser.sh "$@"
elif [[ -n "${JUPYTER_ENABLE_LAB}" ]]; then
    # shellcheck disable=SC1091
    . /usr/local/bin/start.sh ${wrapper} jupyter lab "$@"
else
    echo "WARN: Jupyter Notebook deprecation notice https://github.com/jupyter/docker-stacks#jupyter-notebook-deprecation-notice."
    # shellcheck disable=SC1091
    . /usr/local/bin/start.sh ${wrapper} jupyter notebook "$@"
fi
