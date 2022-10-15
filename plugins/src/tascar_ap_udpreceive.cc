#include "netaudio.h"
#include <tascar/audioplugin.h>
#include <udpsocket.h>

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
  socket.bind(port);
}

void udpreceive_t::configure()
{
  TASCAR::audioplugin_base_t::configure();
  info = new_netaudio_info(f_sample, pcm16bit, n_channels, n_fragment);
  cbufferlen = std::max(get_buffer_length_header(), get_buffer_length(info));
  cbuffer = new char[cbufferlen];
  cyclecounter = 0;
  audiobuffer = new float[n_channels * n_fragment];
}

void udpreceive_t::release()
{
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
