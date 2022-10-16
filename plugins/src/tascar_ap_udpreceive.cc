#include "netaudio.h"
#include <tascar/audioplugin.h>
#include <thread>
#include <udpsocket.h>

class ringbuffer_ooowrite_t {
public:
  ringbuffer_ooowrite_t(size_t frames, size_t channels);
  ~ringbuffer_ooowrite_t();
  void write_data(float* audio, size_t wframes, size_t wchannels,
                  size_t sample_index);
  inline size_t rspace() const
  {
    if(wpos > rpos)
      return wpos - rpos - 1;
    return frames + wpos - rpos - 1;
  };
  inline size_t wspace() const
  {
    if(rpos > wpos)
      return rpos - wpos - 1;
    return frames + rpos - wpos - 1;
  };

private:
  size_t frames = 1;
  size_t channels = 1;
  float* data = NULL;
  size_t wpos = 1;
  size_t rpos = 0;
};

ringbuffer_ooowrite_t::ringbuffer_ooowrite_t(size_t frames, size_t channels)
    : frames(frames), channels(channels)
{
  data = new float[std::max((size_t)1u, frames * channels)];
}

ringbuffer_ooowrite_t::~ringbuffer_ooowrite_t()
{
  delete[] data;
}

void ringbuffer_ooowrite_t::write_data(float* audio, size_t wframes,
                                       size_t wchannels, size_t sample_index)
{
  sample_index = sample_index % frames;
}

/*
  This example implements an audio plugin which is a white noise
  generator.

  Audio plugins inherit from TASCAR::audioplugin_base_t and need to
  implement the method ap_process(), and optionally add_variables().
 */
class udpreceive_t : public TASCAR::audioplugin_base_t {
public:
  udpreceive_t(const TASCAR::audioplugin_cfg_t& cfg);
  void ap_process(std::vector<TASCAR::wave_t>& chunk, const TASCAR::pos_t& pos,
                  const TASCAR::zyx_euler_t& o, const TASCAR::transport_t& tp);
  virtual ~udpreceive_t();
  void configure();
  void release();

private:
  void recsrv();
  std::thread recthread;
  std::atomic_bool runsession = true;
  udpsocket_t socket;
  int32_t port;
  netaudio_info_t info;
  char* cbuffer;
  size_t cbufferlen;
  size_t cyclecounter;
  netaudio_err_t errcode;
  float* audiobuffer;
  size_t sample_index;
};

// default constructor, called while loading the plugin
udpreceive_t::udpreceive_t(const TASCAR::audioplugin_cfg_t& cfg)
    : audioplugin_base_t(cfg), port(0), cbuffer(NULL), cbufferlen(0),
      cyclecounter(0), audiobuffer(NULL), sample_index(0)
{
  // register variable for XML access:
  GET_ATTRIBUTE(port, "", "destination port number");
  socket.set_timeout_usec(10000);
  socket.bind(port, true);
}

void udpreceive_t::configure()
{
  TASCAR::audioplugin_base_t::configure();
  info = new_netaudio_info(f_sample, pcm16bit, n_channels, n_fragment);
  cbufferlen = std::max(get_buffer_length_header(), get_buffer_length(info));
  cbuffer = new char[cbufferlen];
  cyclecounter = 0;
  audiobuffer = new float[n_channels * n_fragment];
  runsession = true;
  recthread = std::thread(&udpreceive_t::recsrv, this);
}

void udpreceive_t::recsrv()
{
  char buffer[BUFSIZE];
  float* audio = NULL;
  size_t audio_numelem = 0;
  endpoint_t sender_endpoint;
  netaudio_info_t info;
  bool has_info = false;
  uint32_t sample_index = 0;
  while(runsession) {
    ssize_t n = socket.recvfrom(buffer, BUFSIZE, sender_endpoint);
    if(n > 0) {
      netaudio_err_t err;
      size_t recbytes = decode_header(info, buffer, n, err);
      if(err == netaudio_success) {
        DEBUG(info.id);
        DEBUG(info.samplefmt);
        DEBUG(info.srate);
        DEBUG(info.channels);
        DEBUG(info.fragsize);
        if(audio_numelem != info.channels * info.fragsize) {
          audio_numelem = info.channels * info.fragsize;
          if(audio)
            delete[] audio;
          audio = new float[audio_numelem];
        }
        has_info = true;
      } else {
        if(has_info) {
          recbytes = decode_audio(info, audio, audio_numelem, sample_index,
                                  buffer, n, err);
          if(err == netaudio_success) {
            DEBUG(sample_index);
          }
        }
      }
    }
  }
  if(audio)
    delete[] audio;
}

void udpreceive_t::release()
{
  runsession = false;
  recthread.join();
  delete[] cbuffer;
  delete[] audiobuffer;
  TASCAR::audioplugin_base_t::release();
}

udpreceive_t::~udpreceive_t() {}

void udpreceive_t::ap_process(std::vector<TASCAR::wave_t>& chunk,
                              const TASCAR::pos_t& pos,
                              const TASCAR::zyx_euler_t& o,
                              const TASCAR::transport_t& tp)
{
}

// create the plugin interface:
REGISTER_AUDIOPLUGIN(udpreceive_t);

/*
 * Local Variables:
 * mode: c++
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * compile-command: "make -C .."
 * End:
 */
