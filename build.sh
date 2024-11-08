#!/bin/bash

HERE="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
cd "${HERE}"
mkdir -p build
cd build
export PICO_SDK_PATH="${HERE}/../pico-sdk"
export PICO_EXTRAS_PATH="${HERE}/../pico-extras"
cmake -Wdev ..
make -j
