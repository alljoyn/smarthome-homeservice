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

import org.alljoyn.smarthome.test.user.R;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ImageButton;
import android.widget.Toast;

public class UserActivity extends Activity {
	private UserApplication application;
	private BroadcastReceiver receiver;
	private ImageButton image;
	private static int status = 0;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.user_layout);
		image = (ImageButton) findViewById(R.id.on_off);
		application = (UserApplication) getApplication();
		application.doConnect();
		
		receiver = new BroadcastReceiver() {
			@Override
			public void onReceive(Context context, Intent intent) {
				
				if ( UserApplication.ACTION_CONNECT_SMARTHOME_OK.equals(intent.getAction()) ) {
					makeToast("Connection Successfull!");
				}
				
				if ( UserApplication.ACTION_CONNECT_SMARTHOME_NOT_OK.equals(intent.getAction()) ) {
					makeToast("Session Lost!");
				}
				
			}
		};
		
		IntentFilter filter = new IntentFilter();
		filter.addAction(UserApplication.ACTION_CONNECT_SMARTHOME_OK);
		filter.addAction(UserApplication.ACTION_CONNECT_SMARTHOME_NOT_OK);
		registerReceiver(receiver, filter);
		
		image.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				
				if ( status == 0 ) {
					status = 1;
					application.methodCall(status);
					image.setImageResource(R.drawable.on);
				} else {
					status = 0;
					application.methodCall(status);
					image.setImageResource(R.drawable.off);
				}
				
			}
		});
	}
	
	@Override
	protected void onDestroy() {
		super.onDestroy();
		unregisterReceiver(receiver);
		application.doDisconnect();
	}
	
	public void makeToast(String msg) {
		Toast.makeText(this, msg, Toast.LENGTH_SHORT).show();
	}
	
}
