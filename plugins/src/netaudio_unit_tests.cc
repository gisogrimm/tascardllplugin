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
  size = decode_header(inf2, "012345678901234567890123456789",30, err);
  EXPECT_EQ(0u, size);
  EXPECT_EQ(netaudio_not_a_header, err);
}

// Local Variables:
// compile-command: "make -C .. unit-tests"
// coding: utf-8-unix
// c-basic-offset: 2
// indent-tabs-mode: nil
// End:
