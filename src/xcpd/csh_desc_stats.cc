
#include "csh_desc_stats.h"
#include <rofl/common/utils/c_logger.h>

morpheus::csh_desc_stats::csh_desc_stats(morpheus * parent, const rofl::cofctl * const src, const rofl::cofmsg_desc_stats_request * const msg):chandlersession_base(parent, msg->get_xid()) {
	std::cout << __PRETTY_FUNCTION__ << " called." << std::endl;
	process_desc_stats_request(src, msg);
	}

bool morpheus::csh_desc_stats::process_desc_stats_request ( const rofl::cofctl * const src, const rofl::cofmsg_desc_stats_request * const msg ) {
	if(msg->get_version() != OFP10_VERSION) throw rofl::eBadVersion();
	uint32_t newxid = m_parent->send_desc_stats_request(m_parent->get_dpt(), msg->get_stats_flags());

	if( ! m_parent->associate_xid( m_request_xid, newxid, this ) ) {
        ROFL_WARN("Problem associating dpt xid in %s\n", __FUNCTION__);
    }
	m_completed = false;
	return m_completed;
}

bool morpheus::csh_desc_stats::process_desc_stats_reply ( rofl::cofdpt * const src, rofl::cofmsg_desc_stats_reply * const msg ) {
	assert(!m_completed);
	if(msg->get_version() != OFP10_VERSION) throw rofl::eBadVersion();
	rofl::cofdesc_stats_reply reply(src->get_version(),"morpheus_mfr_desc","morpheus_hw_desc","morpheus_sw_desc","morpheus_serial_num","morpheus_dp_desc");
	m_parent->send_desc_stats_reply(m_parent->get_ctl(), m_request_xid, reply, false );
	m_completed = true;
	return m_completed;
}

bool morpheus::csh_desc_stats::handle_error (rofl::cofdpt *src, rofl::cofmsg_error *msg) {
	ROFL_DEBUG("Warning: %s has received an error message: %s\n", __PRETTY_FUNCTION__, msg->c_str());
	m_completed = true;
	return m_completed;
}

morpheus::csh_desc_stats::~csh_desc_stats() { 
    ROFL_DEBUG("%s called\n",__FUNCTION__);
}

std::string morpheus::csh_desc_stats::asString() const {
    std::stringstream ss; 
    ss << "csh_desc_stats {request_xid=" << m_request_xid << "}"; 
    return ss.str(); 
}

