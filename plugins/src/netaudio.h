#ifndef NETAUDIO_H
#define NETAUDIO_H

#include <stdint.h>

typedef struct {
  double srate;
  uint16_t channels;
  uint16_t samplefmt;
  uint32_t fragsize;
} netaudio_info_t;

static_assert(sizeof(netaudio_info_t) == 16,
              "size of netaudio_info_t is not 16 bytes");

#endif
/*
 * Local Variables:
 * mode: c++
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * compile-command: "make -C .."
 * End:
 */
