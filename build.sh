#!/bin/bash

set -e

HERE="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
cd "${HERE}"
if [ -r wifi_credentials.rc ] ; then
    source wifi_credentials.rc
else
    echo "ERROR: please provide a file \"wifi_credentials.rc\" which exports \"WIFI_SSID\" and \"WIFI_PASS\""
    exit 1
fi

export PICO_SDK_PATH="${HERE}/../../pico-sdk"
export PICO_EXTRAS_PATH="${HERE}/../../pico-extras"

# comment out the platforms and architechtures you don't want
# by default builds all
declare -A builds=(
    ["rp2040"]="-DPICO_BOARD=pico_w -DPICO_PLATFORM=rp2040"
    ["rp2350"]="-DPICO_BOARD=pico2_w -DPICO_PLATFORM=rp2350-arm-s"
    ["rp2350-riscv"]="-DPICO_BOARD=pico2_w -DPICO_PLATFORM=rp2350-riscv"
)

for build in "${!builds[@]}"; do
    build_dir="build-${build}"
    build_option="${builds[$build]:-}"

    echo "############################"
    echo "###"
    echo "### Building for ${build} in ${build_dir} with ${build_option}"
    echo "###"
    echo "############################"
    mkdir -p "${build_dir}"
    pushd "${build_dir}" > /dev/null

    cmake -Wdev ${build_option} ..
    make -j

    popd > /dev/null
done
