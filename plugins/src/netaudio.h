#ifndef NETAUDIO_H
#define NETAUDIO_H

#include <stdint.h>
#include <stdlib.h>

enum samplefmt_t : uint16_t { pcm16bit, pcmfloat };

typedef struct {
  uint32_t id;
  uint32_t reserved;
  double srate;
  uint16_t samplefmt;
  uint16_t channels;
  uint32_t fragsize;
  uint64_t chksum;
} netaudio_info_t;

static_assert(sizeof(netaudio_info_t) == 32,
              "size of netaudio_info_t is not 32 bytes");

netaudio_info_t new_netaudio_info(double srate, samplefmt_t samplefmt,
                                  uint16_t channels, uint32_t fragsize);

bool encode_header(const netaudio_info_t& info, char* data, size_t len);

bool decode_header(double& srate, samplefmt_t& samplefmt, uint16_t& channels,
                   uint32_t& fragsize, const char* data, size_t len);

bool encode_audio(const netaudio_info_t& info, const float* audio,
                  size_t num_elem, char* data, size_t len);

bool decode_audio(const netaudio_info_t& info, float* audio, size_t num_elem,
                  const char* data, size_t len);

#endif
/*
 * Local Variables:
 * mode: c++
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * compile-command: "make -C .."
 * End:
 */
