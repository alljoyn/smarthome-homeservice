/*
 *    Copyright (c) 2014, AllSeen Alliance. All rights reserved.
 *
 *    Permission to use, copy, modify, and/or distribute this software for any
 *    purpose with or without fee is hereby granted, provided that the above
 *    copyright notice and this permission notice appear in all copies.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
package org.alljoyn.smarthome.centralizedmanagement.test.user;

import org.alljoyn.bus.BusAttachment;
import org.alljoyn.bus.BusListener;
import org.alljoyn.bus.BusObject;
import org.alljoyn.bus.Mutable;
import org.alljoyn.bus.ProxyBusObject;
import org.alljoyn.bus.SessionListener;
import org.alljoyn.bus.SessionOpts;
import org.alljoyn.bus.Status;
import org.alljoyn.bus.Variant;
import org.alljoyn.bus.alljoyn.DaemonInit;
import org.alljoyn.bus.annotation.BusMethod;
import org.alljoyn.bus.annotation.BusSignal;
import org.alljoyn.bus.annotation.BusSignalHandler;
import org.alljoyn.smarthome.centralizedmanagement.client.ArgsDesc;
import org.alljoyn.smarthome.centralizedmanagement.client.HeartBeat;
import org.alljoyn.smarthome.centralizedmanagement.client.ObjectInfo;
import org.alljoyn.smarthome.centralizedmanagement.client.ReturnValueSignalHandler;
import org.alljoyn.smarthome.centralizedmanagement.client.SmartHomeClient;

import android.annotation.SuppressLint;
import android.app.Application;
import android.content.Intent;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.util.Log;

/*
 * The ApplianceApplication class handles all operations between AllJoyn device and SmartHome gateway.
 */

public class UserApplication extends Application {
	public static final String TAG = UserApplication.class.getSimpleName();
	public static final String ACTION_CONNECT_SMARTHOME_OK = "ACTION_CONNECT_SMARTHOME_OK";
	public static final String ACTION_CONNECT_SMARTHOME_NOT_OK = "ACTION_CONNECT_SMARTHOME_NOT_OK";
	
	static {
		try {
			System.loadLibrary("alljoyn_java");
		} catch (Exception e) {
			System.out.println("can't load library alljoyn_java");
		}
	}
	
	// The information of user
	private static final String MY_SERVICE_NAME = "org.alljoyn.smarthome.test.user";
	private static final String MY_DEVICE_ID = "UserDeviceID";
	
	// The information of appliance
	private static final String APP_DEVICE_ID = "ApplianceDeviceID";
	private static final String APP_OBJECT_PATH = "/ApplianceBusObject";
		
	// The information of SmartHome gateway
	private static final String GATEWAY_SERVICE_NAME = "org.alljoyn.SmartHome";
	private static final short GATEWAY_CONTACT_PORT = 24;
	private static final String GATEWAY_INTERFACE_OBJECT_PATH = "/org/alljoyn/SmartHome/CentralizedManagement";

	// The necessary settings of user using AllJoyn
	private BusAttachment bus;
	private ProxyBusObject smartHomeGateway;
	private SmartHomeClient smartHomeClient;
	private ReturnValueHandler returnValueHandler;
	private BusHandler busHandler;
	private UserObject userObject;
	private Mutable.IntegerValue sessionId = new Mutable.IntegerValue(0);
	private boolean isJoined = false;
	
	@Override
	public void onCreate() {
		super.onCreate();
		
		HandlerThread busThread = new HandlerThread("BusHandler");
		busThread.start();
		busHandler = new BusHandler();
		userObject = new UserObject();
		returnValueHandler = new ReturnValueHandler();
	}
	
	public void doConnect() {
		busHandler.sendEmptyMessage(BusHandler.CONNECT);
	}
	
	public void methodCall(int status) {
		Message msg = new Message();
		msg.what = BusHandler.METHOD_CALL;
		msg.obj = status;
		
		busHandler.sendMessage(msg);
	}
	
	public void doDisconnect() {
		busHandler.sendEmptyMessage(BusHandler.DISCONNECT);
	}
	
	@SuppressLint("HandlerLeak")
	class BusHandler extends Handler {
		private static final int CONNECT = 1;
		private static final int JOIN_SESSION = 2;
		private static final int METHOD_CALL = 3;
		private static final int DISCONNECT = 4;
		
		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case CONNECT:
				Log.i(TAG, "Connect to SmartHome gateway");
				
				DaemonInit.PrepareDaemon(getApplicationContext());
				bus = new BusAttachment(getPackageName(), BusAttachment.RemoteMessage.Receive);
				bus.registerBusObject(userObject, "/null");
				bus.registerBusListener(new BusListener() {

					@Override
					public void foundAdvertisedName(String name, short transport, String namePrefix) {
						// find and connect to the SmartHome gateway

						if ( GATEWAY_SERVICE_NAME.equals(name) ) {
							Log.i(TAG, "The found advertised name is " + name);
							
							Message msg = obtainMessage(JOIN_SESSION);
							msg.obj = name;
							sendMessage(msg);
						}
					}
					
				});
				
				Status conStatus = bus.connect();
				Log.i(TAG, conStatus.name());
				
