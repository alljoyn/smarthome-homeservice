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