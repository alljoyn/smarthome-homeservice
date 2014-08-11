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

#ifndef _SMART_HOME_SERVICE_API_H
#define _SMART_HOME_SERVICE_API_H

#include <qcc/Log.h>
#include <alljoyn/version.h>
#include "SmartHomeService.h"

namespace ajn {
namespace services {

/**
 *   SmartHomeServiceApi  is wrapper class that encapsulates the SmartHomeService with a Singleton.
 */
class SmartHomeServiceApi : public SmartHomeService {

  public:

    /**
     * GetInstance
     * @return SmartHomeServiceApi created only once.
     */
    static SmartHomeServiceApi* getInstance();

    /**
     * Init with  BusAttachment only once.
     * After the first Init you can call getInstance to receive a proper instance of the class
     * @param bus
     */
    static void Init(ajn::BusAttachment& bus);

    /**
     * Destroy the instance only once after finished
     */
    static void DestroyInstance();

  private:

    /**
     * Constructor
     * @param bus
     */
    SmartHomeServiceApi(ajn::BusAttachment& bus);

    /**
     * Destructor
     */
    virtual ~SmartHomeServiceApi();

    /**
     *  pointer to SmartHomeServiceApi
     */
    static SmartHomeServiceApi* m_instance;

    /**
     * pointer to BusAttachment
     */
    static ajn::BusAttachment* m_BusAttachment;
};

} //namespace
} //namespace
#endif
