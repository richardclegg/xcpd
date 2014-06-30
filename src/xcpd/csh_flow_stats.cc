
#include "csh_flow_stats.h"
#include <rofl/common/utils/c_logger.h>
#include <rofl/common/cerror.h>


morpheus::csh_flow_stats::csh_flow_stats(morpheus * parent, rofl::cofctl * const src, rofl::cofmsg_flow_stats_request * const msg):chandlersession_base(parent, msg->get_xid()) {
    ROFL_DEBUG("%s called\n",__PRETTY_FUNCTION__);
	process_flow_stats_request(src, msg);
    outstanding_replies= 0;
    replies= std::vector< rofl::cofflow_stats_reply> ();
    reply_map= std::vector<cflowentry>();
    reply_xid= std::vector<uint32_t>();
}

bool morpheus::csh_flow_stats::process_flow_stats_request ( rofl::cofctl * const src, rofl::cofmsg_flow_stats_request * const msg ) {
	if(msg->get_version() != OFP10_VERSION) throw rofl::eBadVersion();
    // Get a list of flowmods we match
	std::vector <cflowentry> entries= m_parent->get_fet()->
        get_translated_matches(msg->get_flow_stats().get_match(),false);
    // Check if there are any, if not send a blank reply
    if (entries.size() == 0) {
        m_parent->send_flow_stats_reply( src, m_request_xid, std::vector< rofl::cofflow_stats_reply > (), false );
        m_completed = true;
        return m_completed;
    }
    // Send a request for every match we find
    for(std::vector<cflowentry>::iterator it = entries.begin(); it != entries.end(); ++it) {
        rofl::cofflow_stats_request req(OFP10_VERSION, it->match, 
            msg->get_flow_stats().get_table_id(), it->get_out_port());
         // TODO is get_stats_flags correct ??
        uint32_t newxid = m_parent->send_flow_stats_request(m_parent->get_dpt(), 
            msg->get_stats_flags(), req ); 
        cflowentry c= cflowentry(*it);
        reply_map.push_back(*it);
        reply_xid.push_back(newxid);
        if( ! m_parent->associate_xid( m_request_xid, newxid, this ) ) {
            ROFL_ERR("Problem associating dpt xid in %s\n",__PRETTY_FUNCTION__);
        }
        outstanding_replies++;
    }

	m_completed = false;
	return m_completed;
}

bool morpheus::csh_flow_stats::process_flow_stats_reply ( rofl::cofdpt * const src, rofl::cofmsg_flow_stats_reply * const msg ) {
	assert(!m_completed);
	//const cportvlan_mapper & mapper = m_parent->get_mapper();
	if(msg->get_version() != OFP10_VERSION) throw rofl::eBadVersion();
    std::vector <cofflow_stats_reply> &newreps= msg->get_flow_stats();
    if (newreps.size() == 0) {
        ROFL_ERR("Got unexpected zero size flow reply in %s\n",__FUNCTION__);
    } else {
        rofl::cofflow_stats_reply oldreply= rofl::cofflow_stats_reply(newreps[0]);
        unsigned int i;
        for (i= 0; i < reply_xid.size(); i++) {
            if (msg->get_xid() == reply_xid[i]) {
                break;
            }
        }
        if (i == reply_xid.size()) {
            ROFL_ERR("Could not find xid %d in %s\n",msg->get_xid(),__FUNCTION__);
        } else {
            rofl::cflowentry e= m_parent->get_fet()->untrans_flow_entry(reply_map[i]);
            rofl::cofflow_stats_reply newreply= cofflow_stats_reply (
                oldreply.get_version(), 
                oldreply.get_table_id(), 
                oldreply.get_duration_sec(), 
                oldreply.get_duration_nsec(), 
                oldreply.get_priority(), 
                oldreply.get_idle_timeout(), 
                oldreply.get_hard_timeout(), 
                oldreply.get_cookie(), 
                oldreply.get_packet_count(), 
                oldreply.get_byte_count(), 
                e.match,
                e.actions);
            replies.push_back(newreply);
        }
    }
    
    
    outstanding_replies--;
    if (outstanding_replies > 0) {
        m_completed= false;
    } else {
        m_parent->send_flow_stats_reply( m_parent->get_ctl(), 
            m_request_xid, replies, false);
        m_completed = true;
	}
    
    return m_completed;
}

bool morpheus::csh_flow_stats::handle_error (rofl::cofdpt *src, rofl::cofmsg_error *msg) {
	ROFL_DEBUG("Warning: %s has received an error message: %s\n", __PRETTY_FUNCTION__, msg->c_str());
	m_completed = true;
	return m_completed;
}

morpheus::csh_flow_stats::~csh_flow_stats() { std::cout << __FUNCTION__ << " called." << std::endl; }

std::string morpheus::csh_flow_stats::asString() const { std::stringstream ss; ss << "csh_flow_stats {request_xid=" << m_request_xid << "}"; return ss.str(); }
