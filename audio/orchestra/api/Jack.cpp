/** @file
 * @author Edouard DUPIN 
 * @copyright 2011, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 * @fork from RTAudio
 */

// must run before :          
#if defined(ORCHESTRA_BUILD_JACK)
extern "C" {
	#include <limits.h>
	#include <string.h>
}
#include <audio/orchestra/Interface.hpp>
#include <audio/orchestra/debug.hpp>
#include <ethread/tools.hpp>
#include <audio/orchestra/api/Jack.hpp>

ememory::SharedPtr<audio::orchestra::Api> audio::orchestra::api::Jack::create() {
	return ememory::SharedPtr<audio::orchestra::api::Jack>(ETK_NEW(audio::orchestra::api::Jack));
}


// JACK is a low-latency audio server, originally written for the
// GNU/Linux operating system and now also ported to OS-X. It can
// connect a number of different applications to an audio device, as
// well as allowing them to share audio between themselves.
//
// When using JACK with RtAudio, "devices" refer to JACK clients that
// have ports connected to the server. The JACK server is typically
// started in a terminal as follows:
//
// .jackd -d alsa -d hw:0
//
// or through an interface program such as qjackctl. Many of the
// parameters normally set for a stream are fixed by the JACK server
// and can be specified when the JACK server is started. In
// particular,
//
// jackd -d alsa -d hw:0 -r 44100 -p 512 -n 4
// jackd -r -d alsa -r 48000
//
// specifies a sample rate of 44100 Hz, a buffer size of 512 sample
// frames, and number of buffers = 4. Once the server is running, it
// is not possible to override these values. If the values are not
// specified in the command-line, the JACK server uses default values.
//
// The JACK server does not have to be running when an instance of
// audio::orchestra::Jack is created, though the function getDeviceCount() will
// report 0 devices found until JACK has been started. When no
// devices are available (i.e., the JACK server is not running), a
// stream cannot be opened.

#include <jack/jack.h>
extern "C" {
#include <stdio.h>
}

namespace audio {
	namespace orchestra {
		namespace api {
			class JackPrivate {
				public:
					jack_client_t *client;
					jack_port_t **ports[2];
					etk::String deviceName[2];
					bool xrun[2];
					ethread::Semaphore m_semaphore;
					int32_t drainCounter; // Tracks callback counts when draining
					bool internalDrain; // Indicates if stop is initiated from callback or not.
					
					JackPrivate() :
					  client(0),
					  drainCounter(0),
					  internalDrain(false) {
						ports[0] = 0;
						ports[1] = 0;
						xrun[0] = false;
						xrun[1] = false;
				}
			};
		}
	}
}

audio::orchestra::api::Jack::Jack() :
  m_private(ETK_NEW(audio::orchestra::api::JackPrivate)) {
	// Nothing to do here.
}

audio::orchestra::api::Jack::~Jack() {
	if (m_state != audio::orchestra::state::closed) {
		closeStream();
	}
}

uint32_t audio::orchestra::api::Jack::getDeviceCount() {
	// See if we can become a jack client.
	jack_options_t options = (jack_options_t) (JackNoStartServer); //JackNullOption;
	jack_status_t *status = null;
	jack_client_t *client = jack_client_open("orchestraJackCount", options, status);
	if (client == null) {
		return 0;
	}
	const char **ports;
	etk::String port, previousPort;
	uint32_t nChannels = 0, nDevices = 0;
	ports = jack_get_ports(client, null, null, 0);
	if (ports) {
		// Parse the port names up to the first colon (:).
		size_t iColon = 0;
		do {
			port = (char *) ports[ nChannels ];
			iColon = port.find(":");
			if (iColon != etk::String::npos) {
				port = port.extract(0, iColon + 1);
				if (port != previousPort) {
					nDevices++;
					previousPort = port;
				}
			}
		} while (ports[++nChannels]);
		free(ports);
		ports = null;
	}
	jack_client_close(client);
	return nDevices*2;
}

