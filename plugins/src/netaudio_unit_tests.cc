#include <gtest/gtest.h>

#include "netaudio.h"

TEST(netaudio, new_netaudio_info)
{
  netaudio_info_t inf(new_netaudio_info(44100, pcm16bit, 2, 64));
  EXPECT_EQ(44100, inf.srate);
  EXPECT_EQ(pcm16bit, inf.samplefmt);
  EXPECT_EQ(2u, inf.channels);
  EXPECT_EQ(64u, inf.fragsize);
  EXPECT_EQ(1u, inf.id);
  EXPECT_EQ(2259497000u, inf.chksum);
  netaudio_info_t inf2(new_netaudio_info(44101, pcm16bit, 2, 64));
  EXPECT_EQ(1193537512u, inf2.chksum);
}

TEST(netaudio, encode_header_errors)
{
  netaudio_info_t inf(new_netaudio_info(44100, pcm16bit, 2, 64));
  netaudio_err_t err;
  size_t size;
  size = encode_header(inf, NULL, 31, err);
  EXPECT_EQ(0u, size);
  EXPECT_EQ(netaudio_invalid_pointer, err);
  char char1[1];
  size = encode_header(inf, char1, 1, err);
  EXPECT_EQ(0u, size);
  EXPECT_EQ(netaudio_unsufficient_memory, err);
  char char128[128];
  size = encode_header(inf, char128, 128, err);
  EXPECT_EQ(netaudio_success, err);
}

TEST(netaudio, decode_header_errors)
{
  netaudio_info_t inf(new_netaudio_info(44100, pcm16bit, 2, 64));
  netaudio_info_t inf2;
  netaudio_err_t err;
  size_t size;
  /* not yet tested:
   * - netaudio_unsupported_protocol_version: the protocol id is not
   *   supported
   * - netaudio_invalid_checksum: the checksum is invalid.
   */
  size = decode_header(inf2, NULL, 1, err);
  EXPECT_EQ(0u, size);
  EXPECT_EQ(netaudio_invalid_pointer, err);
  char char1[1];
  size = decode_header(inf2, char1, 1, err);
  EXPECT_EQ(0u, size);
  EXPECT_EQ(netaudio_unsufficient_memory, err);
  size = decode_header(inf2, "012345678901234567890123456789", 30, err);
  EXPECT_EQ(0u, size);
  EXPECT_EQ(netaudio_not_a_header, err);
}

TEST(netaudio, encode_decode_header)
{
  netaudio_info_t inf(new_netaudio_info(44100, pcm16bit, 2, 64));
  netaudio_info_t inf2;
  netaudio_err_t err;
  size_t size;
  char char128[128];
  size = encode_header(inf, char128, 128, err);
  EXPECT_EQ(netaudio_success, err);
  size_t size2;
  size2 = decode_header(inf2, char128, size, err);
  EXPECT_EQ(netaudio_success, err);
  EXPECT_EQ(size, size2);
  EXPECT_EQ(inf.srate, inf2.srate);
  EXPECT_EQ(inf.samplefmt, inf2.samplefmt);
  EXPECT_EQ(inf.channels, inf2.channels);
  EXPECT_EQ(inf.fragsize, inf2.fragsize);
  char128[7]++;
  size2 = decode_header(inf2, char128, size, err);
  EXPECT_EQ(netaudio_invalid_checksum, err);
  size = encode_header(inf, char128, 128, err);
  char128[1]++;
  size2 = decode_header(inf2, char128, size, err);
  EXPECT_EQ(netaudio_unsupported_protocol_version, err);
}

TEST(netaudio, encode_audio_errors)
{
  netaudio_info_t info(new_netaudio_info(44100, pcm16bit, 2, 64));
  size_t size;
  float f1[1];
  char char1k[1024];
  netaudio_err_t err;
  size = encode_audio(info, NULL, 128, 0, char1k, 1024, err);
  EXPECT_EQ(0u, size);
  EXPECT_EQ(netaudio_invalid_pointer, err);
  size = encode_audio(info, f1, 1, 0, char1k, 1024, err);
  EXPECT_EQ(0u, size);
  EXPECT_EQ(netaudio_invalid_buffer_dimensions, err);
  size = encode_audio(info, f1, 128, 0, NULL, 1024, err);
  EXPECT_EQ(0u, size);
  EXPECT_EQ(netaudio_invalid_pointer, err);
  size = encode_audio(info, f1, 128, 0, char1k, 128, err);
  EXPECT_EQ(0u, size);
  EXPECT_EQ(netaudio_unsufficient_memory, err);
  size = encode_audio(info, f1, 128, 0, char1k, 1024, err);
  EXPECT_EQ(netaudio_success, err);
}

// Local Variables:
// compile-command: "make -C .. unit-tests"
// coding: utf-8-unix
// c-basic-offset: 2
// indent-tabs-mode: nil
// End:
