/** @file
 * @author Edouard DUPIN 
 * @copyright 2011, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 * @fork from RTAudio
 */
#pragma once

#include <etk/Stream.hpp>
#include <audio/orchestra/debug.hpp>
#include <audio/orchestra/type.hpp>
#include <audio/orchestra/state.hpp>
#include <audio/orchestra/mode.hpp>
#include <audio/Time.hpp>
#include <audio/Duration.hpp>
#include <ememory/memory.hpp>
/**
 * @brief Audio library namespace
 */
namespace audio {
	/**
	 * @brief Audio-orchestra library namespace
	 */
	namespace orchestra {
		const etk::Vector<uint32_t>& genericSampleRate();
		/**
		 * @brief airtaudio callback function prototype.
		 * @param _inputBuffer For input (or duplex) streams, this buffer will hold _nbChunk of input audio chunk (null if no data).
		 * @param _timeInput Timestamp of the first buffer sample (recording time).
		 * @param _outputBuffer For output (or duplex) streams, the client should write _nbChunk of audio chunk into this buffer (null if no data).
		 * @param _timeOutput Timestamp of the first buffer sample (playing time).
		 * @param _nbChunk The number of chunk of input or output chunk in the buffer (same size).
		 * @param _status List of error that occured in the laps of time.
		 */
		typedef etk::Function<int32_t (const void* _inputBuffer,
		                               const audio::Time& _timeInput,
		                               void* _outputBuffer,
		                               const audio::Time& _timeOutput,
		                               uint32_t _nbChunk,
		                               const etk::Vector<audio::orchestra::status>& _status)> AirTAudioCallback;
		// A protected structure used for buffer conversion.
		class ConvertInfo {
			public:
				int32_t channels;
				int32_t inJump;
				int32_t outJump;
				enum audio::format inFormat;
				enum audio::format outFormat;
				etk::Vector<int> inOffset;
				etk::Vector<int> outOffset;
		};
	
		class Api : public ememory::EnableSharedFromThis<Api>{
			protected:
				etk::String m_name;
			public:
				Api();
				virtual ~Api();
				void setName(const etk::String& _name) {
					m_name = _name;
				}
				virtual const etk::String& getCurrentApi() = 0;
				virtual uint32_t getDeviceCount() = 0;
				virtual audio::orchestra::DeviceInfo getDeviceInfo(uint32_t _device) = 0;
				// TODO : Check API ...
				virtual	bool getNamedDeviceInfo(const etk::String& _deviceName, audio::orchestra::DeviceInfo& _info) {
					return false;
				}
				virtual uint32_t getDefaultInputDevice();
				virtual uint32_t getDefaultOutputDevice();
				enum audio::orchestra::error openStream(audio::orchestra::StreamParameters* _outputParameters,
				                                        audio::orchestra::StreamParameters* _inputParameters,
				                                        audio::format _format,
				                                        uint32_t _sampleRate,
				                                        uint32_t* _nbChunk,
				                                        audio::orchestra::AirTAudioCallback _callback,
				                                        const audio::orchestra::StreamOptions& _options);
				virtual enum audio::orchestra::error closeStream();
				virtual enum audio::orchestra::error startStream();
				virtual enum audio::orchestra::error stopStream() = 0;
				virtual enum audio::orchestra::error abortStream() = 0;
				long getStreamLatency();
				uint32_t getStreamSampleRate();
				virtual audio::Time getStreamTime();
				bool isStreamOpen() const {
					return m_state != audio::orchestra::state::closed;
				}
				bool isStreamRunning() const {
					return m_state == audio::orchestra::state::running;
				}
				
			protected:
				mutable ethread::Mutex m_mutex;
				audio::orchestra::AirTAudioCallback m_callback;
				uint32_t m_device[2]; // Playback and record, respectively.
				enum audio::orchestra::mode m_mode; // audio::orchestra::mode_output, audio::orchestra::mode_input, or audio::orchestra::mode_duplex.
				enum audio::orchestra::state m_state; // STOPPED, RUNNING, or CLOSED
				etk::Vector<char> m_userBuffer[2]; // Playback and record, respectively.
				char *m_deviceBuffer;
				bool m_doConvertBuffer[2]; // Playback and record, respectively.
				bool m_deviceInterleaved[2]; // Playback and record, respectively.
				bool m_doByteSwap[2]; // Playback and record, respectively.
				uint32_t m_sampleRate; // TODO : Rename frequency
				uint32_t m_bufferSize;
				uint32_t m_nBuffers;
				uint32_t m_nUserChannels[2]; // Playback and record, respectively. // TODO : set only one config (open inout with the same number of channels (limitation)
				uint32_t m_nDeviceChannels[2]; // Playback and record channels, respectively.
				uint32_t m_channelOffset[2]; // Playback and record, respectively.
				uint64_t m_latency[2]; // Playback and record, respectively.
				enum audio::format m_userFormat; // TODO : Remove this ==> use can only open in the Harware format ...
				enum audio::format m_deviceFormat[2]; // Playback and record, respectively.
				audio::orchestra::ConvertInfo m_convertInfo[2];
				
				//audio::Time
				audio::Time m_startTime; //!< start time of the stream (restart at every stop, pause ...)
				audio::Duration m_duration; //!< duration from wich the stream is started
				
				/**
				 * @brief api-specific method that attempts to open a device
				 * with the given parameters. This function MUST be implemented by
				 * all subclasses. If an error is encountered during the probe, a
				 * "warning" message is reported and false is returned. A
				 * successful probe is indicated by a return value of true.
				 */
				virtual bool open(uint32_t _device,
				                  enum audio::orchestra::mode _mode,
				                  uint32_t _channels,
				                  uint32_t _firstChannel,
				                  uint32_t _sampleRate,
				                  enum audio::format _format,
				                  uint32_t *_bufferSize,
				                  const audio::orchestra::StreamOptions& _options);
				virtual bool openName(const etk::String& _deviceName,
				                      audio::orchestra::mode _mode,
				                      uint32_t _channels,
				                      uint32_t _firstChannel,
				                      uint32_t _sampleRate,
				                      audio::format _format,
				                      uint32_t *_bufferSize,
				                                 const audio::orchestra::StreamOptions& _options) { return false; }
				/**
				 * @brief Increment the stream time.
				 */
				void tickStreamTime();
				/**
				 * @brief Clear an RtApiStream structure.
				 */
				void clearStreamInfo();
				/**
				 * @brief Check the current stream status
				 */
				enum audio::orchestra::error verifyStream();
				/**
				 * @brief Protected method used to perform format, channel number, and/or interleaving
				 * conversions between the user and device buffers.
				 */
				void convertBuffer(char *_outBuffer,
				                   char *_inBuffer,
				                   audio::orchestra::ConvertInfo& _info);
				
				/**
				 * @brief Perform byte-swapping on buffers.
				 */
				void byteSwapBuffer(char *_buffer,
				                    uint32_t _samples,
				                    enum audio::format _format);
				/**
				 * @brief Sets up the parameters for buffer conversion.
				 */
				void setConvertInfo(enum audio::orchestra::mode _mode,
				                    uint32_t _firstChannel);
				
			public:
				virtual bool isMasterOf(ememory::SharedPtr<audio::orchestra::Api> _api) {
					return false;
				};
		};
	}
}

