#!/bin/bash

source build-release.sh
status=$?
if [[ status -ne 0 ]]; then
    echo "Building FAILED with exit code $status."
    exit $status
fi
./vfat.test
