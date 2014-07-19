
#include "morpheus.h"
#include "morpheus_nested.h"
#include <rofl/common/utils/c_logger.h>

morpheus::chandlersession_base::chandlersession_base( morpheus * parent, uint32_t xid ):
    m_parent(parent),
    m_completed(false),
    m_lifetime_timer_opaque(0),
    m_request_xid(xid) 
{
}

std::string morpheus::chandlersession_base::asString() const { 
    return "**chandlersession_base**"; 
}

bool morpheus::chandlersession_base::isCompleted() { 
    return m_completed; 
}	// returns true if the session has finished its work and shouldn't be kept alive further

bool morpheus::chandlersession_base::handle_error 
    (rofl::cofdpt *src, rofl::cofmsg_error *msg) { 
    ROFL_ERR("Received error message from dpt %s xid(%d).  No handler\n",
        src->c_str(),msg->get_xid());
    return m_completed;
}

void morpheus::chandlersession_base::handle_timeout (int opaque) { 
    ROFL_ERR("%s called and not overridden by derived class\n");
    throw rofl::eInval();
    assert(false); 
}

void morpheus::chandlersession_base::setLifetimeTimerOpaque( int timer_opaque ) {
     m_lifetime_timer_opaque = timer_opaque; 
}

int morpheus::chandlersession_base::getLifetimeTimerOpaque() const { 
    return m_lifetime_timer_opaque; 
}

morpheus::chandlersession_base::~chandlersession_base() { 
    if (!m_completed) {
        ROFL_ERR("%s: Destructor called on handler when session not complete xid %ld\n",
            __PRETTY_FUNCTION__,m_request_xid);
    }
}

void morpheus::chandlersession_base::push_features(uint32_t new_capabilities, uint32_t new_actions) { m_parent->set_supported_dpe_features( new_capabilities, new_actions ); }

uint32_t morpheus::chandlersession_base::getOriginalXID() const { 
    return m_request_xid; 
} 
