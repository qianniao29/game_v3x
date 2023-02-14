#!/bin/sh

prj_ver=$1
output_file=$2

echo "Version: ${prj_ver}" > ${output_file}
echo "Commit ID: $(git rev-parse HEAD)" >> ${output_file}
echo "Compiling by $(whoami) $(date)" >> ${output_file}
