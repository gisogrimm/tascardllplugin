#include "netaudio.h"
#include <string.h>

#define NETAUDIO_HEADER '\001'
#define NETAUDIO_AUDIO '\002'

#define CRC16 0x8005

uint16_t gen_crc16(const uint8_t* data, uint16_t size)
{
  uint16_t out = 0;
  int bits_read = 0, bit_flag;

  /* Sanity check: */
  if(data == NULL)
    return 0;

  while(size > 0) {
    bit_flag = out >> 15;

    /* Get next bit: */
    out <<= 1;
    out |= (*data >> bits_read) &
           1; // item a) work from the least significant bits

    /* Increment bit counter: */
    bits_read++;
    if(bits_read > 7) {
      bits_read = 0;
      data++;
      size--;
    }

    /* Cycle check: */
    if(bit_flag)
      out ^= CRC16;
  }

  // item b) "push out" the last 16 bits
  int i;
  for(i = 0; i < 16; ++i) {
    bit_flag = out >> 15;
    out <<= 1;
    if(bit_flag)
      out ^= CRC16;
  }

  // item c) reverse the bits
  uint16_t crc = 0;
  i = 0x8000;
  int j = 0x0001;
  for(; i != 0; i >>= 1, j <<= 1) {
    if(i & out)
      crc |= j;
  }

  return crc;
}

uint32_t gen_crc32b(const uint8_t* data, uint16_t size)
{
  int j;
  unsigned int byte, crc, mask;

  crc = 0xFFFFFFFF;
  while(size > 0) {
    byte = *data; // Get next byte.
    crc = crc ^ byte;
    for(j = 7; j >= 0; j--) { // Do eight times.
      mask = -(crc & 1);
      crc = (crc >> 1) ^ (0xEDB88320 & mask);
    }
    --size;
    ++data;
  }
  return ~crc;
}

uint32_t get_checksum(netaudio_info_t info)
{
  info.chksum = 0;
  return gen_crc32b((uint8_t*)(&info), sizeof(info));
}

netaudio_info_t new_netaudio_info(double srate, samplefmt_t samplefmt,
                                  uint16_t channels, uint32_t fragsize)
{
  netaudio_info_t info;
  memset(&info, 0, sizeof(info));
  info.id = 1;
  info.srate = srate;
  info.samplefmt = samplefmt;
  info.channels = channels;
  info.fragsize = fragsize;
  info.chksum = get_checksum(info);
  return info;
}

size_t encode_header(const netaudio_info_t& info, char* data, size_t len,
                     netaudio_err_t& err)
{
  if(!data) {
    err = netaudio_invalid_pointer;
    return 0u;
  }
  if(len < get_buffer_length_header()) {
    err = netaudio_insufficient_memory;
    return 0u;
  }
  // mark data block to be a header:
  data[0] = NETAUDIO_HEADER;
  memcpy(&(data[1]), &info, sizeof(netaudio_info_t));
  err = netaudio_success;
  return sizeof(netaudio_info_t) + 1;
}

size_t get_buffer_length_header()
{
  return sizeof(netaudio_info_t) + 1;
}

size_t decode_header(netaudio_info_t& info, const char* data, size_t len,
                     netaudio_err_t& err)
{
  if(!data) {
    err = netaudio_invalid_pointer;
    return 0u;
  }
  if(len < sizeof(netaudio_info_t) + 1) {
    err = netaudio_insufficient_memory;
    return 0u;
  }
  if(data[0] != NETAUDIO_HEADER) {
    err = netaudio_not_a_header;
    return 0u;
  }
  netaudio_info_t newinfo;
  memcpy(&newinfo, &(data[1]), sizeof(netaudio_info_t));
  if(get_checksum(newinfo) != newinfo.chksum) {
    err = netaudio_invalid_checksum;
    return 0u;
  }
  if(1 != newinfo.id) {
    err = netaudio_unsupported_protocol_version;
    return 0u;
  }
  info = newinfo;
  err = netaudio_success;
  return sizeof(netaudio_info_t) + 1;
}

size_t encode_audio(const netaudio_info_t& info, const float* audio,
                    size_t num_elem, uint32_t sample_index, char* data,
                    size_t len, netaudio_err_t& err)
{
  if(!audio) {
    err = netaudio_invalid_pointer;
    return 0u;
  }
  if(!data) {
    err = netaudio_invalid_pointer;
    return 0u;
  }
  if(num_elem != info.fragsize * info.channels) {
    err = netaudio_invalid_buffer_dimensions;
    return 0u;
  }
  size_t requiredlen(get_buffer_length(info));
  if(len < requiredlen) {
    err = netaudio_insufficient_memory;
    return 0u;
  }
  data[0] = NETAUDIO_AUDIO;
  memcpy(&(data[1]), &(info.chksum), sizeof(info.chksum));
  data += 1 + sizeof(info.chksum);
  memcpy(data,&sample_index,sizeof(uint32_t));
  data += 4u;
  switch(info.samplefmt) {
  case pcm16bit:
    for(size_t k = 0; k < num_elem; ++k) {
      int16_t v(audio[k] * ((1 << 15) - 1));
      memcpy(data, &v, sizeof(int16_t));
      data += sizeof(int16_t);
    }
    break;
  case pcmfloat:
    memcpy(data, audio, sizeof(float) * num_elem);
    break;
  }
  err = netaudio_success;
  return requiredlen;
}

size_t decode_audio(const netaudio_info_t& info, float* audio, size_t num_elem,
                    uint32_t& sample_index, const char* data, size_t len,
                    netaudio_err_t& err)
{
  if(!audio) {
    err = netaudio_invalid_pointer;
    return 0u;
  }
  if(!data) {
    err = netaudio_invalid_pointer;
    return 0u;
  }
  if(num_elem != info.fragsize * info.channels) {
    err = netaudio_invalid_buffer_dimensions;
    return 0u;
  }
  size_t requiredlen(get_buffer_length(info));
  if(len < requiredlen) {
    err = netaudio_insufficient_memory;
    return 0u;
  }
  if(data[0] != NETAUDIO_AUDIO) {
    err = netaudio_no_audiochunk;
    return 0u;
  }
  uint32_t chksum;
  memcpy(&chksum, &(data[1]), sizeof(chksum));
  if(chksum != info.chksum) {
    err = netaudio_invalid_checksum;
    return 0u;
  }
  data += 1 + sizeof(chksum);
  memcpy(&sample_index,data,sizeof(uint32_t));
  data += 4u;
  switch(info.samplefmt) {
  case pcm16bit:
    for(size_t k = 0; k < num_elem; ++k) {
      int16_t v;
      memcpy(&v, data, sizeof(int16_t));
      audio[k] = v * (1.0f / ((1 << 15) - 1));
      data += sizeof(int16_t);
    }
    break;
  case pcmfloat:
    memcpy(audio, data, sizeof(float) * num_elem);
    break;
  }
  err = netaudio_success;
  return requiredlen;
}

size_t get_buffer_length(const netaudio_info_t& info)
{
  size_t requiredlen;
  switch(info.samplefmt) {
  case pcm16bit:
    requiredlen = info.channels * info.fragsize * sizeof(int16_t);
    break;
  case pcmfloat:
    requiredlen = info.channels * info.fragsize * sizeof(float);
    break;
  }
  return requiredlen + 1 + sizeof(info.chksum) + 4;
}

/*
 * Local Variables:
 * mode: c++
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * compile-command: "make -C .."
 * End:
 */
