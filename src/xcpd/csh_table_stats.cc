
#include "csh_table_stats.h"
#include <rofl/common/utils/c_logger.h>

morpheus::csh_table_stats::csh_table_stats(morpheus * parent, const rofl::cofctl * const src, const rofl::cofmsg_table_stats_request * const msg):chandlersession_base(parent, msg->get_xid()) {
	ROFL_DEBUG("%s called\n", __PRETTY_FUNCTION__);
	process_table_stats_request(src, msg);
	}

bool morpheus::csh_table_stats::process_table_stats_request ( const rofl::cofctl * const src, const rofl::cofmsg_table_stats_request * const msg ) {
	if(msg->get_version() != OFP10_VERSION) 
		throw rofl::eBadVersion();
	uint32_t newxid = m_parent->send_table_stats_request(m_parent->get_dpt(), msg->get_stats_flags());

	if( ! m_parent->associate_xid( m_request_xid, newxid, this ) ) {
		ROFL_ERR("Problem associating dpt xid in %s");
	}
	m_completed = false;
	return m_completed;
}

bool morpheus::csh_table_stats::process_table_stats_reply ( rofl::cofdpt * const src, rofl::cofmsg_table_stats_reply * const msg ) {
	assert(!m_completed);
	if(msg->get_version() != OFP10_VERSION) throw rofl::eBadVersion();
	m_parent->send_table_stats_reply(m_parent->get_ctl(), m_request_xid, msg->get_table_stats(), false ); // TODO how to deal with "more" flag (last arg)
	m_completed = true;
	return m_completed;
}

bool morpheus::csh_table_stats::handle_error (rofl::cofdpt *src, rofl::cofmsg_error *msg) {
	ROFL_DEBUG("Error message propagated to controller: %s\n", __PRETTY_FUNCTION__, msg->c_str());
	m_parent->send_error_message(m_parent->get_ctl(), 
		m_request_xid, msg->get_err_type(),
	    msg->get_err_code(),
	    msg->get_body().somem(), msg->get_length());
	m_completed = true;
	return m_completed;
}

morpheus::csh_table_stats::~csh_table_stats() {
	ROFL_DEBUG("Destructor called: %s\n", 
		__PRETTY_FUNCTION__);
}

std::string morpheus::csh_table_stats::asString() const { 
	std::stringstream ss; ss << 
		"csh_table_stats {request_xid=" << 
		m_request_xid << "}"; 
	return ss.str(); 
}
