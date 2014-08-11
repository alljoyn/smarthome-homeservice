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
package org.alljoyn.smarthome.centralizedmanagement.test.appliance;

import org.alljoyn.bus.BusAttachment;
import org.alljoyn.bus.BusException;
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
import org.alljoyn.smarthome.centralizedmanagement.client.ArgsDesc;
import org.alljoyn.smarthome.centralizedmanagement.client.HeartBeat;
import org.alljoyn.smarthome.centralizedmanagement.client.ObjectInfo;
import org.alljoyn.smarthome.centralizedmanagement.client.SmartHomeClient;

import android.annotation.SuppressLint;
import android.app.Application;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.util.Log;

/*
 * The ApplianceApplication class handles all operations between AllJoyn device and SmartHome gateway.
 */

public class ApplianceApplication extends Application {
	public static final String TAG = ApplianceApplication.class.getSimpleName();
	public static final String ACTION_CONNECT_SMARTHOME_OK = "ACTION_CONNECT_SMARTHOME_OK";
	public static final String ACTION_CONNECT_SMARTHOME_NOT_OK = "ACTION_CONNECT_SMARTHOME_NOT_OK";
	public static final String ACTION_LAMP_STATUS_CHANGED = "ACTION_LAMP_STATUS_CHANGED";
	
	static {
		
		try {
			System.loadLibrary("alljoyn_java");
		} catch (Exception e) {
			System.out.println("can't load library alljoyn_java");
		}
		
	}
	
	// The information of appliance
	private static final String MY_SERVICE_NAME = "org.alljoyn.smarthome.test.appliance";
	private static final String MY_DEVICE_ID = "ApplianceDeviceID";
	private static final String MY_OBJECT_PATH = "/ApplianceBusObject";
	private static final String MY_INTERFACE_NAME = "/org/alljoyn/smarthome/test/appliance";
	
	// The information of SmartHome gateway
	private static final String GATEWAY_SERVICE_NAME = "org.alljoyn.SmartHome";
	private static final short GATEWAY_CONTACT_PORT = 24;
	private static final String GATEWAY_INTERFACE_OBJECT_PATH = "/org/alljoyn/SmartHome/CentralizedManagement";
	
	// The necessary settings of appliance using AllJoyn
	private BusAttachment bus;
	private ProxyBusObject smartHomeGateway;
	private SmartHomeClient smartHomeClient;
	private Mutable.IntegerValue sessionId = new Mutable.IntegerValue(0);
	private boolean isJoined = false;
	private BusHandler busHandler;
	
	// Appliance
	private SmartLamp smartLamp;
	
	@Override
	public void onCreate() {
		super.onCreate();
		
		HandlerThread busThread = new HandlerThread("BusHandler");
		busThread.start();
		smartLamp = new SmartLamp();
		busHandler = new BusHandler();
	}
	
	public void doConnect() {
		busHandler.sendEmptyMessage(BusHandler.CONNECT);
	}
	
	public void doDisconncet() {
		busHandler.sendEmptyMessage(BusHandler.DISCONNECT);
	}
	
	@SuppressLint("HandlerLeak")
	class BusHandler extends Handler {
		private static final int CONNECT = 1;
		private static final int JOIN_SESSION = 2;
		private static final int DISCONNECT = 3;
		
		@Override
		public void handleMessage(Message msg) {
			switch (msg.what) {
			case CONNECT:
				Log.i(TAG, "Connect to SmartHome gateway");
				
				DaemonInit.PrepareDaemon(getApplicationContext());
				bus = new BusAttachment(getPackageName(), BusAttachment.RemoteMessage.Receive);
				bus.registerBusObject(smartLamp, MY_OBJECT_PATH);
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
				if ( Status.OK == conStatus ) {
					Log.d(TAG, "succeed to connect.");
				} else {
					Log.d(TAG, "fail to connect.");
				}
				
				int flag = BusAttachment.ALLJOYN_REQUESTNAME_FLAG_DO_NOT_QUEUE | BusAttachment.ALLJOYN_REQUESTNAME_FLAG_REPLACE_EXISTING;
                Status reqStatus = bus.requestName(MY_SERVICE_NAME, flag);
                if ( Status.OK == reqStatus ) {
                	Log.i(TAG, "advertisement succeed");
                	
					bus.advertiseName(MY_SERVICE_NAME, SessionOpts.TRANSPORT_ANY);
				}
				
				Status findStatus = bus.findAdvertisedName(GATEWAY_SERVICE_NAME);
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
					argsDesc1.methodName = "Lamp";
					argsDesc1.inputValueType = "v";
					argsDesc1.returnValueType = "v";
					argsDesc1.argsName = "status";
					ArgsDesc[] argsDescs = { argsDesc1 };
					
					ObjectInfo objectInfo = new ObjectInfo();
					objectInfo.interfaceName = MY_INTERFACE_NAME;
					objectInfo.objectPath = MY_OBJECT_PATH;
					objectInfo.argsDescs = new Variant(argsDescs, "a(ssss)");
					ObjectInfo[] objectInfos = { objectInfo };
		
					// Implement the registration of device
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

	public class SmartLamp implements BusObject, Appliance {
		private int status = 0;
		
		@Override
		@BusMethod(signature = "v", replySignature = "v")
		public Variant Lamp(Variant args) {
			Log.i(TAG, "LAMP");
			
			try {
				status = args.getObject(Integer.class);
				
				Log.i(TAG, Integer.toString(status));
			} catch (BusException e) {
				e.printStackTrace();
			}
			this.setStatus(status);
			
			// Notify the activity to change the status of lamp
			Intent intent = new Intent(ACTION_LAMP_STATUS_CHANGED);
			Bundle extras = new Bundle();
			extras.putInt("lamp_status", status);
			intent.putExtras(extras);
			sendBroadcast(intent);
			
			// Return the status of methodcall
			Variant returnValue = new Variant("AllJoyn SmartHome", "s");
			return returnValue;
		}

		public int isStatus() {
			return status;
		}

		public void setStatus(int status) {
			this.status = status;
		}
	}
}
