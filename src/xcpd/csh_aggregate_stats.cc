
#include "csh_aggregate_stats.h"
#include <rofl/common/utils/c_logger.h>
#include <rofl/common/cerror.h>

morpheus::csh_aggregate_stats::csh_aggregate_stats(morpheus * parent, const rofl::cofctl * const src, rofl::cofmsg_aggr_stats_request * const msg):chandlersession_base(parent) {
	std::cout << __PRETTY_FUNCTION__ << " called." << std::endl;
	process_aggr_stats_request(src, msg);
	}

bool morpheus::csh_aggregate_stats::process_aggr_stats_request ( const rofl::cofctl * const src, rofl::cofmsg_aggr_stats_request * const msg ) {
	if(msg->get_version() != OFP10_VERSION) throw rofl::eBadVersion();
	m_request_xid = msg->get_xid();
	uint16_t stats_flags = msg->get_stats_flags();
	rofl::cofaggr_stats_request aggr_req( msg->get_aggr_stats() );
	uint32_t newxid = m_parent->send_aggr_stats_request(m_parent->get_dpt(), stats_flags, aggr_req);
	if( ! m_parent->associate_xid( true, m_request_xid, this ) ) std::cout << "Problem associating ctl xid in " << __FUNCTION__ << std::endl;
	if( ! m_parent->associate_xid( false, newxid, this ) ) std::cout << "Problem associating dpt xid in " << __FUNCTION__ << std::endl;
	m_completed = false;
    // 1) Take get match -- match against untranslated keys in flow dict
    // 2) Send flow_stats_request exact for translated match for all matches
    // 3) Set replies outstanding
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
   	m_completed = true;
    // 1) Receive messages -- decrement replies out_standing.
    // 2) if replies zero m_completed
    // 3) if replies non-zero m not completed
    // 4) If replies zero and no error then send stats
	return m_completed; 
}

// Error handler here?
// if error then set error handled and send error

morpheus::csh_aggregate_stats::~csh_aggregate_stats() { std::cout << __FUNCTION__ << " called." << std::endl; }

std::string morpheus::csh_aggregate_stats::asString() const { std::stringstream ss; ss << "csh_aggregate_stats {request_xid=" << m_request_xid << "}"; return ss.str(); }

