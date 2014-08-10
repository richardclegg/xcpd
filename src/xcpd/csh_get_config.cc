
#include "csh_get_config.h"
#include <rofl/common/utils/c_logger.h>

morpheus::csh_get_config::csh_get_config(morpheus * parent, const rofl::cofctl * const src, const rofl::cofmsg_get_config_request * const msg):chandlersession_base(parent, msg->get_xid()) {
	ROFL_DEBUG ("%s: called\n",__PRETTY_FUNCTION__);
	process_config_request(src, msg);
	}

bool morpheus::csh_get_config::process_config_request ( const rofl::cofctl * const src, const rofl::cofmsg_get_config_request * const msg ) {
	if(msg->get_version() != OFP10_VERSION) throw rofl::eBadVersion();
	uint32_t newxid = m_parent->send_get_config_request( m_parent->get_dpt() );
	m_parent->associate_xid( m_request_xid, newxid, this );
	m_completed = false;
	return m_completed;
}

bool morpheus::csh_get_config::process_config_reply ( const rofl::cofdpt * const src, rofl::cofmsg_get_config_reply * const msg ) {
	assert(!m_completed);
	if(msg->get_version() != OFP10_VERSION) throw rofl::eBadVersion();
	m_parent->send_get_config_reply(m_parent->get_ctl(), m_request_xid, msg->get_flags(), msg->get_miss_send_len() );
	m_completed = true;
	return m_completed;
}
bool morpheus::csh_get_config::handle_error (rofl::cofdpt *src, rofl::cofmsg_error *msg) {
	ROFL_DEBUG("Warning: %s has received an error message: %s\n", __PRETTY_FUNCTION__, msg->c_str());
	m_completed = true;
	return m_completed;
}

morpheus::csh_get_config::~csh_get_config() { std::cout << __FUNCTION__ << " called." << std::endl; }

std::string morpheus::csh_get_config::asString() const { std::stringstream ss; ss << "csh_get_config {request_xid=" << m_request_xid << "}"; return ss.str(); }
