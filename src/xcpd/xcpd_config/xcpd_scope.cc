#include "xcpd_scope.h"
#include "virtual_port_scope.h"
#include <vector>
#include <stdlib.h>
#include <inttypes.h>
#include "../control_manager.h"

using namespace xdpd;
using namespace rofl;

#define XCPD_HIGHER_CONTROLLER_IP "higher-controller-ip"
#define XCPD_HIGHER_CONTROLLER_PORT "higher-controller-port"
#define XCPD_SWITCH_IP "switch-ip"
#define XCPD_SWITCH_PORT "switch-port"
#define XCPD_MODE "upward-mode"
#define XCPD_ACTIVE_MODE "active"
#define XCPD_PASSIVE_MODE "passive"


xcpd_scope::xcpd_scope(std::string name, bool mandatory):scope(name, mandatory){
    register_parameter(XCPD_HIGHER_CONTROLLER_IP, true);
	register_parameter(XCPD_HIGHER_CONTROLLER_PORT, true);
    register_parameter(XCPD_MODE, false);
    register_subscope(new virtual_port_scope());
}


void xcpd_scope::post_validate(libconfig::Setting& setting, bool dry_run){
    
    //ROFL_INFO("Reading XCPD scope\n");
    if((setting.exists(XCPD_MODE))){
        
		std::string mode= setting[XCPD_MODE];
		if( mode == XCPD_PASSIVE_MODE){
			control_manager::Instance()->set_xcpd_to_control_conn_passive();
		} else if(mode == XCPD_ACTIVE_MODE){	
             control_manager::Instance()->set_xcpd_to_control_conn_active();
        } else if(dry_run) {
            ROFL_WARN("%s: Unable to parse xcpd mode.. assuming ACTIVE\n", 
                setting.getPath().c_str()); 
            control_manager::Instance()->set_xcpd_to_control_conn_active();
		} 
    }
    
        
    if(setting.exists(XCPD_HIGHER_CONTROLLER_IP)){
        std::string ip = setting[XCPD_HIGHER_CONTROLLER_IP];
        control_manager::Instance()->set_higher_ip(ip);
    }

    //Parse master controller port if it exists 
    if(setting.exists(XCPD_HIGHER_CONTROLLER_PORT)){
        int port = setting[XCPD_HIGHER_CONTROLLER_PORT];
        if(port < 1 || port > 65535){
            ROFL_ERR("%s: invalid Master controller TCP port number %u. Must be [1-65535]\n", setting.getPath().c_str(), port);
            throw eConfParseError(); 	
                
        }
        control_manager::Instance()->set_higher_port(port);
        
    } 
    
    if(setting.exists(XCPD_SWITCH_IP)){
        std::string ip = setting[XCPD_SWITCH_IP];
        control_manager::Instance()->set_switch_ip(ip);
    }

    //Parse master controller port if it exists 
    if(setting.exists(XCPD_SWITCH_PORT)){
        int port = setting[XCPD_SWITCH_PORT];
        if(port < 1 || port > 65535){
            ROFL_ERR("%s: invalid Master controller TCP port number %u. Must be [1-65535]\n", setting.getPath().c_str(), port);
            throw eConfParseError(); 	
                
        }
        control_manager::Instance()->set_switch_port(port);
        
    }       
       
}
