/** @file
 * @author Edouard DUPIN 
 * @copyright 2011, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

extern "C" {
	#include <jni.h>
	#include <pthread.h>
}
#include <ethread/Mutex.hpp>
#include <audio/orchestra/debug.hpp>
#include <audio/orchestra/error.hpp>
#include <audio/orchestra/api/AndroidNativeInterface.hpp>
#include <audio/orchestra/api/Android.hpp>
/* include auto generated file */
#include <org_musicdsp_orchestra_OrchestraConstants.h>
#include <jvm-basics/jvm-basics.hpp>
#include <ememory/memory.hpp>
#include <ejson/ejson.hpp>

class AndroidOrchestraContext {
	public:
		// get a resources from the java environement : 
		JNIEnv* m_JavaVirtualMachinePointer; //!< the JVM
		jclass m_javaClassOrchestra; //!< main activity class (android ...)
		jclass m_javaClassOrchestraCallback;
		jobject m_javaObjectOrchestraCallback;
		jmethodID m_javaMethodOrchestraActivityAudioGetDeviceCount;
		jmethodID m_javaMethodOrchestraActivityAudioGetDeviceProperty;
		jmethodID m_javaMethodOrchestraActivityAudioOpenDeviceInput;
		jmethodID m_javaMethodOrchestraActivityAudioOpenDeviceOutput;
		jmethodID m_javaMethodOrchestraActivityAudioCloseDevice;
		jmethodID m_javaMethodOrchestraActivityAudioStart;
		jmethodID m_javaMethodOrchestraActivityAudioStop;
		jclass m_javaDefaultClassString; //!< default string class
	private:
		bool safeInitMethodID(jmethodID& _mid, jclass& _cls, const char* _name, const char* _sign) {
			_mid = m_JavaVirtualMachinePointer->GetMethodID(_cls, _name, _sign);
			if(_mid == null) {
				ATA_ERROR("C->java : Can't find the method " << _name);
				/* remove access on the virtual machine : */
				m_JavaVirtualMachinePointer = null;
				return false;
			}
			return true;
		}
		bool java_attach_current_thread(int *_rstatus) {
			ATA_DEBUG("C->java : call java");
			if (jvm_basics::getJavaVM() == null) {
				ATA_ERROR("C->java : JVM not initialised");
				m_JavaVirtualMachinePointer = null;
				return false;
			}
			*_rstatus = jvm_basics::getJavaVM()->GetEnv((void **) &m_JavaVirtualMachinePointer, JNI_VERSION_1_6);
			if (*_rstatus == JNI_EDETACHED) {
				JavaVMAttachArgs lJavaVMAttachArgs;
				lJavaVMAttachArgs.version = JNI_VERSION_1_6;
				lJavaVMAttachArgs.name = "EwolNativeThread";
				lJavaVMAttachArgs.group = null; 
				int status = jvm_basics::getJavaVM()->AttachCurrentThread(&m_JavaVirtualMachinePointer, &lJavaVMAttachArgs);
				jvm_basics::checkExceptionJavaVM(m_JavaVirtualMachinePointer);
				if (status != JNI_OK) {
					ATA_ERROR("C->java : AttachCurrentThread failed : " << status);
					m_JavaVirtualMachinePointer = null;
					return false;
				}
			}
			return true;
		}
		void java_detach_current_thread(int _status) {
			if(_status == JNI_EDETACHED) {
				jvm_basics::getJavaVM()->DetachCurrentThread();
				m_JavaVirtualMachinePointer = null;
			}
		}
		
	public:
		AndroidOrchestraContext(JNIEnv* _env, jclass _classBase, jobject _objCallback) :
		  m_JavaVirtualMachinePointer(null),
		  m_javaClassOrchestra(0),
		  m_javaClassOrchestraCallback(0),
		  m_javaObjectOrchestraCallback(0),
		  
