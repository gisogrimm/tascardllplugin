/**
 * @file netaudio.h
 * @brief Definition of the netaudio protocol and API
 */

#ifndef NETAUDIO_H
#define NETAUDIO_H

#include <stdint.h>
#include <stdlib.h>

/**
 * @defgroup netaudioproto network audio protocol
 */

/**
 * @brief List of sample format
 */
enum samplefmt_t : uint16_t { pcm16bit, pcmfloat };

/**
 * @brief List of error codes
 */
enum netaudio_err_t { success, unspecified };

/**
 * @brief Audio information data
 */
typedef struct {
  uint32_t id;        ///< identifier for protocol version
  uint32_t reserved;  ///< reserved for future extensions
  double srate;       ///< sampling rate in Hz
  uint16_t samplefmt; ///< sample format, actually of type samplefmt_t
  uint16_t channels;  ///< number of channels
  uint32_t fragsize;  ///< number of samples per audio chunk
  uint64_t chksum;    ///< check sum generated during compilation
} netaudio_info_t;

static_assert(sizeof(netaudio_info_t) == 32,
              "size of netaudio_info_t is not 32 bytes");

/**
 * @brief Compile an info header from sampling rate, sample format,
 *  channels and fragment size
 *
 * @param[in] srate Sampling rate in Hz
 * @param[in] samplefmt Sample format
 * @param[in] channels Number of channels
 * @param[in] fragsize Number of samples per audio chunk
 * @return Audio information data
 *
 * This function fills all fields of netaudio_info_t. The
 * netaudio_info_t::id member is set to a static value which depends
 * on the protocol version. The netaudio_info_t::chksum member is set
 * to a checksum of all values.
 */
netaudio_info_t new_netaudio_info(double srate, samplefmt_t samplefmt,
                                  uint16_t channels, uint32_t fragsize);

/**
 * @brief Encode an netaudio_info_t into a header package
 *
 * @param[in] info Audio information data
 * @param[out] data Character array where the data is stored
 * @param[in] len Available length
 * @param[out] err Error code in case of failure
 * @return Number of bytes written, or zero in case of failure
 */
size_t encode_header(const netaudio_info_t& info, char* data, size_t len,
                     netaudio_err_t& err);

/**
 * @brief Decode a header package into audio configuration parameters
 *
 * @param[out] srate Sampling rate in Hz
 * @param[out] samplefmt Sample format
 * @param[out] channels Number of channels
 * @param[out] fragsize Number of samples per audio chunk
 * @param[in] data Character array containing data package
 * @param[in] len Number of available Bytes
 * @param[out] err Error code in case of failure
 * @return Number of Bytes used to decode parameter, or zero in case of failure
 */
size_t decode_header(double& srate, samplefmt_t& samplefmt, uint16_t& channels,
                     uint32_t& fragsize, const char* data, size_t len,
                     netaudio_err_t& err);

/**
 * @brief Encode an audio chunk into a character array, given a netaudio info
 * structure
 * @param[in] info Netaudio info structure
 * @param[in] audio Audio samples
 * @param[in] num_elem Total number of audio samples, must be fragsize *
 * channels
 * @param[out] data Character array to store package
 * @param[in] len Size of character array
 * @param[out] err Error code in case of failure
 * @return Number of Bytes used, or zero in case of failure
 */
size_t encode_audio(const netaudio_info_t& info, const float* audio,
                    size_t num_elem, char* data, size_t len,
                    netaudio_err_t& err);

/**
 * @brief Decode an audio package into audio samples
 * @param[in] info Netaudio info structure
 * @param[out] audio buffer to write audio samples into
 * @param[in] num_elem Space of sample buffer in elemens, must be fragsize *
 * channels
 * @param[in] data Character array containing the message
 * @param[in] len Length of character array in Bytes
 * @param[out] err Error code in case of failure
 * @return Number of Bytes used, or zero in case of failure
 */
size_t decode_audio(const netaudio_info_t& info, float* audio, size_t num_elem,
                    const char* data, size_t len, netaudio_err_t& err);

#endif
/*
 * Local Variables:
 * mode: c++
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * compile-command: "make -C .."
 * End:
 */
