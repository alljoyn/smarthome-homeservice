#ifndef BUSLISTENERIMPL_H_
#define BUSLISTENERIMPL_H_

#include <alljoyn/BusListener.h>
#include <alljoyn/SessionPortListener.h>
#include <alljoyn/BusAttachment.h>
#include "SmartHomeService.h"

extern ajn::SessionId ID;

class BusListenerImpl : public ajn::BusListener, public ajn::SessionPortListener {

  public:
    
    /**
     * Constructor of CommonBusListener
     */
    BusListenerImpl();

    /**
     * Constructor of CommonBusListener
     * @param sessionPort - port of listener
     */
    BusListenerImpl(ajn::SessionPort sessionPort);

    /**
     * Destructor of CommonBusListener
     */
    ~BusListenerImpl();

    /**
     * AcceptSessionJoiner - Receive request to join session and decide whether to accept it or not
     * @param sessionPort - the port of the request
     * @param joiner - the name of the joiner
     * @param opts - the session options
     * @return true/false
     */
    bool AcceptSessionJoiner(ajn::SessionPort sessionPort, const char* joiner, const ajn::SessionOpts& opts);

    /**
     * Set the Value of the SessionPort associated with this SessionPortListener
     * @param sessionPort
     */
    void setSessionPort(ajn::SessionPort sessionPort);

    /**
     * Get the SessionPort of the listener
     * @return
     */
    ajn::SessionPort getSessionPort();

	/**
     * Called by the bus when a session has been successfully joined. The session is now fully up
     * This callback is only used by session creators. Therefore it is only called on listeners
     * passed to BusAttachment::BindSessionPort.
     *
     * @param sessionPort    Session port that was joined.
     * @param id             Id of session.
     * @param joiner         Unique name of the joiner.
     */
	void SessionJoined(ajn::SessionPort sessionPort, ajn::SessionId id, const char* joiner);

  private:

    /**
     * The port used as part of the join session request
     */
    ajn::SessionPort m_SessionPort;
};

#endif /* BUSLISTENERIMPL_H_ */