		  m_javaMethodOrchestraActivityAudioGetDeviceCount(0),
		  m_javaMethodOrchestraActivityAudioGetDeviceProperty(0),
		  m_javaMethodOrchestraActivityAudioOpenDeviceInput(0),
		  m_javaMethodOrchestraActivityAudioOpenDeviceOutput(0),
		  m_javaMethodOrchestraActivityAudioCloseDevice(0),
		  m_javaMethodOrchestraActivityAudioStart(0),
		  m_javaMethodOrchestraActivityAudioStop(0),
		  m_javaDefaultClassString(0) {
			ATA_DEBUG("*******************************************");
			ATA_DEBUG("** set JVM Pointer (orchestra)           **");
			ATA_DEBUG("*******************************************");
			m_JavaVirtualMachinePointer = _env;
			// get default needed all time elements : 
			if (m_JavaVirtualMachinePointer == null) {
				ATA_ERROR("C->java: NULLPTR jvm interface");
				return;
			}
			ATA_DEBUG("C->java: try load org/musicdsp/orchestra/OrchestraNative class");
			m_javaClassOrchestra = m_JavaVirtualMachinePointer->FindClass("org/musicdsp/orchestra/OrchestraNative" );
			if (m_javaClassOrchestra == 0) {
				ATA_ERROR("C->java : Can't find org/musicdsp/orchestra/OrchestraNative class");
				// remove access on the virtual machine : 
				m_JavaVirtualMachinePointer = null;
				return;
			}
			/* The object field extends Activity and implement OrchestraCallback */
			m_javaClassOrchestraCallback = m_JavaVirtualMachinePointer->GetObjectClass(_objCallback);
			if(m_javaClassOrchestraCallback == null) {
				ATA_ERROR("C->java : Can't find org/musicdsp/orchestra/OrchestraManagerCallback class");
				// remove access on the virtual machine : 
				m_JavaVirtualMachinePointer = null;
				return;
			}
			bool functionCallbackIsMissing = false;
			bool ret= false;
			
			ret = safeInitMethodID(m_javaMethodOrchestraActivityAudioGetDeviceCount,
			                       m_javaClassOrchestraCallback,
			                       "getDeviceCount",
			                       "()I");
			if (ret == false) {
				jvm_basics::checkExceptionJavaVM(_env);
				ATA_ERROR("system can not start without function : getDeviceCount");
				functionCallbackIsMissing = true;
			}
			
			ret = safeInitMethodID(m_javaMethodOrchestraActivityAudioGetDeviceProperty,
			                       m_javaClassOrchestraCallback,
			                       "getDeviceProperty",
			                       "(I)Ljava/lang/String;");
			if (ret == false) {
				jvm_basics::checkExceptionJavaVM(_env);
				ATA_ERROR("system can not start without function : getDeviceProperty");
				functionCallbackIsMissing = true;
			}
			ret = safeInitMethodID(m_javaMethodOrchestraActivityAudioOpenDeviceInput,
			                       m_javaClassOrchestraCallback,
			                       "openDeviceInput",
			                       "(IIII)I");
			if (ret == false) {
				jvm_basics::checkExceptionJavaVM(_env);
				ATA_ERROR("system can not start without function : openDeviceInput");
				functionCallbackIsMissing = true;
			}
			ret = safeInitMethodID(m_javaMethodOrchestraActivityAudioOpenDeviceOutput,
			                       m_javaClassOrchestraCallback,
			                       "openDeviceOutput",
			                       "(IIII)I");
			if (ret == false) {
				jvm_basics::checkExceptionJavaVM(_env);
				ATA_ERROR("system can not start without function : openDeviceOutput");
				functionCallbackIsMissing = true;
			}
			ret = safeInitMethodID(m_javaMethodOrchestraActivityAudioCloseDevice,
			                       m_javaClassOrchestraCallback,
			                       "closeDevice",
			                       "(I)Z");
			if (ret == false) {
				jvm_basics::checkExceptionJavaVM(_env);
				ATA_ERROR("system can not start without function : closeDevice");
				functionCallbackIsMissing = true;
			}
			ret = safeInitMethodID(m_javaMethodOrchestraActivityAudioStart,
			                       m_javaClassOrchestraCallback,
			                       "start",
			                       "(I)Z");
			if (ret == false) {
				jvm_basics::checkExceptionJavaVM(_env);
				ATA_ERROR("system can not start without function : start");
				functionCallbackIsMissing = true;
			}
			ret = safeInitMethodID(m_javaMethodOrchestraActivityAudioStop,
			                       m_javaClassOrchestraCallback,
			                       "stop",
			                       "(I)Z");
			if (ret == false) {
				jvm_basics::checkExceptionJavaVM(_env);
				ATA_ERROR("system can not start without function : stop");
				functionCallbackIsMissing = true;
			}
			
			
			m_javaObjectOrchestraCallback = _env->NewGlobalRef(_objCallback);
			if (m_javaObjectOrchestraCallback == null) {
				functionCallbackIsMissing = true;
			}
			
			m_javaDefaultClassString = m_JavaVirtualMachinePointer->FindClass("java/lang/String" );
			if (m_javaDefaultClassString == 0) {
				ATA_ERROR("C->java : Can't find java/lang/String" );
				// remove access on the virtual machine : 
				m_JavaVirtualMachinePointer = null;
				functionCallbackIsMissing = true;
			}
			if (functionCallbackIsMissing == true) {
				ATA_CRITICAL(" mission one function ==> system can not work withut it...");
			}
		}
		
