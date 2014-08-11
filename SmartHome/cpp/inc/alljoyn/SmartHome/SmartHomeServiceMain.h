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

#ifndef _SMART_HOME_SERVICE_MAIN_H
#define _SMART_HOME_SERVICE_MAIN_H
#include <signal.h>

#include "ThreadService.h"
#include "BusListenerImpl.h"
#include <alljoyn/BusAttachment.h>
#include "SmartHomeServiceApi.h"


using namespace std;
using namespace ajn;
using namespace services;

#define SERVICE_EXIT_OK	0
static const SessionPort SERVICE_PORT = 24;
static volatile sig_atomic_t s_interrupt = false;

static BusListenerImpl  s_busListener(SERVICE_PORT);
/** Top level message bus object. */
static BusAttachment* s_msgBus = NULL;

#endif
