
#include "csh_aggregate_stats.h"
#include <rofl/common/utils/c_logger.h>
#include <rofl/common/cerror.h>

morpheus::csh_aggregate_stats::csh_aggregate_stats(morpheus * parent, const rofl::cofctl * const src, rofl::cofmsg_aggr_stats_request * const msg):chandlersession_base(parent, msg->get_xid()) {
    pkt_count= 0;
    byte_count= 0;
    flow_count= 0;
    outstanding_replies= 0;
	process_aggr_stats_request(src, msg);
    
	}

bool morpheus::csh_aggregate_stats::process_aggr_stats_request ( const rofl::cofctl * const src, rofl::cofmsg_aggr_stats_request * const msg ) {
	if(msg->get_version() != OFP10_VERSION) throw rofl::eBadVersion();
	rofl::cofaggr_stats_request aggr_req( msg->get_aggr_stats() );
    // Get a list of flowmods we match
	std::vector <cflowentry> entries= m_parent->get_fet()->
        get_translated_matches(msg->get_aggr_stats().get_match(),false);
    // Check if there are any, if not send a blank reply
    if (entries.size() == 0) {
        m_parent->send_flow_stats_reply(m_parent->get_ctl(), m_request_xid, 
            std::vector<rofl::cofflow_stats_reply> (), false );
        m_completed = true;
        return m_completed;
    }
	// Send a request for every match we find
    for(std::vector<cflowentry>::iterator it = entries.begin(); it != entries.end(); ++it) {
        rofl::cofflow_stats_request req(OFP10_VERSION, it->match, 
            msg->get_aggr_stats().get_table_id(), it->get_out_port());
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

bool morpheus::csh_aggregate_stats::process_aggr_stats_reply ( rofl::cofdpt * const src, rofl::cofmsg_aggr_stats_reply * const msg ) {
	assert(!m_completed);
    ROFL_ERR("Aggregate stats reply received -- should never happen %s in %s\n",
        msg, __PRETTY_FUNCTION__);
    throw rofl::eInval();
	
}

bool morpheus::csh_aggregate_stats::process_flow_stats_reply ( rofl::cofdpt * const src, rofl::cofmsg_flow_stats_reply * const msg )
{
    assert(!m_completed);
	if(msg->get_version() != OFP10_VERSION) throw rofl::eBadVersion();
    std::vector <cofflow_stats_reply> &newreps= msg->get_flow_stats();
    if (newreps.size() == 0) {
        ROFL_ERR("Got unexpected zero size flow reply in %s\n",__FUNCTION__);
    } else {
        if (newreps.size() > 1) {
            ROFL_ERR("Expected reply size 1 in %s\n",__FUNCTION__);
        }
        rofl::cofflow_stats_reply thisreply= rofl::cofflow_stats_reply(newreps[0]);
        pkt_count+= thisreply.get_packet_count();
        byte_count+= thisreply.get_byte_count();
        flow_count++;
    }
    
    
    outstanding_replies--;
    if (outstanding_replies > 0) {
        m_completed= false;
    } else {
        send_reply();
        m_completed = true;
	}
    return m_completed;
}

void morpheus::csh_aggregate_stats::send_reply()
{
    std::vector< rofl::cofflow_stats_reply> replies= 
        std::vector< rofl::cofflow_stats_reply> ();
    //rofl::cofflow_stats_reply newreply= cofflow_stats_reply (
        //oldreply.get_version(), 
        //oldreply.get_table_id(), 
        //oldreply.get_duration_sec(), 
        //oldreply.get_duration_nsec(), 
        //oldreply.get_priority(), 
        //oldreply.get_idle_timeout(), 
        //oldreply.get_hard_timeout(), 
        //oldreply.get_cookie(), 
        //pkt_count, 
        //byte_count, 
        //e.match,
        //e.actions);
    //replies.push_back(newreply);
    m_parent->send_flow_stats_reply( m_parent->get_ctl(), 
            m_request_xid, replies, false);
}

morpheus::csh_aggregate_stats::~csh_aggregate_stats() { 
    if (m_completed == false) {
        send_reply();
        m_completed= true;
    }
}

std::string morpheus::csh_aggregate_stats::asString() const { 
    std::stringstream ss; 
    ss << "csh_aggregate_stats {request_xid=" << m_request_xid << "}";
    return ss.str(); 
}
    

