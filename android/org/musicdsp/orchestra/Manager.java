/**
 * @author Edouard DUPIN
 *
 * @copyright 2015, Edouard DUPIN, all right reserved
 *
 * @license APACHE v2.0 (see license file)
 */

package org.musicdsp.orchestra;

import android.util.Log;
import java.util.Vector;

//import org.musicdsp.orchestra.Constants;
//import org.musicdsp.orchestra.ManagerCallback;
import org.musicdsp.orchestra.Orchestra;
import org.musicdsp.orchestra.InterfaceOutput;
import org.musicdsp.orchestra.InterfaceInput;

/**
 * @brief Class : 
 *
 */
public class Manager implements ManagerCallback, Constants {
	private Orchestra orchestraHandle;
	private int uid = 0;
	private Vector<InterfaceOutput> outputList;
	private Vector<InterfaceInput> inputList;
	
	public Manager() {
		// set the java evironement in the C sources :
		orchestraHandle = new Orchestra(this);
		outputList = new Vector<InterfaceOutput>();
		inputList = new Vector<InterfaceInput>();
	}
	
	public int getDeviceCount() {
		Log.e("Manager", "Get device List");
		return 1;
	}
	
	public String getDeviceProperty(int idDevice) {
		if (idDevice == 0) {
			return   "{\n"
			       + "	name:'speaker',\n"
			       + "	type:'output',\n"
			       + "	sample-rate:[8000,16000,24000,32000,48000,96000],\n"
			       + "	channels:['front-left','front-right'],\n"
			       + "	format:['int16'],\n"
			       + "	default:true\n"
			       + "}";
		} else {
			return "{}";
		}
	}
	
	public int openDeviceOutput(int idDevice, int freq, int nbChannel, int format) {
		InterfaceOutput iface = new InterfaceOutput(uid, orchestraHandle, idDevice, freq, nbChannel, format);
		uid++;
		Log.e("Manager", "Open device Output: " + idDevice + " with UID=" + (uid-1));
		if (iface != null) {
			outputList.add(iface);
			Log.e("Manager", "Added element count=" + outputList.size());
			return uid-1;
		}
		return -1;
	}
	
	public int openDeviceInput(int idDevice, int freq, int nbChannel, int format) {
		InterfaceInput iface = new InterfaceInput(uid, orchestraHandle, idDevice, freq, nbChannel, format);
		uid++;
		Log.e("Manager", "Open device Input: " + idDevice + " with UID=" + (uid-1));
		if (iface != null) {
			inputList.add(iface);
			return uid-1;
		}
		return -1;
	}
	
	public boolean closeDevice(int uniqueID) {
		Log.e("Manager", "Close device : " + uniqueID);
		if (uniqueID<0) {
			Log.e("Manager", "Can not Close device with UID: " + uniqueID);
			return false;
		}
		// find the Element with his ID:
		if (inputList != null) {
			for (int iii=0; iii<inputList.size(); iii++) {
				if (inputList.get(iii) == null) {
					Log.e("Manager", "Null input element: " + iii);
					continue;
				}
				if (inputList.get(iii).getUId() == uniqueID) {
					// find it ...
					inputList.remove(iii);
					return true;
				}
			}
		}
		if (outputList != null) {
			for (int iii=0; iii<outputList.size(); iii++) {
				if (outputList.get(iii) == null) {
					Log.e("Manager", "Null input element: " + iii);
					continue;
				}
				if (outputList.get(iii).getUId() == uniqueID) {
					// find it ...
					outputList.remove(iii);
					return true;
				}
			}
		}
		Log.e("Manager", "Can not start device with UID: " + uniqueID + " Element does not exist ...");
		return false;
	}
	
	public boolean start(int uniqueID) {
		Log.e("Manager", "start device : " + uniqueID);
		if (uniqueID<0) {
			Log.e("Manager", "Can not start device with UID: " + uniqueID);
			return false;
		}
		// find the Element with his ID:
		if (inputList != null) {
			for (int iii=0; iii<inputList.size(); iii++) {
				if (inputList.get(iii) == null) {
					Log.e("Manager", "Null input element: " + iii);
					continue;
				}
				if (inputList.get(iii).getUId() == uniqueID) {
					// find it ...
					inputList.get(iii).start();
					return true;
				}
			}
		}
		if (outputList != null) {
			for (int iii=0; iii<outputList.size(); iii++) {
				if (outputList.get(iii) == null) {
					Log.e("Manager", "Null input element: " + iii);
					continue;
				}
				if (outputList.get(iii).getUId() == uniqueID) {
					// find it ...
					outputList.get(iii).start();
					return true;
				}
			}
		}
		Log.e("Manager", "Can not start device with UID: " + uniqueID + " Element does not exist ...");
		return false;
	}
	
	public boolean stop(int uniqueID) {
		Log.e("Manager", "stop device : " + uniqueID);
		if (uniqueID<0) {
			Log.e("Manager", "Can not stop device with UID: " + uniqueID);
			return false;
		}
		// find the Element with his ID:
		if (inputList != null) {
			for (int iii=0; iii<inputList.size(); iii++) {
				if (inputList.get(iii) == null) {
					Log.e("Manager", "Null input element: " + iii);
					continue;
				}
				if (inputList.get(iii).getUId() == uniqueID) {
					// find it ...
					inputList.get(iii).AutoStop();
					try {
						inputList.get(iii).join();
					} catch(InterruptedException e) { }
					return true;
				}
			}
		}
		if (outputList != null) {
			for (int iii=0; iii<outputList.size(); iii++) {
				if (outputList.get(iii) == null) {
					Log.e("Manager", "Null input element: " + iii);
					continue;
				}
				if (outputList.get(iii).getUId() == uniqueID) {
					// find it ...
					outputList.get(iii).AutoStop();
					try {
						outputList.get(iii).join();
					} catch(InterruptedException e) { }
					return true;
				}
			}
		}
		Log.e("Manager", "Can not stop device with UID: " + uniqueID + " Element does not exist ...");
		return false;
	}
	public void onCreate() {
		Log.w("Manager", "onCreate ...");
	}
	public void onStart() {
		Log.w("Manager", "onStart ...");
	}
	public void onRestart() {
		Log.w("Manager", "onRestart ...");
	}
	public void onResume() {
		Log.w("Manager", "onResume ...");
	}
	public void onPause() {
		Log.w("Manager", "onPause ...");
	}
	public void onStop() {
		Log.w("Manager", "onStop ...");
	}
	public void onDestroy() {
		Log.w("Manager", "onDestroy ...");
	}
}

