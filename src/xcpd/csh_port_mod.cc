
#include "csh_port_mod.h"
#include "control_manager.h"
#include <rofl/common/utils/c_logger.h>
#include <rofl/common/cerror.h>
#include <string>

using namespace xdpd;

morpheus::csh_port_mod::csh_port_mod(morpheus * parent, rofl::cofctl * const src, rofl::cofmsg_port_mod * const msg ):chandlersession_base(parent, msg->get_xid()) {
	ROFL_DEBUG("%s called.\n",__PRETTY_FUNCTION__);
	process_port_mod(src, msg);
	}

bool morpheus::csh_port_mod::process_port_mod ( rofl::cofctl * const src, rofl::cofmsg_port_mod * const msg ) {
	if(msg->get_version() != OFP10_VERSION) throw rofl::eBadVersion();
    int type= control_manager::Instance()->get_port_config_handling();
    if (type == control_manager::DROP_COMMAND) {
        m_completed= true;
        return true;
    }
    if (type == control_manager::PASSTHROUGH_COMMAND) {
        const cportvlan_mapper & mapper = m_parent->get_mapper();
        
        try {
			cportvlan_mapper::port_spec_t real_port= 
				mapper.get_actual_port( msg->get_port_no() );
			uint32_t new_port= real_port.port;
			m_parent->send_port_mod_message( m_parent->get_dpt(), 
				new_port, msg->get_hwaddr(), 
				msg->get_config(), msg->get_mask(), 
				msg->get_advertise() );
			m_completed = true;
			return m_completed;
		} catch (std::out_of_range) {
			ROFL_DEBUG("%s: Port out of range %ld \n", __PRETTY_FUNCTION__,
				msg->get_port_no());
			size_t datalen = (msg->framelen() > 64) ? 64 : msg->framelen();
			m_parent->send_error_message(
			m_parent->get_ctl(),
			msg->get_xid(),
			OFPET_PORT_MOD_FAILED,
			OFPPMFC_BAD_PORT,
			(unsigned char*)msg->soframe(),
			datalen
			);	
			m_completed= true;
			return m_completed;
		}
		
    }
    if (type == control_manager::HARDWARE_SPECIFIC_COMMAND) {
        hardware_manager *hwm= control_manager::Instance()->get_hardware_manager();
        if (hwm == NULL) {
            ROFL_ERR("No hardware manager defined for port-mod in %s\n",
                type, __PRETTY_FUNCTION__);
            throw rofl::eInval();
        }
        hwm->process_port_mod (src, msg );
        m_completed= true;
        return true;
    }
    ROFL_ERR("Undefined value for command type %d in %s\n",
        type, __PRETTY_FUNCTION__);
    throw rofl::eInval();
}

morpheus::csh_port_mod::~csh_port_mod() { 
	ROFL_DEBUG ("%s: called.",__PRETTY_FUNCTION__);
}


std::string morpheus::csh_port_mod::asString() const 
{ 
	std::stringstream ss; ss << 
		"csh_port_mod {request_xid=" << m_request_xid 
		<< "}"; return ss.str(); 
}
