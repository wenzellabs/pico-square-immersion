#pragma once

#include <unordered_map>
#include <functional>
#include <cstdint>
#include <tlv.h>

class TLV_registry
{
public:
  using callback_func = std::function<void(tlv_packet_t*)>;

  // Registers a callback function for a specific TLV type
  void set_callback(uint8_t type, callback_func func);

  // Executes the registered callback function based on the TLV packet type
  void run_callbacks(tlv_packet_t *p);

private:
    std::unordered_map<uint8_t, callback_func> callbacks; // Maps TLV types to their corresponding callbacks
};
