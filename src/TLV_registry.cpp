#include "TLV_registry.hpp"
#include <stdio.h>

void TLV_registry::set_callback(uint8_t type, callback_func func)
{
  printf("registering callback for 0x%02x\n", type);
  callbacks[type] = func;
}

void TLV_registry::run_callbacks(tlv_packet_t *p)
{
  auto it = callbacks.find(p->header.type);
  if (it != callbacks.end()) {
      it->second(p);
  } else {
      printf("No callback found for 0x%x!\n", p->header.type);
  }
}