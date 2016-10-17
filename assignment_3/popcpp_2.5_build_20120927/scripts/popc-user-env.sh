#!/bin/sh
if [ -z "${POPC_LOCATION}" ]; then
POPC_LOCATION=/usr/local/popc
export POPC_LOCATION
fi

POPC_PLUGIN_LOCATION=/usr/local/popc/lib/plugins
export POPC_PLUGIN_LOCATION

PATH=${POPC_LOCATION}/bin:${PATH}
export PATH

PATH=${POPC_LOCATION}/sbin:${PATH}
export PATH

LD_LIBRARY_PATH=${POPC_LOCATION}/lib:${LD_LIBRARY_PATH}
export LD_LIBRARY_PATH
