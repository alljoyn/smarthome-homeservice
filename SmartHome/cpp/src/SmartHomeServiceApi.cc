#include "SmartHomeServiceApi.h"

namespace ajn {
namespace services {

SmartHomeServiceApi* SmartHomeServiceApi::m_instance = NULL;
BusAttachment* SmartHomeServiceApi::m_BusAttachment = NULL;

SmartHomeServiceApi* SmartHomeServiceApi::getInstance()
{
	if (!m_BusAttachment){
		return NULL;
	}

	if (!m_instance) {
		m_instance = new SmartHomeServiceApi(*m_BusAttachment);
	}

	return m_instance;
}

void SmartHomeServiceApi::Init(ajn::BusAttachment& bus) 
{
	m_BusAttachment = &bus;
}

void SmartHomeServiceApi::DestroyInstance()
{
	if (m_instance){
		delete m_instance;
		m_instance = NULL;
	}
}

SmartHomeServiceApi::SmartHomeServiceApi(ajn::BusAttachment& bus)
:SmartHomeService(bus)
{
}

SmartHomeServiceApi::~SmartHomeServiceApi()
{
}

} //namespace
} //namespace