audio::orchestra::DeviceInfo audio::orchestra::api::Jack::getDeviceInfo(uint32_t _device) {
	audio::orchestra::DeviceInfo info;
	jack_options_t options = (jack_options_t) (JackNoStartServer); //JackNullOption
	jack_status_t *status = null;
	jack_client_t *client = jack_client_open("orchestraJackInfo", options, status);
	if (client == null) {
		ATA_ERROR("Jack server not found or connection error!");
		// TODO : audio::orchestra::error_warning;
		info.clear();
		return info;
	}
	const char **ports;
	etk::String port, previousPort;
	uint32_t nPorts = 0, nDevices = 0;
	ports = jack_get_ports(client, null, null, 0);
	int32_t deviceID = _device/2;
	info.input = _device%2==0?true:false; // note that jack sens are inverted
	if (ports) {
		// Parse the port names up to the first colon (:).
		size_t iColon = 0;
		do {
			port = (char *) ports[nPorts];
			iColon = port.find(":");
			if (iColon != etk::String::npos) {
				port = port.extract(0, iColon);
				if (port != previousPort) {
					if (nDevices == deviceID) {
						info.name = port;
					}
					nDevices++;
					previousPort = port;
				}
			}
		} while (ports[++nPorts]);
		free(ports);
	}
	if (deviceID >= nDevices) {
		jack_client_close(client);
		ATA_ERROR("device ID is invalid!");
		// TODO : audio::orchestra::error_invalidUse;
		return info;
	}
	// Get the current jack server sample rate.
	info.sampleRates.clear();
	info.sampleRates.pushBack(jack_get_sample_rate(client));
	if (info.input == true) {
		ports = jack_get_ports(client, info.name.c_str(), null, JackPortIsOutput);
		if (ports) {
			int32_t iii=0;
			while (ports[iii]) {
				ATA_ERROR(" ploppp='" << ports[iii] << "'");
				info.channels.pushBack(audio::channel_unknow);
				iii++;
			}
			free(ports);
		}
	} else {
		ports = jack_get_ports(client, info.name.c_str(), null, JackPortIsInput);
		if (ports) {
			int32_t iii=0;
			while (ports[iii]) {
				ATA_ERROR(" ploppp='" << ports[iii] << "'");
				info.channels.pushBack(audio::channel_unknow);
				iii++;
			}
			free(ports);
		}
	}
	if (info.channels.size() == 0) {
		jack_client_close(client);
		ATA_ERROR("error determining Jack input/output channels!");
		// TODO : audio::orchestra::error_warning;
		info.clear();
		return info;
	}
	// Jack always uses 32-bit floats.
	info.nativeFormats.pushBack(audio::format_float);
	// Jack doesn't provide default devices so we'll use the first available one.
	if (deviceID == 0) {
		info.isDefault = true;
	}
	jack_client_close(client);
	info.isCorrect = true;
	return info;
}

int32_t audio::orchestra::api::Jack::jackCallbackHandler(jack_nframes_t _nframes, void* _userData) {
	ATA_VERBOSE("Jack callback: [BEGIN] " << uint64_t(_userData));
	audio::orchestra::api::Jack* myClass = reinterpret_cast<audio::orchestra::api::Jack*>(_userData);
	if (myClass->callbackEvent((uint64_t)_nframes) == false) {
		ATA_VERBOSE("Jack callback: [END] 1");
		return 1;
	}
	ATA_VERBOSE("Jack callback: [END] 0");
	return 0;
}


void audio::orchestra::api::Jack::jackShutdown(void* _userData) {
	audio::orchestra::api::Jack* myClass = reinterpret_cast<audio::orchestra::api::Jack*>(_userData);
	// Check current stream state. If stopped, then we'll assume this
	// was called as a result of a call to audio::orchestra::api::Jack::stopStream (the
	// deactivation of a client handle causes this function to be called).
	// If not, we'll assume the Jack server is shutting down or some
	// other problem occurred and we should close the stream.
	if (myClass->isStreamRunning() == false) {
		return;
	}
	ETK_NEW(ethread::Thread, [=](){myClass->closeStream();});
	ATA_ERROR("The Jack server is shutting down this client ... stream stopped and closed!!");
}

