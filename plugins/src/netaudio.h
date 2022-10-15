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
 * List of sample formats.
 *
 * All PCM sample formats are little-endian except when explicitly stated
 * otherwise.
 */
enum samplefmt_t : uint16_t { pcm16bit = 0, pcmfloat = 1 };

/**
 * List of error codes.
 */
enum netaudio_err_t {
  netaudio_success,
  netaudio_generic_error,
  netaudio_insufficient_memory,
  netaudio_invalid_pointer,
  netaudio_not_a_header,
  netaudio_no_audiochunk,
  netaudio_unsupported_protocol_version,
  netaudio_invalid_buffer_dimensions,
  netaudio_invalid_checksum
};

/**
 * @brief Audio information data
 */
struct netaudio_info_t {
  uint16_t id;           ///< identifier for protocol version
  samplefmt_t samplefmt; ///< sample format, actually of type samplefmt_t
  float srate;           ///< sampling rate in Hz
  uint16_t channels;     ///< number of channels
  uint16_t fragsize;     ///< number of samples per audio chunk
  uint32_t chksum;       ///< check sum generated during compilation, see
                         ///< new_netaudio_info() for details.
};

static_assert(sizeof(netaudio_info_t) == 16,
              "size of netaudio_info_t is not 16 bytes");

/**
 * Compile an info header from sampling rate, sample format, channels
 *  and fragment size
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
 * to a checksum of all values. A CRC32 checksum algorithm is used.
 */
netaudio_info_t new_netaudio_info(double srate, samplefmt_t samplefmt,
                                  uint16_t channels, uint32_t fragsize);

/**
 * Encode a netaudio_info_t into a header package
 *
 * @param[in] info Audio information data.
 * @param[out] data Start of memory area where the data is stored.
 * @param[in] len Size of the data memory n bytes.
 * @param[out] err Set to error code in case of failure, or to netaudio_success.
 * @return Number of bytes written, or zero in case of failure.
 *
 * This function may fail with these error codes:
 * - netaudio_invalid_pointer: the data pointer is not valid
 * - netaudio_insufficient_memory: the size of the memory area is not
 *   large enough to store the encoded header
 */
size_t encode_header(const netaudio_info_t& info, char* data, size_t len,
                     netaudio_err_t& err);

/**
 * Decode a header package into audio configuration parameters
 *
 * @param[out] info Netaudio info structure
 * @param[in] data Start of memory area where the data is stored.
 * @param[in] len Number of available bytes
 * @param[out] err Set to error code in case of failure, or to netaudio_success.
 * @return Number of Bytes used to decode parameter, or zero in case of failure.
 *
 * This function may fail with these error codes:
 * - netaudio_invalid_pointer: the data pointer is not valid
 * - netaudio_insufficient_memory: the size of the memory area is not
 *   large enough to read an encoded header
 * - netaudio_not_a_header: the data is not containing a netaudio info structure
 * - netaudio_unsupported_protocol_version: the protocol id is not
 *   supported
 * - netaudio_invalid_checksum: the checksum is invalid.
 */
size_t decode_header(netaudio_info_t& info, const char* data, size_t len,
                     netaudio_err_t& err);

/**
 * Encode an audio chunk into a character array, given a netaudio info
 * structure
 *
 * @param[in] info Netaudio info structure
 * @param[in] audio Audio samples
 * @param[in] num_elem Total number of audio samples, must be fragsize *
 * channels
 * @param[in] sample_index Index of first sample of buffer
 * @param[out] data Start of memory area where the data is stored.
 * @param[in] len Size of character array
 * @param[out] err Set to error code in case of failure, or to netaudio_success.
 * @return Number of Bytes used, or zero in case of failure
 *
 * This function may fail with the error code
 * - netaudio_insufficient_memory: the size of the memory area is not
 * large enough to store the audio chunk.
 * - netaudio_invalid_pointer: the data or audio pointer is not valid
 * - netaudio_invalid_buffer_dimensions: num_elem is not fragsize * channels
 */
size_t encode_audio(const netaudio_info_t& info, const float* audio,
                    size_t num_elem, uint32_t sample_index, char* data,
                    size_t len, netaudio_err_t& err);

/**
 * Decode an audio package into audio samples.
 *
 * @param[in] info Netaudio info structure
 * @param[out] audio buffer to write audio samples into
 * @param[in] num_elem Space of sample buffer in elemens, must be fragsize *
 * channels
 * @param[out] sample_index Index of first sample of buffer
 * @param[in] data Start of memory area where the data is stored.
 * @param[in] len Length of character array in Bytes
 * @param[out] err Set to error code in case of failure, or to netaudio_success.
 * @return Number of Bytes used, or zero in case of failure
 *
 * This function may fail with these error codes:
 * - netaudio_invalid_pointer: the data or audio pointer is not valid
 * - netaudio_insufficient_memory: the size of the memory area is not
 * large enough to read an encoded audio chunk
 * - netaudio_no_audiochunk: the data is not containing an audio
 * chunk
 * - netaudio_invalid_checksum: the checksum is invalid
 * - netaudio_invalid_buffer_dimensions: num_elem is not fragsize * channels
 */
size_t decode_audio(const netaudio_info_t& info, float* audio, size_t num_elem,
                    uint32_t& sample_index, const char* data, size_t len,
                    netaudio_err_t& err);

/**
 * Return the maximum buffer length required to store one audio chunk.
 *
 * The returned number includes space required for control data.
 *
 * @param[in] info Netaudio info structure
 * @return Number of Bytes needed
 */
size_t get_buffer_length(const netaudio_info_t& info);

/**
 * Return the buffer length required to store a header.
 *
 * @return Number of Bytes needed
 */
size_t get_buffer_length_header();

/**
 * Return the checksum of a netaudio info structure.
 *
 * @param[in] info Netaudio info structure
 * @return CRC32 checksum of all fields except checksum field
 */
uint32_t get_checksum(netaudio_info_t info);

/**
 * CRC32 algorithm used for validation of headers
 *
 * @param[in] data Start of memory area where the data is stored.
 * @param[in] size Number of bytes to be analyzed.
 * @return CRC32 checksum
 */
uint32_t gen_crc32b(const uint8_t* data, uint16_t size);

#endif

/*
 * Local Variables:
 * mode: c++
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * compile-command: "make -C .."
 * End:
 */
