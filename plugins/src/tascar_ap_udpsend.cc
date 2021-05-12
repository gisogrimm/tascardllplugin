#include "netaudio.h"
#include <tascar/audioplugin.h>
#include <udpsocket.h>

/*
  This example implements an audio plugin which is a white noise
  generator.

  Audio plugins inherit from TASCAR::audioplugin_base_t and need to
  implement the method ap_process(), and optionally add_variables().
 */
class udpsend_t : public TASCAR::audioplugin_base_t {
public:
  udpsend_t(const TASCAR::audioplugin_cfg_t& cfg);
  void ap_process(std::vector<TASCAR::wave_t>& chunk, const TASCAR::pos_t& pos,
                  const TASCAR::zyx_euler_t& o, const TASCAR::transport_t& tp);
  virtual ~udpsend_t();
  void configure();
  void release();

private:
  udpsocket_t socket;
  std::string host;
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
udpsend_t::udpsend_t(const TASCAR::audioplugin_cfg_t& cfg)
    : audioplugin_base_t(cfg), host("localhost"), port(0), cbuffer(NULL),
      cbufferlen(0), cyclecounter(0), audiobuffer(NULL), sample_index(0)
{
  // register variable for XML access:
  GET_ATTRIBUTE(host, "", "destination host");
  GET_ATTRIBUTE(port, "", "destination port number");
  socket.bind(port);
  socket.set_destination(host.c_str());
}

void udpsend_t::configure()
{
  info = new_netaudio_info(f_sample, pcm16bit, n_channels, n_fragment);
  cbufferlen = std::max(get_buffer_length_header(), get_buffer_length(info));
  cbuffer = new char[cbufferlen];
  cyclecounter = 0;
  audiobuffer = new float[n_channels * n_fragment];
}

void udpsend_t::release()
{
  delete[] cbuffer;
  delete[] audiobuffer;
}

udpsend_t::~udpsend_t() {}

void udpsend_t::ap_process(std::vector<TASCAR::wave_t>& chunk,
                           const TASCAR::pos_t& pos,
                           const TASCAR::zyx_euler_t& o,
                           const TASCAR::transport_t& tp)
{
  // implement the algrithm:
  if(!cyclecounter) {
    cyclecounter = std::max(1.0, f_fragment);
    size_t codedbytes(encode_header(info, cbuffer, cbufferlen, errcode));
    socket.send(cbuffer, codedbytes, port);
    // ignore errors for now.
  } else {
    --cyclecounter;
  }
  for(size_t k = 0; k < n_fragment; ++k)
    for(size_t c = 0; c < n_channels; ++c)
      audiobuffer[c + n_channels * k] = chunk[c][k];
  size_t codedbytes(encode_audio(info, audiobuffer, n_fragment * n_channels,
                                 sample_index, cbuffer, cbufferlen, errcode));
  sample_index += n_fragment;
  socket.send(cbuffer, codedbytes, port);
}

// create the plugin interface:
REGISTER_AUDIOPLUGIN(udpsend_t);

/*
 * Local Variables:
 * mode: c++
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * compile-command: "make -C .."
 * End:
 */
