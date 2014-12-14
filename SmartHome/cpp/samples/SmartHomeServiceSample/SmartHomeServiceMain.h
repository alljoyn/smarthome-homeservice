#ifndef _SMART_HOME_SERVICE_MAIN_H
#define _SMART_HOME_SERVICE_MAIN_H
#include <signal.h>

#include "ThreadService.h"
#include "BusListenerImpl.h"
#include <alljoyn/BusAttachment.h>
#include "SmartHomeServiceApi.h"
#include <alljoyn/about/AboutIconService.h>
#include <alljoyn/about/AboutServiceApi.h>
#include <alljoyn/about/AboutPropertyStoreImpl.h>
#include "OptParser.h"

using namespace std;
using namespace ajn;
using namespace services;

#define SERVICE_EXIT_OK	0
#define SERVICE_OPTION_ERROR  1
#define SERVICE_CONFIG_ERROR  2
static const SessionPort SERVICE_PORT = 24;
static volatile sig_atomic_t s_interrupt = false;

static BusListenerImpl  s_busListener(SERVICE_PORT);
/** Top level message bus object. */
static BusAttachment* s_msgBus = NULL;


static AboutIconService* aboutIconService = NULL;
static AboutPropertyStoreImpl* aboutPropertyStore = NULL;
#endif