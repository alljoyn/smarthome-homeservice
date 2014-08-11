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

import org.alljoyn.smarthome.appliance.R;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.widget.ImageView;
import android.widget.Toast;

public class ApplianceActivity extends Activity {
	private ApplianceApplication application;
	private BroadcastReceiver receiver;
	private ImageView image;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.appliance_layout);
		image = (ImageView) findViewById(R.id.lamp);
		application = (ApplianceApplication) getApplication();
		application.doConnect();
		
		receiver = new BroadcastReceiver() {
			
			@Override
			public void onReceive(Context context, Intent intent) {
				if ( ApplianceApplication.ACTION_CONNECT_SMARTHOME_OK.equals(intent.getAction()) ) {
					makeToast("Connection Successfull!");
				}
				
				if ( ApplianceApplication.ACTION_CONNECT_SMARTHOME_NOT_OK.equals(intent.getAction()) ) {
					makeToast("Session Lost!");
				}
				
				if ( ApplianceApplication.ACTION_LAMP_STATUS_CHANGED.equals(intent.getAction()) ) {
					int lampStatus = intent.getIntExtra("lamp_status", 0);
					changeLampStatus(lampStatus);
				}
			}
		};
		
		IntentFilter filter = new IntentFilter();
		filter.addAction(ApplianceApplication.ACTION_CONNECT_SMARTHOME_OK);
		filter.addAction(ApplianceApplication.ACTION_LAMP_STATUS_CHANGED);
		filter.addAction(ApplianceApplication.ACTION_CONNECT_SMARTHOME_NOT_OK);
		registerReceiver(receiver, filter);
	}
	
	@Override
	protected void onDestroy() {
		super.onDestroy();
		unregisterReceiver(receiver);
		application.doDisconncet();
	}

	public void makeToast(String msg) {
		Toast.makeText(this, msg, Toast.LENGTH_SHORT).show();
	}
	
	public void changeLampStatus(int status) {
		if ( status == 1 ) {
			image.setImageResource(R.drawable.lamp_on);
		} else {
			image.setImageResource(R.drawable.lamp_off);
		}
	}

}
