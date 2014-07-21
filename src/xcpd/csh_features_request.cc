
#include "csh_features_request.h"
#include <rofl/common/openflow/openflow_rofl_exceptions.h>
#include <rofl/common/openflow/openflow10.h>
#include <rofl/common/cerror.h>
#include <rofl/common/openflow/cofaction.h>
#include <rofl/common/utils/c_logger.h>
#include "control_manager.h"

using namespace xdpd;

morpheus::csh_features_request::csh_features_request(morpheus * parent):chandlersession_base(parent, 0),m_local_request(true) {
	ROFL_DEBUG("%s called.\n",__PRETTY_FUNCTION__);
	uint32_t newxid = m_parent->send_features_request( m_parent->get_dpt() );
	if( ! m_parent->associate_xid( m_request_xid, newxid, this ) ) {
		ROFL_ERR("Problem associating dpt xid in %s.\n",__PRETTY_FUNCTION__);
	}
}

morpheus::csh_features_request::csh_features_request(morpheus * parent, const rofl::cofctl * const src, const rofl::cofmsg_features_request * const msg):chandlersession_base(parent, msg->get_xid()),m_local_request(false) {
	ROFL_DEBUG("%s called.\n",__PRETTY_FUNCTION__);
	process_features_request(src, msg);
	}

bool morpheus::csh_features_request::process_features_request ( const rofl::cofctl * const src, const rofl::cofmsg_features_request * const msg ) {
	if(msg->get_version() != OFP10_VERSION) throw rofl::eBadVersion();
	uint32_t newxid = m_parent->send_features_request( m_parent->get_dpt() );
	if( ! m_parent->associate_xid( m_request_xid, newxid, this ) ) {
		ROFL_ERR("Problem associating dpt xid in %s.\n",__PRETTY_FUNCTION__);
	}
	m_completed = false;
	return m_completed;
}

bool morpheus::csh_features_request::process_features_reply ( const rofl::cofdpt * const src, rofl::cofmsg_features_reply * const msg ) {
	assert(!m_completed);
	if(msg->get_version() != OFP10_VERSION) throw rofl::eBadVersion();
	if(m_local_request) {
		push_features( msg->get_capabilities(), msg->get_actions_bitmap() );
		m_completed = true;
	} else {
		uint64_t dpid = m_parent->get_dpid();	
		uint32_t capabilities = msg->get_capabilities();
		// first check whether we have the ones we need, then rewrite them anyway
		ROFL_DEBUG ("%s Capabilities of DPE reported as: %s.\n",
			__PRETTY_FUNCTION__,capabilities_to_string(capabilities).c_str());
		uint32_t of10_actions_bitmap = msg->get_actions_bitmap();	// 	TODO ofp10 only
		ROFL_DEBUG("%s : supported actions of DPE reported as: %s.\n",
			__PRETTY_FUNCTION__,
			action_mask_to_string(of10_actions_bitmap).c_str());
		push_features( capabilities, of10_actions_bitmap );

		capabilities = m_parent->get_supported_features();
				
		rofl::cofportlist realportlist = msg->get_ports();
		// check whether all the ports we're using are actually supported by the DPE
		
		rofl::cofportlist virtualportlist;

		const cportvlan_mapper & mapper = m_parent->get_mapper();
		for(unsigned portno = 1; portno <= mapper.get_number_virtual_ports(); ++portno) {
			rofl::cofport p(OFP10_VERSION);
			p.set_config(OFP10PC_NO_STP);
			uint32_t feats = OFP10PF_10GB_FD | OFP10PF_FIBER;
			p.set_peer (feats);
			p.set_curr (feats);
			p.set_advertised (feats);
			p.set_supported (feats);
			p.set_state(0);	// link is up and ignoring STP
			p.set_port_no(portno);
			p.set_hwaddr(control_manager::Instance()->
				get_vport(portno-1).get_mac());	
			p.set_name(control_manager::Instance()->
				get_vport(portno-1).get_name());
			virtualportlist.next() = p;
			}
		m_parent->send_features_reply(m_parent->get_ctl(), m_request_xid, dpid, msg->get_n_buffers(), msg->get_n_tables(), capabilities, 0, of10_actions_bitmap, virtualportlist );	// TODO get_action_bitmap is OF1.0 only
		std::cout << "SENT" << std::endl;
		m_completed = true;
	}
	std::cout << "DONE " << std::endl;
	return m_completed;
}

bool morpheus::csh_features_request::handle_error (rofl::cofdpt *src, 
	rofl::cofmsg_error *msg) {
	ROFL_DEBUG("Warning: %s has received an error message: %s\n", __PRETTY_FUNCTION__, msg->c_str());
	m_completed = true;
	return m_completed;
}

morpheus::csh_features_request::~csh_features_request() { 
	ROFL_DEBUG("%s called.\n",__PRETTY_FUNCTION__); 
}

std::string morpheus::csh_features_request::asString() const { std::stringstream ss; ss << "csh_features_request {request_xid=" << m_request_xid << "}"; return ss.str(); }