		~AndroidOrchestraContext() {
			// TODO ...
		}
		
		void unInit(JNIEnv* _env) {
			_env->DeleteGlobalRef(m_javaObjectOrchestraCallback);
			m_javaObjectOrchestraCallback = null;
		}
		
		uint32_t getDeviceCount() {
			// Request the clipBoard :
			ATA_WARNING("C->java : audio get device count");
			int status;
			if(!java_attach_current_thread(&status)) {
				return 0;
			}
			ATA_DEBUG("Call CallIntMethod ...");
			//Call java ...
			jint ret = m_JavaVirtualMachinePointer->CallIntMethod(m_javaObjectOrchestraCallback, m_javaMethodOrchestraActivityAudioGetDeviceCount);
			// manage execption : 
			jvm_basics::checkExceptionJavaVM(m_JavaVirtualMachinePointer);
			java_detach_current_thread(status);
			ATA_WARNING(" find " << (uint32_t)ret << " IO");
			return (uint32_t)ret;
		}
		audio::orchestra::DeviceInfo getDeviceInfo(uint32_t _idDevice) {
			audio::orchestra::DeviceInfo info;
			// Request the clipBoard :
			ATA_WARNING("C->java : audio get device info " << _idDevice);
			int status;
			if(!java_attach_current_thread(&status)) {
				return info;
			}
			//Call java ...
			jstring returnString = (jstring) m_JavaVirtualMachinePointer->CallObjectMethod(m_javaObjectOrchestraCallback, m_javaMethodOrchestraActivityAudioGetDeviceProperty, _idDevice);
			const char *js = m_JavaVirtualMachinePointer->GetStringUTFChars(returnString, null);
			etk::String retString(js);
			m_JavaVirtualMachinePointer->ReleaseStringUTFChars(returnString, js);
			//m_JavaVirtualMachinePointer->DeleteLocalRef(returnString);
			// manage execption : 
			jvm_basics::checkExceptionJavaVM(m_JavaVirtualMachinePointer);
			java_detach_current_thread(status);
			ATA_WARNING("get device information : " << retString);
			ejson::Document doc;
			if (doc.parse(retString) == false) {
				
				return info;
			}
			info.name = doc["name"].toString().get("no-name");
			if (doc["type"].toString().get("output") == "output") {
				info.input = false;
			} else {
				info.input = true;
			}
			ejson::Array list = doc["sample-rate"].toArray();
			if (list.exist() == true) {
				for (auto it : list) {
					info.sampleRates.pushBack(int32_t(it.toNumber().get(48000)));
				}
			}
			list = doc["channels"].toArray();
			if (list.exist() == true) {
				for (auto it : list) {
					info.channels.pushBack(audio::getChannelFromString(it.toString().get("???")));
				}
			}
			list = doc["format"].toArray();
			if (list.exist() == true) {
				for (auto it : list) {
					info.nativeFormats.pushBack(audio::getFormatFromString(it.toString().get("???")));
				}
			}
			info.isDefault = doc["default"].toBoolean().get(false);
			info.isCorrect = true;
			return info;
		}
	private:
		etk::Vector<ememory::WeakPtr<audio::orchestra::api::Android> > m_instanceList; // list of connected handle ...
		//AndroidAudioCallback m_audioCallBack;
		//void* m_audioCallBackUserData;
	public:
		int32_t open(uint32_t _idDevice,
		             audio::orchestra::mode _mode,
		             uint32_t _channels,
		             uint32_t _firstChannel,
		             uint32_t _sampleRate,
		             audio::format _format,
		             uint32_t *_bufferSize,
		             const audio::orchestra::StreamOptions& _options,
		             ememory::SharedPtr<audio::orchestra::api::Android> _instance) {
			ATA_DEBUG("C->java : audio open device");
			int status;
			if(!java_attach_current_thread(&status)) {
				return -1;
			}
			//Call java ...
			jint ret = false;
			if (_mode == audio::orchestra::mode_output) {
				ret = m_JavaVirtualMachinePointer->CallIntMethod(m_javaObjectOrchestraCallback, m_javaMethodOrchestraActivityAudioOpenDeviceOutput, _idDevice, _sampleRate, _channels, /*_format*/ 1);
			} else {
				ret = m_JavaVirtualMachinePointer->CallIntMethod(m_javaObjectOrchestraCallback, m_javaMethodOrchestraActivityAudioOpenDeviceInput, _idDevice, _sampleRate, _channels, /*_format*/ 1);
			}
			// manage execption : 
			jvm_basics::checkExceptionJavaVM(m_JavaVirtualMachinePointer);
			java_detach_current_thread(status);
			if (int32_t(ret) >= 0) {
				m_instanceList.pushBack(_instance);
				return int32_t(ret);
			}
			return -1;
		}
	public:
		enum audio::orchestra::error closeStream(int32_t _id) {
			ATA_DEBUG("C->java : audio close device");
			int status;
			if(!java_attach_current_thread(&status)) {
				return audio::orchestra::error_fail;
			}
			//Call java ...
			jboolean ret = m_JavaVirtualMachinePointer->CallBooleanMethod(m_javaObjectOrchestraCallback, m_javaMethodOrchestraActivityAudioCloseDevice, _id);
			// manage execption : 
			jvm_basics::checkExceptionJavaVM(m_JavaVirtualMachinePointer);
			java_detach_current_thread(status);
			if (bool(ret) == false) {
				return audio::orchestra::error_fail;
			}
			return audio::orchestra::error_none;
		}
		
