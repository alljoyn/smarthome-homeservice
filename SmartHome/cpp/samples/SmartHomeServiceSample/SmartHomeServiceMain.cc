/******************************************************************************
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
 ******************************************************************************/

#include <iostream>
#include <stdlib.h>
#include <alljoyn/PasswordManager.h>
#include "SmartHomeServiceMain.h"

const char* APPLICATION_NAME = "SmartHomeService";
const char* DAEMON_BUS_NAME = "quiet@org.alljoyn.SmartHome.ThinClient";
const char* WELL_KNOWN_NAME = "org.alljoyn.SmartHome";

static void SigIntHandler(int sig) {
	s_interrupt = true;
}

/** Register the bus object and connect, report the result to stdout, and return the status code. */
QStatus RegisterBusObject(SmartHomeServiceApi* busObject) {
	QStatus status = s_msgBus->RegisterBusObject(*busObject);

	if (ER_OK == status) {
		std::cout << "RegisterBusObject succeeded." << std::endl;
	} else {
		std::cout << "RegisterBusObject failed (" << QCC_StatusText(status) << ")." << std::endl;
	}

	return status;
 }

/** Connect to the daemon, report the result to stdout, and return the status code. */
QStatus ConnectToDaemon() {
	QStatus status;
	status = s_msgBus->Connect();

	if (ER_OK == status) {
		std::cout << "Daemon connect succeeded." << std::endl;
	} else {
		std::cout << "Failed to connect daemon (" << QCC_StatusText(status) << ")." << std::endl;
	}

	return status;
}

/** Start the message bus, report the result to stdout, and return the status code. */
QStatus StartMessageBus(void) {
	QStatus status = s_msgBus->Start();

	if (ER_OK == status) {
		std::cout << "BusAttachment started." << std::endl;
	} else {
		std::cout << "Start of BusAttachment failed (" << QCC_StatusText(status) << ")." << std::endl;
	}

	return status;
}

/** Create the session, report the result to stdout, and return the status code. */
QStatus BindSession(TransportMask mask) {
	SessionOpts opts(SessionOpts::TRAFFIC_MESSAGES, true, SessionOpts::PROXIMITY_ANY, mask);
	SessionPort sp = SERVICE_PORT;
	
	QStatus status = s_msgBus->BindSessionPort(sp, opts, s_busListener);
	

	if (ER_OK == status) {
		std::cout << "BindSessionPort succeeded." << std::endl;
	} else {
		std::cout << "BindSessionPort failed (" << QCC_StatusText(status) << ")." << std::endl;
	}

	return status;
}

/** Advertise the service name, report the result to stdout, and return the status code. */
QStatus AdvertiseName(TransportMask mask) {
	QStatus status = ER_BUS_ESTABLISH_FAILED;

	const uint32_t flags = DBUS_NAME_FLAG_REPLACE_EXISTING | DBUS_NAME_FLAG_DO_NOT_QUEUE;
	status = s_msgBus->RequestName(WELL_KNOWN_NAME, flags);

	if (ER_OK == status) {
		if (s_msgBus->IsConnected()) {
			status = s_msgBus->AdvertiseName(WELL_KNOWN_NAME, mask);
			std::cout << "AdvertiseName " << WELL_KNOWN_NAME << " =" << status << std::endl;
		}
	}
	return status;
}

void WaitForSigInt(void) {
	while (s_interrupt == false) {
#ifdef _WIN32
		Sleep(100);
#else
		usleep(100 * 1000);
#endif
	}
}

static void shutdown()
{
	s_msgBus->CancelAdvertiseName(WELL_KNOWN_NAME, TRANSPORT_ANY);
	s_msgBus->UnregisterBusListener(s_busListener);
	s_msgBus->UnbindSessionPort(s_busListener.getSessionPort());

	SmartHomeServiceApi::DestroyInstance();

	delete s_msgBus;
	s_msgBus = NULL;
}

int main(int argc, char* argv[])
{
	QStatus status = ER_OK;

	std::cout << "AllJoyn Library version: " << ajn::GetVersion() << std::endl;
	std::cout << "AllJoyn Library build info: " << ajn::GetBuildInfo() << std::endl;

	/* Install SIGINT handler so Ctrl + C deallocates memory properly */
	signal(SIGINT, SigIntHandler);

	/* set Daemon password only for bundled app */
#ifdef QCC_USING_BD
	PasswordManager::SetCredentials("ALLJOYN_PIN_KEYX", "1234");
#endif

	/* Create message bus */
	s_msgBus = new BusAttachment(APPLICATION_NAME, true);

	if (!s_msgBus) {
		status = ER_OUT_OF_MEMORY;
		return status;
	}
	
	if (ER_OK == status) {
		status = StartMessageBus();
	}

	if (ER_OK == status) {
		status = ConnectToDaemon();
	}

	if (ER_OK == status) {
		s_msgBus->RegisterBusListener(s_busListener);
	}

	SmartHomeServiceApi::Init(*s_msgBus);
	if (!SmartHomeServiceApi::getInstance()) {
		shutdown();
		return EXIT_FAILURE;
	}

  	status = RegisterBusObject(SmartHomeServiceApi::getInstance());

	const TransportMask SERVICE_TRANSPORT_TYPE = TRANSPORT_ANY;
	if (ER_OK == status) {
		status = BindSession(SERVICE_TRANSPORT_TYPE);
	}

	if (ER_OK == status) {
		status = AdvertiseName(SERVICE_TRANSPORT_TYPE);
	}

	/* Perform the service asynchronously until the user signals for an exit. */
	if (ER_OK == status) {
		WaitForSigInt();
	}
	shutdown();

	return SERVICE_EXIT_OK;
}
