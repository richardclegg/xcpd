#include "csh_port_stats.h"
#include "control_manager.h"
#include "hardware_management/hardware_manager.h"
#include "control_manager.h"
#include <rofl/common/utils/c_logger.h>

using namespace xdpd;

morpheus::csh_port_stats::csh_port_stats(morpheus * parent, rofl::cofctl * const src, rofl::cofmsg_port_stats_request * const msg):chandlersession_base(parent) {
	std::cout << __PRETTY_FUNCTION__ << " called." << std::endl;
	process_port_stats_request(src, msg);
	}

bool morpheus::csh_port_stats::process_port_stats_request ( rofl::cofctl * const src, rofl::cofmsg_port_stats_request * const msg ) {
	if(msg->get_version() != OFP10_VERSION) throw rofl::eBadVersion();
	int type= control_manager::Instance()->get_port_config_handling();
    if (type == control_manager::DROP_COMMAND) {
        m_completed= true;
        return true;
    }
    if (type == control_manager::PASSTHROUGH_COMMAND) {
        const cportvlan_mapper & mapper = m_parent->get_mapper();
        
        cportvlan_mapper::port_spec_t real_port= 
            mapper.get_actual_port( msg->get_port_stats().get_portno() );
        uint32_t new_port= real_port.port;
        rofl::cofport_stats_request new_msg(msg->get_version(), new_port);
        m_parent->send_port_stats_request(m_parent->get_dpt(), 
            (uint16_t)msg->get_stats_flags(), new_msg);
        m_completed = true;
        return m_completed;
    }
    if (type == control_manager::HARDWARE_SPECIFIC_COMMAND) {
        hardware_manager *hwm= control_manager::Instance()->get_hardware_manager();
        if (hwm == NULL) {
            ROFL_ERR("No hardware manager defined for port-mod in %s\n",
                type, __PRETTY_FUNCTION__);
            throw rofl::eInval();
        }
        hwm->process_port_stats_request (src, msg );
        m_completed= true;
        return true;
    }
    ROFL_ERR("Undefined value for command type %d in %s\n",
        type, __PRETTY_FUNCTION__);
    throw rofl::eInval();
}

bool morpheus::csh_port_stats::process_port_stats_reply ( rofl::cofdpt * const src, rofl::cofmsg_port_stats_reply * const msg ) {
	assert(!m_completed);
//	const cportvlan_mapper & mapper = m_parent->get_mapper();
	if(msg->get_version() != OFP10_VERSION) throw rofl::eBadVersion();
	
	m_completed = true;
	return m_completed;
}

morpheus::csh_port_stats::~csh_port_stats() { std::cout << __FUNCTION__ << " called." << std::endl; }

std::string morpheus::csh_port_stats::asString() const { std::stringstream ss; ss << "csh_port_stats {request_xid=" << m_request_xid << "}"; return ss.str(); }
