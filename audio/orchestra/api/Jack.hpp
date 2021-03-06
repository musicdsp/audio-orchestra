/** @file
 * @author Edouard DUPIN 
 * @copyright 2011, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 * @fork from RTAudio
 */
#pragma once
#ifdef ORCHESTRA_BUILD_JACK

#include <jack/jack.h>

namespace audio {
	namespace orchestra {
		namespace api {
			class JackPrivate;
			class Jack: public audio::orchestra::Api {
				public:
					static ememory::SharedPtr<audio::orchestra::Api> create();
				public:
					Jack();
					virtual ~Jack();
					const etk::String& getCurrentApi() {
						return audio::orchestra::typeJack;
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
					bool callbackEvent(uint64_t _nframes);
				private:
					static int32_t jackXrun(void* _userData);
					static void jackShutdown(void* _userData);
					static int32_t jackCallbackHandler(jack_nframes_t _nframes, void* _userData);
				private:
					ememory::SharedPtr<JackPrivate> m_private;
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