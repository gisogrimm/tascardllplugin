#include "netaudio.h"
#include <string.h>

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
  info.chksum = gen_crc32b((uint8_t*)(&info),sizeof(info));
  return info;
}

size_t encode_header(const netaudio_info_t& info, char* data, size_t len,
                     netaudio_err_t& err)
{
  err = netaudio_generic_error;
  return 0;
}

size_t decode_header(netaudio_info_t& info, const char* data, size_t len,
                     netaudio_err_t& err)
{
  err = netaudio_generic_error;
  return 0;
}

size_t encode_audio(const netaudio_info_t& info, const float* audio,
                    size_t num_elem, size_t sample_index, char* data,
                    size_t len, netaudio_err_t& err)
{
  err = netaudio_generic_error;
  return 0;
}

size_t decode_audio(const netaudio_info_t& info, float* audio, size_t num_elem,
                    size_t& sample_index, const char* data, size_t len,
                    netaudio_err_t& err)
{
  err = netaudio_generic_error;
  return 0;
}

/*
 * Local Variables:
 * mode: c++
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * compile-command: "make -C .."
 * End:
 */
