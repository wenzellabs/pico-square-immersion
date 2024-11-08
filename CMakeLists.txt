cmake_minimum_required(VERSION 3.18)

set(PICO_BOARD pico_w)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
include($ENV{PICO_EXTRAS_PATH}/external/pico_extras_import.cmake)

project(simple-stupid-synth)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_CXX_ENABLE_EXCEPTIONS 1)
add_compile_options(-Wall -Wextra -Wnull-dereference)

# initialize the Pico SDK
pico_sdk_init()

add_executable(simple-stupid-synth
  src/simple-stupid-synth.cpp
  src/midi-state-machine.cpp
  src/audio-target.cpp
  src/i2s-audio-target.cpp
  src/pwm-audio-target.cpp
  src/usb_descriptors.c
  src/wifi-stuff.cpp
  src/network-source.cpp
  src/TLV_registry.cpp
  )

target_compile_definitions(simple-stupid-synth PRIVATE
  PICO_AUDIO_I2S_MONO_OUTPUT=0
  PICO_AUDIO_I2S_MONO_INPUT=0
  WIFI_SSID="$ENV{WIFI_SSID}"
  WIFI_PASS="$ENV{WIFI_PASS}"
  )

target_include_directories(simple-stupid-synth PRIVATE
  src
  ../pico-sdk/lib/tinyusb/src
  ../pico-sdk/lib/tinyusb/hw
  )

# pull in common dependencies
target_link_libraries(simple-stupid-synth
  pico_stdlib
  pico_multicore
  tinyusb_device
  tinyusb_board
  pico_audio_i2s
  pico_audio_pwm
  pico_cyw43_arch_lwip_threadsafe_background
#  pico_cyw43_arch_lwip_poll
  )

pico_enable_stdio_uart(simple-stupid-synth 1)
pico_enable_stdio_usb(simple-stupid-synth 0)
 
# create map/bin/hex file etc.
pico_add_extra_outputs(simple-stupid-synth)

# Local variables:
#   mode: cmake
#   coding: utf-8
# End:
