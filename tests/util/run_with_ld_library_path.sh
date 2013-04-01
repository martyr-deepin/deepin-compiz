#!/bin/bash

args=("$@")
ld_library_path=${args[0]}
unset args[0]
LD_LIBRARY_PATH=${ld_library_path}
export LD_LIBRARY_PATH

echo ${args[@]}

exec ${args[@]}