int32_t audio::orchestra::api::Jack::jackXrun(void* _userData) {
	audio::orchestra::api::Jack* myClass = reinterpret_cast<audio::orchestra::api::Jack*>(_userData);
	if (myClass->m_private->ports[0]) {
		myClass->m_private->xrun[0] = true;
	}
	if (myClass->m_private->ports[1]) {
		myClass->m_private->xrun[1] = true;
	}
	return 0;
}

bool audio::orchestra::api::Jack::open(uint32_t _device,
                                       audio::orchestra::mode _mode,
                                       uint32_t _channels,
                                       uint32_t _firstChannel,
                                       uint32_t _sampleRate,
                                       audio::format _format,
                                       uint32_t* _bufferSize,
                                       const audio::orchestra::StreamOptions& _options) {
	// Look for jack server and try to become a client (only do once per stream).
	jack_client_t *client = 0;
	if (    _mode == audio::orchestra::mode_output
	     || (    _mode == audio::orchestra::mode_input
	          && m_mode != audio::orchestra::mode_output)) {
		jack_options_t jackoptions = (jack_options_t) (JackNoStartServer); //JackNullOption;
		jack_status_t *status = null;
		if (_options.streamName.size() != 0) {
			client = jack_client_open(_options.streamName.c_str(), jackoptions, status);
		} else {
			client = jack_client_open("orchestraJack", jackoptions, status);
		}
		if (client == 0) {
			ATA_ERROR("Jack server not found or connection error!");
			return false;
		}
	} else {
		// The handle must have been created on an earlier pass.
		client = m_private->client;
	}
	const char **ports;
	etk::String port, previousPort, deviceName;
	uint32_t nPorts = 0, nDevices = 0;
	int32_t deviceID = _device/2;
	bool isInput = _device%2==0?true:false;
	ports = jack_get_ports(client, null, null, 0);
	if (ports) {
		// Parse the port names up to the first colon (:).
		size_t iColon = 0;
		do {
			port = (char *) ports[ nPorts ];
			iColon = port.find(":");
			if (iColon != etk::String::npos) {
				port = port.extract(0, iColon);
				if (port != previousPort) {
					if (nDevices == deviceID) {
						deviceName = port;
					}
					nDevices++;
					previousPort = port;
				}
			}
		} while (ports[++nPorts]);
		free(ports);
	}
	if (_device >= nDevices) {
		ATA_ERROR("device ID is invalid!");
		return false;
	}
	// Count the available ports containing the client name as device
	// channels.	Jack "input ports" equal RtAudio output channels.
	uint32_t nChannels = 0;
	uint64_t flag = JackPortIsInput;
	if (_mode == audio::orchestra::mode_input) {
		flag = JackPortIsOutput;
	}
	ports = jack_get_ports(client, deviceName.c_str(), null, flag);
	if (ports) {
		while (ports[ nChannels ]) {
			nChannels++;
		}
		free(ports);
	}
	// Compare the jack ports for specified client to the requested number of channels.
	if (nChannels < (_channels + _firstChannel)) {
		ATA_ERROR("requested number of channels (" << _channels << ") + offset (" << _firstChannel << ") not found for specified device (" << _device << ":" << deviceName << ").");
		return false;
	}
	// Check the jack server sample rate.
	uint32_t jackRate = jack_get_sample_rate(client);
	if (_sampleRate != jackRate) {
		jack_client_close(client);
		ATA_ERROR("the requested sample rate (" << _sampleRate << ") is different than the JACK server rate (" << jackRate << ").");
		return false;
	}
	m_sampleRate = jackRate;
	// Get the latency of the JACK port.
	ports = jack_get_ports(client, deviceName.c_str(), null, flag);
	if (ports[ _firstChannel ]) {
		// Added by Ge Wang
		jack_latency_callback_mode_t cbmode = (_mode == audio::orchestra::mode_input ? JackCaptureLatency : JackPlaybackLatency);
		// the range (usually the min and max are equal)
		jack_latency_range_t latrange; latrange.min = latrange.max = 0;
		// get the latency range
		jack_port_get_latency_range(jack_port_by_name(client, ports[_firstChannel]), cbmode, &latrange);
		// be optimistic, use the min!
		m_latency[modeToIdTable(_mode)] = latrange.min;
		//m_latency[modeToIdTable(_mode)] = jack_port_get_latency(jack_port_by_name(client, ports[ _firstChannel ]));
	}
	free(ports);
	// The jack server always uses 32-bit floating-point data.
	m_deviceFormat[modeToIdTable(_mode)] = audio::format_float;
	m_userFormat = _format;
	// Jack always uses non-interleaved buffers.
	m_deviceInterleaved[modeToIdTable(_mode)] = false;
	// Jack always provides host byte-ordered data.
	m_doByteSwap[modeToIdTable(_mode)] = false;
	// Get the buffer size.	The buffer size and number of buffers
	// (periods) is set when the jack server is started.
	m_bufferSize = (int) jack_get_buffer_size(client);
	*_bufferSize = m_bufferSize;
	m_nDeviceChannels[modeToIdTable(_mode)] = _channels;
	m_nUserChannels[modeToIdTable(_mode)] = _channels;
	// Set flags for buffer conversion.
	m_doConvertBuffer[modeToIdTable(_mode)] = false;
	if (m_userFormat != m_deviceFormat[modeToIdTable(_mode)]) {
		m_doConvertBuffer[modeToIdTable(_mode)] = true;
		ATA_CRITICAL("Can not update format ==> use RIVER lib for this ...");
	}
	if (    m_deviceInterleaved[modeToIdTable(_mode)] == false
	     && m_nUserChannels[modeToIdTable(_mode)] > 1) {
		ATA_ERROR("Reorder channel for the interleaving properties ...");
		m_doConvertBuffer[modeToIdTable(_mode)] = true;
	}
	// Allocate our JackHandle structure for the stream.
	m_private->client = client;
	m_private->deviceName[modeToIdTable(_mode)] = deviceName;
	// Allocate necessary internal buffers.
	uint64_t bufferBytes;
	bufferBytes = m_nUserChannels[modeToIdTable(_mode)] * *_bufferSize * audio::getFormatBytes(m_deviceFormat[modeToIdTable(_mode)]);
	ATA_VERBOSE("allocate : nbChannel=" << m_nUserChannels[modeToIdTable(_mode)] << " bufferSize=" << *_bufferSize << " format=" << m_deviceFormat[modeToIdTable(_mode)] << "=" << audio::getFormatBytes(m_deviceFormat[modeToIdTable(_mode)]));
	m_userBuffer[modeToIdTable(_mode)].resize(bufferBytes, 0);
	if (m_userBuffer[modeToIdTable(_mode)].size() == 0) {
		ATA_ERROR("error allocating user buffer memory.");
		goto error;
	}
	if (m_doConvertBuffer[modeToIdTable(_mode)]) {
		bool makeBuffer = true;
		if (_mode == audio::orchestra::mode_output) {
			bufferBytes = m_nDeviceChannels[0] * audio::getFormatBytes(m_deviceFormat[0]);
		} else { // _mode == audio::orchestra::mode_input
			bufferBytes = m_nDeviceChannels[1] * audio::getFormatBytes(m_deviceFormat[1]);
			if (m_mode == audio::orchestra::mode_output && m_deviceBuffer) {
				uint64_t bytesOut = m_nDeviceChannels[0] * audio::getFormatBytes(m_deviceFormat[0]);
				if (bufferBytes < bytesOut) {
					makeBuffer = false;
				}
			}
		}
		if (makeBuffer) {
			bufferBytes *= *_bufferSize;
			if (m_deviceBuffer) free(m_deviceBuffer);
			m_deviceBuffer = (char *) calloc(bufferBytes, 1);
			if (m_deviceBuffer == null) {
				ATA_ERROR("error allocating device buffer memory.");
				goto error;
			}
		}
	}
	// Allocate memory for the Jack ports (channels) identifiers.
	m_private->ports[modeToIdTable(_mode)] = (jack_port_t **) malloc (sizeof (jack_port_t *) * _channels);
	if (m_private->ports[modeToIdTable(_mode)] == null)	{
		ATA_ERROR("error allocating port memory.");
		goto error;
	}
	m_device[modeToIdTable(_mode)] = _device;
	m_channelOffset[modeToIdTable(_mode)] = _firstChannel;
	m_state = audio::orchestra::state::stopped;
	if (    m_mode == audio::orchestra::mode_output
	     && _mode == audio::orchestra::mode_input) {
		// We had already set up the stream for output.
		m_mode = audio::orchestra::mode_duplex;
	} else {
		m_mode = _mode;
		jack_set_process_callback(m_private->client, &audio::orchestra::api::Jack::jackCallbackHandler, this);
		jack_set_xrun_callback(m_private->client, &audio::orchestra::api::Jack::jackXrun, this);
		jack_on_shutdown(m_private->client, &audio::orchestra::api::Jack::jackShutdown, this);
	}
	// Register our ports.
	char label[64];
	if (_mode == audio::orchestra::mode_output) {
		for (uint32_t i=0; i<m_nUserChannels[0]; i++) {
			snprintf(label, 64, "outport %d", i);
			m_private->ports[0][i] = jack_port_register(m_private->client,
			                                            (const char *)label,
			                                            JACK_DEFAULT_AUDIO_TYPE,
			                                            JackPortIsOutput,
			                                            0);
		}
	} else {
		for (uint32_t i=0; i<m_nUserChannels[1]; i++) {
			snprintf(label, 64, "inport %d", i);
			m_private->ports[1][i] = jack_port_register(m_private->client,
			                                            (const char *)label,
			                                            JACK_DEFAULT_AUDIO_TYPE,
			                                            JackPortIsInput,
			                                            0);
		}
	}
	// Setup the buffer conversion information structure.	We don't use
	// buffers to do channel offsets, so we override that parameter
	// here.
	if (m_doConvertBuffer[modeToIdTable(_mode)]) {
		setConvertInfo(_mode, 0);
	}
	return true;
error:
	jack_client_close(m_private->client);
	if (m_private->ports[0] != null) {
		free(m_private->ports[0]);
		m_private->ports[0] = null;
	}
	if (m_private->ports[1] != null) {
		free(m_private->ports[1]);
		m_private->ports[1] = null;
	}
	for (int32_t iii=0; iii<2; ++iii) {
		m_userBuffer[iii].clear();
	}
	if (m_deviceBuffer) {
		free(m_deviceBuffer);
		m_deviceBuffer = null;
	}
	return false;
}

