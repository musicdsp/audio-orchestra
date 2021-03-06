/** @file
 * @author Edouard DUPIN 
 * @copyright 2011, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#ifdef ORCHESTRA_BUILD_JAVA

#include <audio/orchestra/Interface.hpp>

namespace audio {
	namespace orchestra {
		namespace api {
			class Android: public audio::orchestra::Api {
				public:
					static ememory::SharedPtr<audio::orchestra::Api> create();
				public:
					Android();
					virtual ~Android();
					const etk::String& getCurrentApi() {
						return audio::orchestra::typeJava;
					}
					uint32_t getDeviceCount();
					audio::orchestra::DeviceInfo getDeviceInfo(uint32_t _device);
					enum audio::orchestra::error closeStream();
					enum audio::orchestra::error startStream();
					enum audio::orchestra::error stopStream();
					enum audio::orchestra::error abortStream();
					// This function is intended for internal use only.	It must be
					// public because it is called by the internal callback handler,
					// which is not a member of RtAudio.	External use of this function
					// will most likely produce highly undesireable results!
					void callbackEvent();
				private:
					int32_t m_uid;
				public:
					int32_t getUId() {
						return m_uid;
					}
				private:
					etk::Vector<audio::orchestra::DeviceInfo> m_devices;
					void saveDeviceInfo();
					bool open(uint32_t _device,
					          audio::orchestra::mode _mode,
					          uint32_t _channels,
					          uint32_t _firstChannel,
					          uint32_t _sampleRate,
					          audio::format _format,
					          uint32_t *_bufferSize,
					          const audio::orchestra::StreamOptions& _options);
				public:
					void playback(int16_t* _dst, int32_t _nbChunk);
					void record(int16_t* _dst, int32_t _nbChunk);
			};
		}
	}
}

#endif
