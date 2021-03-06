/** @file
 * @author Edouard DUPIN 
 * @copyright 2011, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 * @fork from RTAudio
 */

#if defined(ORCHESTRA_BUILD_DUMMY)
#include <audio/orchestra/api/Dummy.hpp>
#include <audio/orchestra/debug.hpp>

ememory::SharedPtr<audio::orchestra::Api> audio::orchestra::api::Dummy::create() {
	return ememory::SharedPtr<audio::orchestra::api::Dummy>(ETK_NEW(audio::orchestra::api::Dummy));
}


audio::orchestra::api::Dummy::Dummy() {
	ATA_WARNING("This class provides no functionality.");
}

uint32_t audio::orchestra::api::Dummy::getDeviceCount() {
	return 0;
}

audio::orchestra::DeviceInfo audio::orchestra::api::Dummy::getDeviceInfo(uint32_t _device) {
	(void)_device;
	return audio::orchestra::DeviceInfo();
}

enum audio::orchestra::error audio::orchestra::api::Dummy::closeStream() {
	return audio::orchestra::error_none;
}

enum audio::orchestra::error audio::orchestra::api::Dummy::startStream() {
	// TODO : Check return ...
	audio::orchestra::Api::startStream();
	return audio::orchestra::error_none;
}

enum audio::orchestra::error audio::orchestra::api::Dummy::stopStream() {
	return audio::orchestra::error_none;
}

enum audio::orchestra::error audio::orchestra::api::Dummy::abortStream() {
	return audio::orchestra::error_none;
}

bool audio::orchestra::api::Dummy::open(uint32_t _device,
                                        audio::orchestra::mode _mode,
                                        uint32_t _channels,
                                        uint32_t _firstChannel,
                                        uint32_t _sampleRate,
                                        audio::format _format,
                                        uint32_t *_bufferSize,
                                        const audio::orchestra::StreamOptions& _options) {
	return false;
}

#endif

