#!/bin/bash
HERE="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
cd "${HERE}"
if [ -r wifi_credentials.rc ] ; then
    source wifi_credentials.rc
else
    echo "ERROR: please provide a file \"wifi_credentials.rc\" which exports \"WIFI_SSID\" and \"WIFI_PASS\""
    exit 1
fi
mkdir -p build
cd build
export PICO_SDK_PATH="${HERE}/../../pico-sdk"
export PICO_EXTRAS_PATH="${HERE}/../../pico-extras"
cmake -Wdev ..
make -j
