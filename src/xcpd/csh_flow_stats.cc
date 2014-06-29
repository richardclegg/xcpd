
#include "csh_flow_stats.h"
#include <rofl/common/utils/c_logger.h>
#include <rofl/common/cerror.h>


morpheus::csh_flow_stats::csh_flow_stats(morpheus * parent, rofl::cofctl * const src, rofl::cofmsg_flow_stats_request * const msg):chandlersession_base(parent, msg->get_xid()) {
    ROFL_DEBUG("%s called\n",__PRETTY_FUNCTION__);
	process_flow_stats_request(src, msg);
    outstanding_replies= 0;
    replies= std::vector< rofl::cofflow_stats_reply> ();
	}

bool morpheus::csh_flow_stats::process_flow_stats_request ( rofl::cofctl * const src, rofl::cofmsg_flow_stats_request * const msg ) {
	if(msg->get_version() != OFP10_VERSION) throw rofl::eBadVersion();
    // Get a list of flowmods we match
	std::vector <cflowentry> entries= m_parent->get_fet()->
        get_translated_matches(msg->get_flow_stats().get_match(),false);
    // Check if there are any, if not send a blank reply
    if (entries.size() == 0) {
        m_parent->send_flow_stats_reply( src, m_request_xid, std::vector< rofl::        cofflow_stats_reply > (), false );
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
    std::vector < cofflow_stats_reply > &newreps= msg->get_flow_stats();
    if (newreps.size() == 0) {
        ROFL_ERR("Got unexpected zero size flow reply in %s\n",__FUNCTION__);
    } else {
        //TODO translate match
        rofl::cofflow_stats_reply newreply= rofl::cofflow_stats_reply(newreps[0]);
        
        replies.push_back(newreply);
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
