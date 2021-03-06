
#include "csh_set_config.h"
#include <rofl/common/utils/c_logger.h>

morpheus::csh_set_config::csh_set_config(morpheus * parent, const rofl::cofctl * const src, const rofl::cofmsg_set_config * const msg ):chandlersession_base(parent, msg->get_xid()) {
	std::cout << __PRETTY_FUNCTION__ << " called." << std::endl;
	process_set_config(src, msg);
	}

bool morpheus::csh_set_config::process_set_config ( const rofl::cofctl * const src, const rofl::cofmsg_set_config * const msg ) {
	if(msg->get_version() != OFP10_VERSION) throw rofl::eBadVersion();
	m_parent->send_set_config_message(m_parent->get_dpt(), msg->get_flags(), msg->get_miss_send_len());
	m_completed = true;
	return m_completed;
}

bool morpheus::csh_set_config::handle_error (rofl::cofdpt *src, rofl::cofmsg_error *msg) {
	ROFL_DEBUG("Warning: %s has received an error message: %s\n", __PRETTY_FUNCTION__, msg->c_str());
	m_completed = true;
	return m_completed;
}

morpheus::csh_set_config::~csh_set_config() { std::cout << __FUNCTION__ << " called." << std::endl; }	// nothing to do as we didn't register anywhere.

// std::string morpheus::csh_set_config::asString() const { return "csh_set_config {no xid}"; }
std::string morpheus::csh_set_config::asString() const { std::stringstream ss; ss << "csh_set_config {request_xid=" << m_request_xid << "}"; return ss.str(); }
