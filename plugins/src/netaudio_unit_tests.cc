#include <gtest/gtest.h>

#include "netaudio.h"

TEST(netaudio, new_netaudio_info)
{
  netaudio_info_t inf(new_netaudio_info(44100, pcm16bit, 2, 64));
  EXPECT_EQ(inf.srate,44100);
  EXPECT_EQ(inf.samplefmt,pcm16bit);
  EXPECT_EQ(inf.channels,2u);
  EXPECT_EQ(inf.fragsize,64u);
  EXPECT_EQ(inf.id,1);
  EXPECT_EQ(inf.chksum,2259497000u);
  netaudio_info_t inf2(new_netaudio_info(44101, pcm16bit, 2, 64));
  EXPECT_EQ(inf2.chksum,1193537512u);
}

// Local Variables:
// compile-command: "make -C .. unit-tests"
// coding: utf-8-unix
// c-basic-offset: 2
// indent-tabs-mode: nil
// End:
