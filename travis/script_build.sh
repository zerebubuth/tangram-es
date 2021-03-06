#!/usr/bin/env bash

set -e
set -o pipefail

if [[ ${PLATFORM} == "osx" ]]; then
    # Build osx project
    echo "Building osx project"
    TANGRAM_CMAKE_OPTIONS="-DUNIT_TESTS=1 -DBENCHMARK=1" make -j osx
fi

if [[ ${PLATFORM} == "linux" ]]; then
    # Build linux project
    echo "Building linux project"
    TANGRAM_CMAKE_OPTIONS="-DUNIT_TESTS=1 -DBENCHMARK=1" make -j 4 linux
fi

if [[ ${PLATFORM} == "ios" ]]; then
    # Build ios project
    echo "Building ios project (simulator)"
    make ios-sim
fi

if [[ ${PLATFORM} == "android" ]]; then
    # Build android project
    echo "Building android project"
    export TERM=dumb
    make android
fi