enum audio::orchestra::error audio::orchestra::api::Jack::closeStream() {
	if (m_state == audio::orchestra::state::closed) {
		ATA_ERROR("no open stream to close!");
		return audio::orchestra::error_warning;
	}
	if (m_private != null) {
		if (m_state == audio::orchestra::state::running) {
			jack_deactivate(m_private->client);
		}
		jack_client_close(m_private->client);
	}
	if (m_private->ports[0] != null) {
		free(m_private->ports[0]);
		m_private->ports[0] = null;
	}
	if (m_private->ports[1] != null) {
		free(m_private->ports[1]);
		m_private->ports[1] = null;
	}
	for (int32_t i=0; i<2; i++) {
		m_userBuffer[i].clear();
	}
	if (m_deviceBuffer) {
		free(m_deviceBuffer);
		m_deviceBuffer = null;
	}
	m_mode = audio::orchestra::mode_unknow;
	m_state = audio::orchestra::state::closed;
	return audio::orchestra::error_none;
}

enum audio::orchestra::error audio::orchestra::api::Jack::startStream() {
	// TODO : Check return ...
	audio::orchestra::Api::startStream();
	if (verifyStream() != audio::orchestra::error_none) {
		return audio::orchestra::error_fail;
	}
	if (m_state == audio::orchestra::state::running) {
		ATA_ERROR("the stream is already running!");
		return audio::orchestra::error_warning;
	}
	int32_t result = jack_activate(m_private->client);
	if (result) {
		ATA_ERROR("unable to activate JACK client!");
		goto unlock;
	}
	const char **ports;
	// Get the list of available ports.
	if (    m_mode == audio::orchestra::mode_output
	     || m_mode == audio::orchestra::mode_duplex) {
		result = 1;
		ports = jack_get_ports(m_private->client, m_private->deviceName[0].c_str(), null, JackPortIsInput);
		if (ports == null) {
			ATA_ERROR("error determining available JACK input ports!");
			goto unlock;
		}
		// Now make the port connections.	Since RtAudio wasn't designed to
		// allow the user to select particular channels of a device, we'll
		// just open the first "nChannels" ports with offset.
		for (uint32_t i=0; i<m_nUserChannels[0]; i++) {
			result = 1;
			if (ports[ m_channelOffset[0] + i ])
				result = jack_connect(m_private->client, jack_port_name(m_private->ports[0][i]), ports[ m_channelOffset[0] + i ]);
			if (result) {
				free(ports);
				ATA_ERROR("error connecting output ports!");
				goto unlock;
			}
		}
		free(ports);
	}
	if (    m_mode == audio::orchestra::mode_input
	     || m_mode == audio::orchestra::mode_duplex) {
		result = 1;
		ports = jack_get_ports(m_private->client, m_private->deviceName[1].c_str(), null, JackPortIsOutput);
		if (ports == null) {
			ATA_ERROR("error determining available JACK output ports!");
			goto unlock;
		}
		// Now make the port connections. See note above.
		for (uint32_t i=0; i<m_nUserChannels[1]; i++) {
			result = 1;
			if (ports[ m_channelOffset[1] + i ]) {
				result = jack_connect(m_private->client, ports[ m_channelOffset[1] + i ], jack_port_name(m_private->ports[1][i]));
			}
			if (result) {
				free(ports);
				ATA_ERROR("error connecting input ports!");
				goto unlock;
			}
		}
		free(ports);
	}
	m_private->drainCounter = 0;
	m_private->internalDrain = false;
	m_state = audio::orchestra::state::running;
unlock:
	if (result == 0) {
		return audio::orchestra::error_none;
	}
	return audio::orchestra::error_systemError;
}