		enum audio::orchestra::error startStream(int32_t _id) {
			ATA_DEBUG("C->java : audio start device");
			int status;
			if(!java_attach_current_thread(&status)) {
				return audio::orchestra::error_fail;
			}
			//Call java ...
			jboolean ret = m_JavaVirtualMachinePointer->CallBooleanMethod(m_javaObjectOrchestraCallback, m_javaMethodOrchestraActivityAudioStart, _id);
			// manage execption : 
			jvm_basics::checkExceptionJavaVM(m_JavaVirtualMachinePointer);
			java_detach_current_thread(status);
			if (bool(ret) == false) {
				return audio::orchestra::error_fail;
			}
			return audio::orchestra::error_none;
		}
		enum audio::orchestra::error stopStream(int32_t _id) {
			
			ATA_DEBUG("C->java : audio close device");
			int status;
			if(!java_attach_current_thread(&status)) {
				return audio::orchestra::error_fail;
			}
			//Call java ...
			jboolean ret = m_JavaVirtualMachinePointer->CallBooleanMethod(m_javaObjectOrchestraCallback, m_javaMethodOrchestraActivityAudioStop, _id);
			// manage execption : 
			jvm_basics::checkExceptionJavaVM(m_JavaVirtualMachinePointer);
			java_detach_current_thread(status);
			if (bool(ret) == false) {
				return audio::orchestra::error_fail;
			}
			return audio::orchestra::error_none;
		}
		enum audio::orchestra::error abortStream(int32_t _id) {
			return audio::orchestra::error_fail;
		}
		
		void playback(int32_t _id, int16_t* _dst, int32_t _nbChunk) {
			auto it = m_instanceList.begin();
			while (it != m_instanceList.end()) {
				auto elem = it->lock();
				if (elem == null) {
					it = m_instanceList.erase(it);
					continue;
				}
				if (elem->getUId() == _id) {
					elem->playback(_dst, _nbChunk);
				}
				++it;
			}
		}
		void record(int32_t _id, int16_t* _dst, int32_t _nbChunk) {
			auto it = m_instanceList.begin();
			while (it != m_instanceList.end()) {
				auto elem = it->lock();
				if (elem == null) {
					it = m_instanceList.erase(it);
					continue;
				}
				if (elem->getUId() == _id) {
					elem->record(_dst, _nbChunk);
				}
				++it;
			}
		}
};

static ememory::SharedPtr<AndroidOrchestraContext> s_localContext;
static int32_t s_nbContextRequested(0);


uint32_t audio::orchestra::api::android::getDeviceCount() {
	if (s_localContext == null) {
		ATA_ERROR("Have no Orchertra API instanciate in JAVA ...");
		return 0;
	}
	return s_localContext->getDeviceCount();
}

audio::orchestra::DeviceInfo audio::orchestra::api::android::getDeviceInfo(uint32_t _device) {
	if (s_localContext == null) {
		return audio::orchestra::DeviceInfo();
	}
	return s_localContext->getDeviceInfo(_device);
}

int32_t audio::orchestra::api::android::open(uint32_t _device,
                                             audio::orchestra::mode _mode,
                                             uint32_t _channels,
                                             uint32_t _firstChannel,
                                             uint32_t _sampleRate,
                                             audio::format _format,
                                             uint32_t *_bufferSize,
                                             const audio::orchestra::StreamOptions& _options,
                                             ememory::SharedPtr<audio::orchestra::api::Android> _instance) {
	if (s_localContext == null) {
		return -1;
	}
	return s_localContext->open(_device, _mode, _channels, _firstChannel, _sampleRate, _format, _bufferSize, _options, _instance);
}

