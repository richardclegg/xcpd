
#include "csh_port_stats.h"

morpheus::csh_port_stats::csh_port_stats(morpheus * parent, rofl::cofctl * const src, rofl::cofmsg_port_stats_request * const msg):chandlersession_base(parent) {
	std::cout << __PRETTY_FUNCTION__ << " called." << std::endl;
	process_port_stats_request(src, msg);
	}

bool morpheus::csh_port_stats::process_port_stats_request ( rofl::cofctl * const src, rofl::cofmsg_port_stats_request * const msg ) {
	if(msg->get_version() != OFP10_VERSION) throw rofl::eBadVersion();
	//const cportvlan_mapper & mapper = m_parent->get_mapper();
	//m_request_xid = msg->get_xid();
    // TODO write this
	m_completed= true;
	return m_completed;
}

bool morpheus::csh_port_stats::process_port_stats_reply ( rofl::cofdpt * const src, rofl::cofmsg_port_stats_reply * const msg ) {
	assert(!m_completed);
//	const cportvlan_mapper & mapper = m_parent->get_mapper();
	if(msg->get_version() != OFP10_VERSION) throw rofl::eBadVersion();
	rofl::cofdesc_stats_reply reply(src->get_version(),"morpheus_mfr_desc","morpheus_hw_desc","morpheus_sw_desc","morpheus_serial_num","morpheus_dp_desc");
	m_parent->send_desc_stats_reply(m_parent->get_ctl(), m_request_xid, reply, false );
	m_completed = true;
	return m_completed;
}

morpheus::csh_port_stats::~csh_port_stats() { std::cout << __FUNCTION__ << " called." << std::endl; }

std::string morpheus::csh_port_stats::asString() const { std::stringstream ss; ss << "csh_port_stats {request_xid=" << m_request_xid << "}"; return ss.str(); }
