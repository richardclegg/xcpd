#include "xcpd_scope.h"
#include "virtual_port_scope.h"
#include <vector>
#include <stdlib.h>
#include <inttypes.h>
#include "../control_manager.h"
#include "../hardware_management/hardware_manager.h"

using namespace xdpd;
using namespace rofl;

#define XCPD_HIGHER_CONTROLLER_IP "higher-controller-ip"
#define XCPD_HIGHER_CONTROLLER_PORT "higher-controller-port"
#define XCPD_SWITCH_IP "switch-ip"
#define XCPD_SWITCH_PORT "switch-port"
#define XCPD_MODE "upward-mode"
#define XCPD_ACTIVE_MODE "active"
#define XCPD_PASSIVE_MODE "passive"
#define HARDWARE_MANAGER "hardware-manager"
#define HARDWARE_PLANET_GEPON "planet-gepon"
#define HARDWARE_PARMS "hardware-parameters"
#define QUEUE_COMMAND_HANDLING "queue-command-handling"
#define PORT_STAT_HANDLING "port-stat-handling"
#define PORT_CONFIG_HANDLING "port-config-handling"
#define HANDLE_DROP "drop"
#define HANDLE_PASSTHROUGH "passthrough"
#define HANDLE_HARDWARE_SPECIFIC "hardware-specific"

xcpd_scope::xcpd_scope(std::string name, bool mandatory):scope(name, mandatory){
    register_parameter(XCPD_HIGHER_CONTROLLER_IP, true);
	register_parameter(XCPD_HIGHER_CONTROLLER_PORT, true);
    register_parameter(XCPD_MODE, false);
    register_parameter(HARDWARE_MANAGER,false);
    register_parameter(HARDWARE_PARMS,false);
    register_parameter(QUEUE_COMMAND_HANDLING,false);
    register_parameter(PORT_STAT_HANDLING,false);
    register_parameter(PORT_CONFIG_HANDLING,false);
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
    if (dry_run && setting.exists(QUEUE_COMMAND_HANDLING)) {
        control_manager::Instance()->set_queue_command_handling
            (parse_command_handling(setting,setting[QUEUE_COMMAND_HANDLING]));
    }
    if (dry_run && setting.exists(PORT_STAT_HANDLING)) {
        control_manager::Instance()->set_port_stat_handling
            (parse_command_handling(setting,setting[PORT_STAT_HANDLING]));
    }
    if (dry_run && setting.exists(PORT_CONFIG_HANDLING)) {
        control_manager::Instance()->set_port_config_handling
            (parse_command_handling(setting,setting[PORT_CONFIG_HANDLING]));
    }
    if (dry_run && setting.exists(HARDWARE_MANAGER)) {
        parse_hardware_manager(setting,setting[HARDWARE_MANAGER]);
    }
}

void xcpd_scope::parse_hardware_manager(libconfig::Setting& setting,std::string hm)
{
    std::vector<std::string> parms= std::vector<std::string>();
    if (setting.exists(HARDWARE_PARMS)) {
        for(int i=0; i<setting[HARDWARE_PARMS].getLength(); ++i){
            std::string s= setting[HARDWARE_PARMS][i];
            parms.push_back(s);
        }
    } 
    if (hm == HARDWARE_PLANET_GEPON) {
        planet_gepon_manager *hwm = new planet_gepon_manager();
        control_manager::Instance()->set_hardware_manager(hwm,parms);
        return;
    }
    ROFL_ERR("Unrecognised hardware manager defined %s\n", hm.c_str());
            throw eConfParseError();     
}
    
int xcpd_scope::parse_command_handling(libconfig::Setting& setting, std::string ch)
{
    if (ch == HANDLE_DROP) 
        return control_manager::DROP_COMMAND; 
    if (ch == HANDLE_PASSTHROUGH) 
        return control_manager::PASSTHROUGH_COMMAND;  
    if (ch == HANDLE_HARDWARE_SPECIFIC) 
        return control_manager::HARDWARE_SPECIFIC_COMMAND;
    ROFL_ERR("Invalid command handler %s\n", ch.c_str());
            throw eConfParseError(); 	
 
}