enum audio::orchestra::error audio::orchestra::api::Jack::stopStream() {
	if (verifyStream() != audio::orchestra::error_none) {
		return audio::orchestra::error_fail;
	}
	if (m_state == audio::orchestra::state::stopped) {
		ATA_ERROR("the stream is already stopped!");
		return audio::orchestra::error_warning;
	}
	if (    m_mode == audio::orchestra::mode_output
	     || m_mode == audio::orchestra::mode_duplex) {
		if (m_private->drainCounter == 0) {
			m_private->drainCounter = 2;
			m_private->m_semaphore.wait();
		}
	}
	jack_deactivate(m_private->client);
	m_state = audio::orchestra::state::stopped;
	return audio::orchestra::error_none;
}

enum audio::orchestra::error audio::orchestra::api::Jack::abortStream() {
	if (verifyStream() != audio::orchestra::error_none) {
		return audio::orchestra::error_fail;
	}
	if (m_state == audio::orchestra::state::stopped) {
		ATA_ERROR("the stream is already stopped!");
		return audio::orchestra::error_warning;
	}
	m_private->drainCounter = 2;
	return stopStream();
}

bool audio::orchestra::api::Jack::callbackEvent(uint64_t _nframes) {
	if (    m_state == audio::orchestra::state::stopped
	     || m_state == audio::orchestra::state::stopping) {
		return true;
	}
	if (m_state == audio::orchestra::state::closed) {
		ATA_ERROR("the stream is closed ... this shouldn't happen!");
		return false;
	}
	if (m_bufferSize != _nframes) {
		ATA_ERROR("the JACK buffer size has changed ... cannot process!");
		return false;
	}
	// Check if we were draining the stream and signal is finished.
	if (m_private->drainCounter > 3) {
		m_state = audio::orchestra::state::stopping;
		if (m_private->internalDrain == true) {
			ETK_NEW(ethread::Thread, [&](){stopStream();}, "Jack_stopStream");
		} else {
			m_private->m_semaphore.post();
		}
		return true;
	}
	// Invoke user callback first, to get fresh output data.
	if (m_private->drainCounter == 0) {
		audio::Time streamTime = getStreamTime();
		etk::Vector<enum audio::orchestra::status> status;
		if (m_mode != audio::orchestra::mode_input && m_private->xrun[0] == true) {
			status.pushBack(audio::orchestra::status::underflow);
			m_private->xrun[0] = false;
		}
		if (m_mode != audio::orchestra::mode_output && m_private->xrun[1] == true) {
			status.pushBack(audio::orchestra::status::overflow);
			m_private->xrun[1] = false;
		}
		int32_t cbReturnValue = m_callback(&m_userBuffer[1][0],
		                                   streamTime,
		                                   &m_userBuffer[0][0],
		                                   streamTime,
		                                   m_bufferSize,
		                                   status);
		if (cbReturnValue == 2) {
			m_state = audio::orchestra::state::stopping;
			m_private->drainCounter = 2;
			ETK_NEW(ethread::Thread, [&](){stopStream();}, "Jack_stopStream2");
			return true;
		}
		else if (cbReturnValue == 1) {
			m_private->drainCounter = 1;
			m_private->internalDrain = true;
		}
	}
	jack_default_audio_sample_t *jackbuffer;
	uint64_t bufferBytes = _nframes * sizeof(jack_default_audio_sample_t);
	if (    m_mode == audio::orchestra::mode_output
	     || m_mode == audio::orchestra::mode_duplex) {
		if (m_private->drainCounter > 1) { // write zeros to the output stream
			for (uint32_t i=0; i<m_nDeviceChannels[0]; i++) {
				jackbuffer = (jack_default_audio_sample_t *) jack_port_get_buffer(m_private->ports[0][i], (jack_nframes_t) _nframes);
				memset(jackbuffer, 0, bufferBytes);
			}
		} else if (m_doConvertBuffer[0]) {
			convertBuffer(m_deviceBuffer, &m_userBuffer[0][0], m_convertInfo[0]);
			for (uint32_t i=0; i<m_nDeviceChannels[0]; i++) {
				jackbuffer = (jack_default_audio_sample_t *) jack_port_get_buffer(m_private->ports[0][i], (jack_nframes_t) _nframes);
				memcpy(jackbuffer, &m_deviceBuffer[i*bufferBytes], bufferBytes);
			}
		} else { // no buffer conversion
			for (uint32_t i=0; i<m_nUserChannels[0]; i++) {
				jackbuffer = (jack_default_audio_sample_t *) jack_port_get_buffer(m_private->ports[0][i], (jack_nframes_t) _nframes);
				memcpy(jackbuffer, &m_userBuffer[0][i*bufferBytes], bufferBytes);
			}
		}
		if (m_private->drainCounter) {
			m_private->drainCounter++;
			goto unlock;
		}
	}
	if (    m_mode == audio::orchestra::mode_input
	     || m_mode == audio::orchestra::mode_duplex) {
		if (m_doConvertBuffer[1]) {
			for (uint32_t i=0; i<m_nDeviceChannels[1]; i++) {
				jackbuffer = (jack_default_audio_sample_t *) jack_port_get_buffer(m_private->ports[1][i], (jack_nframes_t) _nframes);
				memcpy(&m_deviceBuffer[i*bufferBytes], jackbuffer, bufferBytes);
			}
			convertBuffer(&m_userBuffer[1][0], m_deviceBuffer, m_convertInfo[1]);
		} else {
			// no buffer conversion
			for (uint32_t i=0; i<m_nUserChannels[1]; i++) {
				jackbuffer = (jack_default_audio_sample_t *) jack_port_get_buffer(m_private->ports[1][i], (jack_nframes_t) _nframes);
				memcpy(&m_userBuffer[1][i*bufferBytes], jackbuffer, bufferBytes);
			}
		}
	}
unlock:
	audio::orchestra::Api::tickStreamTime();
	return true;
}

#endif

