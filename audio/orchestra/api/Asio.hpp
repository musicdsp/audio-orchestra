/** @file
 * @author Edouard DUPIN 
 * @copyright 2011, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 * @fork from RTAudio
 */
#pragma once
#ifdef ORCHESTRA_BUILD_ASIO

namespace audio {
	namespace orchestra {
		namespace api {
			class AsioPrivate:
			class Asio: public audio::orchestra::Api {
				public:
					static ememory::SharedPtr<audio::orchestra::Api> create();
				public:
					Asio();
					virtual ~Asio();
					const etk::String& getCurrentApi() {
						return audio::orchestra::typeAsio;
					}
					uint32_t getDeviceCount();
					audio::orchestra::DeviceInfo getDeviceInfo(uint32_t _device);
					enum audio::orchestra::error closeStream();
					enum audio::orchestra::error startStream();
					enum audio::orchestra::error stopStream();
					enum audio::orchestra::error abortStream();
					long getStreamLatency();
					// This function is intended for internal use only.	It must be
					// public because it is called by the internal callback handler,
					// which is not a member of RtAudio.	External use of this function
					// will most likely produce highly undesireable results!
					bool callbackEvent(long _bufferIndex);
				private:
					ememory::SharedPtr<AsioPrivate> m_private;
					etk::Vector<audio::orchestra::DeviceInfo> m_devices;
					void saveDeviceInfo();
					bool m_coInitialized;
					bool open(uint32_t _device,
					          audio::orchestra::mode _mode,
					          uint32_t _channels,
					          uint32_t _firstChannel,
					          uint32_t _sampleRate,
					          audio::format _format,
					          uint32_t *_bufferSize,
					          const audio::orchestra::StreamOptions& _options);
			};
		}
	}
}

#endif