				if ( Status.OK == conStatus ) {
					Log.d(TAG, "succeed to connect.");
				} else {
					Log.d(TAG, "fail to connect.");
				}
				
				Status  regStatus = bus.registerSignalHandlers(returnValueHandler);
				Log.i(TAG, regStatus.name());
				
				Status findStatus = bus.findAdvertisedName(GATEWAY_SERVICE_NAME);
				Log.i(TAG, findStatus.name());
				
				if ( Status.OK == findStatus ) {
					Log.d(TAG, "succeed to find advertised name of SmartHome gateway.");
				} else {
					Log.d(TAG, "fail to find advertised name of SmartHome gateway.");
				}
				break;
				
			case JOIN_SESSION:
				SessionOpts sessionOpts = new SessionOpts();
				sessionOpts.traffic = SessionOpts.TRAFFIC_MESSAGES;
		        sessionOpts.isMultipoint = true;
		        sessionOpts.proximity = SessionOpts.PROXIMITY_ANY;
		        sessionOpts.transports = SessionOpts.TRANSPORT_ANY;
		        short contactPort = GATEWAY_CONTACT_PORT; 
		        
		        Status joinStatus = bus.joinSession((String) msg.obj, contactPort, sessionId, sessionOpts, new SessionListener() {
		        	
					@Override
					public void sessionLost(int arg0, int arg1) {
						Log.i(TAG, "Session is lost.");
						
						if ( isJoined == true ) {
							isJoined = false;
						}
						
						Intent intent = new Intent(ACTION_CONNECT_SMARTHOME_NOT_OK);
						sendBroadcast(intent);
					}
		        	
		        });
		        
		        Log.i(TAG, joinStatus.toString());
		        
		        if ( Status.OK == joinStatus ) {
					Log.d(TAG, "succeed to join session.");
					
					if ( isJoined == false ) {
						isJoined = true;
					}
					
					smartHomeGateway = bus.getProxyBusObject(GATEWAY_SERVICE_NAME,
															GATEWAY_INTERFACE_OBJECT_PATH, 
															sessionId.value,  
															new Class<?>[] {SmartHomeClient.class});
					smartHomeClient = smartHomeGateway.getInterface(SmartHomeClient.class);
		    
					// Implement the registration of device
					ArgsDesc argsDesc1 = new ArgsDesc();
					argsDesc1.methodName = "";
					argsDesc1.inputValueType = "";
					argsDesc1.returnValueType = "";
					argsDesc1.argsName = "";
					ArgsDesc[] argsDescs = { argsDesc1 };
					
					ObjectInfo objectInfo = new ObjectInfo();
					objectInfo.interfaceName = "";
					objectInfo.objectPath = "/null";
					objectInfo.argsDescs = new Variant(argsDescs, "a(ssss)");
					ObjectInfo[] objectInfos = { objectInfo };
					
					smartHomeClient.ApplianceRegistration(MY_SERVICE_NAME, bus.getUniqueName(), MY_DEVICE_ID, objectInfos);
					
					// Let SmartHome gateway monitor the status of device and this is necessary.
					HeartBeat heartBeat = new HeartBeat(smartHomeClient, MY_DEVICE_ID);
					heartBeat.deviceHeartBeat();
					
					Intent intent = new Intent(ACTION_CONNECT_SMARTHOME_OK);
					sendBroadcast(intent);
		        } else {
					Log.d(TAG, "fail to join session.");
				}
				break;

			case METHOD_CALL:
				int status =  (int) msg.obj;
				Variant args = new Variant(status, "u");
				smartHomeClient.Execute(true, APP_DEVICE_ID, APP_OBJECT_PATH, "Lamp", args);
				
				break;

			case DISCONNECT:
				Log.i(TAG, "Disconnect to SmartHome gateway");
				
				smartHomeClient.ApplianceUnRegistration(MY_DEVICE_ID);
				
				if ( isJoined ) {
					bus.leaveSession(sessionId.value);
				}
				bus.disconnect();
			
				break;

			default:
				break;
			}
		}
		
	}
	
	class UserObject implements BusObject, SmartHomeClient {

		@Override
		@BusMethod(signature = "sssa(sov)")
		public void ApplianceRegistration(String wellKnownName, String uniqueName, String deviceId, ObjectInfo[] objectInfo) {
		}

		@Override
		@BusMethod(signature = "s")
		public void ApplianceUnRegistration(String deviceId) {		
		}

		@Override
		@BusMethod(signature = "s")
		public void DeviceHeartBeat(String deviceId) {
		}

		@Override
		@BusMethod(signature = "bsosv")
		public void Execute(boolean isReturn, String deviceId, String objectPath, String methodName, Variant arguments) {			
		}

		@Override
		@BusSignal(signature = "ssv")
		public void ReturnValue(String methodName, String status, Variant value) {
		}
		
	}
	
	class ReturnValueHandler extends ReturnValueSignalHandler {

		@Override
		@BusSignalHandler(iface = "org.alljoyn.SmartHome.CentralizedManagement", signal = "ReturnValue")
		public void ReturnValue(String methodName, String status, Variant value) {
			
			Log.i(TAG, "Return Value");
			
			// Receive the return value here.
		}

	}
}
