// nested classes

#ifndef MORPHEUS_NESTED_H
#define MORPHEUS_NESTED_H 1

#include <sstream>
#include <string>
#include <iterator>
#include <algorithm>
#include <rofl/common/openflow/openflow_rofl_exceptions.h>
#include <rofl/common/openflow/openflow10.h>
#include <rofl/common/cerror.h>
#include <rofl/common/openflow/cofaction.h>
#include "morpheus.h"

/**
 * MASTER TODO:
 * secondary messages (either replies or those forwarded) may return errors - session handlers should have error_msg handlers, and morpheus should register a timer after a call to a session method.  If this timer expires (it's set to longer than a reply message timeout) and the session is completed then only then can it be removed.
 * assert that DPE config supports VLAN tagging and stripping.
 * Somewhere the number of bytes of ethernet frame to send for packet-in is set - this must be adjusted for removal of VLAN tag
 * During a packet-in - what to do with the buffer-id? Because we've sent a packet fragment up in the message, but the switch's buffer has a different version of the packet => should be ok, as switch won't be making changes to stored packet and will forward when asked.
 * in Flow-mod - if the incoming virtual port is actually untagged should we set this in match? Can;t really do that in cofmatch (no access to vid_mask). Should we include a vlan_strip action always anyway?
 * 
 * add morpheus::register_session_timer and morpheus::cancel_session_timer so that session handlers can create and respond to their own timing events
 */

// class morpheus;

void dumpBytes (std::ostream & os, uint8_t * bytes, size_t n_bytes);

class morpheus::chandlersession_base {
	
protected:

    morpheus * m_parent;
    bool m_completed;
    int m_lifetime_timer_opaque;	// this is lifetime timer associated with this session, or -1 if it isn't being used (e.g. a session handling a message which doesn't require an ACK or reply)
    const uint32_t m_request_xid;
    chandlersession_base( morpheus * parent, uint32_t xid );

public:

    virtual std::string asString() const;
    virtual bool isCompleted();
    virtual bool handle_error (rofl::cofdpt *src, rofl::cofmsg_error *msg);	// returns whether this session has completed
    virtual void handle_timeout (int opaque);
    virtual ~chandlersession_base();
    void push_features(uint32_t new_capabilities, uint32_t new_actions);
    virtual void setLifetimeTimerOpaque( int timer_opaque );
    virtual int getLifetimeTimerOpaque() const;
    virtual uint32_t getOriginalXID() const;

};

#endif // UCL_EE_MORPHEUS_NESTED_H