enum audio::orchestra::error audio::orchestra::api::android::closeStream(int32_t _id) {
	if (s_localContext == null) {
		return audio::orchestra::error_fail;
	}
	return s_localContext->closeStream(_id);
}

enum audio::orchestra::error audio::orchestra::api::android::startStream(int32_t _id) {
	if (s_localContext == null) {
		return audio::orchestra::error_fail;
	}
	return s_localContext->startStream(_id);
}

enum audio::orchestra::error audio::orchestra::api::android::stopStream(int32_t _id) {
	if (s_localContext == null) {
		return audio::orchestra::error_fail;
	}
	return s_localContext->stopStream(_id);
}

enum audio::orchestra::error audio::orchestra::api::android::abortStream(int32_t _id) {
	if (s_localContext == null) {
		return audio::orchestra::error_fail;
	}
	return s_localContext->abortStream(_id);
}

extern "C" {
	void Java_org_musicdsp_orchestra_OrchestraNative_NNsetJavaManager(JNIEnv* _env,
	                                                                  jclass _classBase,
	                                                                  jobject _objCallback) {
		ethread::UniqueLock lock(jvm_basics::getMutexJavaVM());
		ATA_INFO("*******************************************");
		ATA_INFO("** Creating Orchestra context            **");
		ATA_INFO("*******************************************");
		if (s_localContext != null) {
			s_nbContextRequested++;
		}
		s_localContext = ememory::makeShared<AndroidOrchestraContext>(_env, _classBase, _objCallback);
		if (s_localContext == null) {
			ATA_ERROR("Can not allocate the orchestra main context instance");
			return;
		}
		s_nbContextRequested++;
	}
	
	void Java_org_musicdsp_orchestra_OrchestraNative_NNsetJavaManagerRemove(JNIEnv* _env, jclass _cls) {
		ethread::UniqueLock lock(jvm_basics::getMutexJavaVM());
		ATA_INFO("*******************************************");
		ATA_INFO("** remove Orchestra Pointer              **");
		ATA_INFO("*******************************************");
		if (s_nbContextRequested == 0) {
			ATA_ERROR("Request remove orchestra interface from Android, but no more interface availlable");
			return;
		}
		s_nbContextRequested--;
		if (s_nbContextRequested == 0) {
			s_localContext.reset();
		}
	}
	void Java_org_musicdsp_orchestra_OrchestraNative_NNPlayback(JNIEnv* _env,
	                                                            void* _reserved,
	                                                            jint _id,
	                                                            jshortArray _location,
	                                                            jint _nbChunk) {
		ethread::UniqueLock lock(jvm_basics::getMutexJavaVM());
		if (s_localContext == null) {
			ATA_ERROR("Call audio with no more Low level interface");
			return;
		}
		// get the short* pointer from the Java array
		jboolean isCopy;
		jshort* dst = _env->GetShortArrayElements(_location, &isCopy);
		if (dst != null) {
			//ATA_INFO("Need audioData " << int32_t(_nbChunk));
			s_localContext->playback(int32_t(_id), static_cast<short*>(dst), int32_t(_nbChunk));
		}
		// TODO : Understand why it did not work corectly ...
		//if (isCopy == JNI_TRUE) {
		// release the short* pointer
		_env->ReleaseShortArrayElements(_location, dst, 0);
		//}
	}
	void Java_org_musicdsp_orchestra_OrchestraNative_NNRecord(JNIEnv* _env,
	                                                          void* _reserved,
	                                                          jint _id,
	                                                          jshortArray _location,
	                                                          jint _nbChunk) {
		ethread::UniqueLock lock(jvm_basics::getMutexJavaVM());
		if (s_localContext == null) {
			ATA_ERROR("Call audio with no more Low level interface");
			return;
		}
		// get the short* pointer from the Java array
		jboolean isCopy;
		jshort* dst = _env->GetShortArrayElements(_location, &isCopy);
		if (dst != null) {
			//ATA_INFO("Need audioData " << int32_t(_nbChunk));
			s_localContext->record(int32_t(_id), static_cast<short*>(dst), int32_t(_nbChunk));
		}
		// TODO : Understand why it did not work corectly ...
		//if (isCopy == JNI_TRUE) {
		// release the short* pointer
		_env->ReleaseShortArrayElements(_location, dst, 0);
		//}
	}
}

