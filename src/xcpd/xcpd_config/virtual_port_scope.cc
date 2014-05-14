#include "virtual_port_scope.h"
#include <vector>
#include <stdlib.h>
#include <inttypes.h>
#include "../control_manager.h"

using namespace xdpd;
using namespace rofl;



#define VPORT_ASSOCIATED_PHYSICAL_PORT "physical"
#define VPORT_ASSOCIATED_VLAN "vlan"

virtual_port_scope::virtual_port_scope(std::string name, bool mandatory):scope(name, mandatory){

}

void virtual_port_scope::pre_validate(libconfig::Setting& setting, bool dry_run){
//Detect existing subscopes (for virtual ports) and register
 	if (setting.getLength() != 1) {
        ROFL_ERR("xcpd must define exactly one switch", setting.getPath().c_str());
		throw eConfParseError(); 
    }
    if (!dry_run && std::string(setting[0].getName()) != control_manager::Instance()->get_switch_name()) {
        ROFL_ERR("xcpd switch name %s must match xdpd %s!\n", setting[0].getName(),
           control_manager::Instance()->get_switch_name().c_str());
		throw eConfParseError(); 
    }
    register_subscope(std::string(setting[0].getName()), 
        new switch_vports_scope(setting[0].getName()));
}

switch_vports_scope::switch_vports_scope(std::string name, bool mandatory):scope(name, mandatory){
}

void switch_vports_scope::pre_validate(libconfig::Setting& setting, bool dry_run){
//Detect existing subscopes (for virtual ports) and register
 	
    for(int i = 0; i<setting.getLength(); ++i){
        one_port_scope *ops=  new one_port_scope(setting[i].getName());
		register_subscope(std::string(setting[i].getName()), ops);
	}
}

one_port_scope::one_port_scope(std::string name, bool mandatory):scope(name, mandatory){
    port_name= name;
    register_parameter(VPORT_ASSOCIATED_PHYSICAL_PORT, true);
    register_parameter(VPORT_ASSOCIATED_VLAN, false);
}

void one_port_scope::post_validate(libconfig::Setting& setting, bool dry_run){
    if (!dry_run) {
        int port= 0;
        bool vlan_set= false;
        unsigned int vlan= 0;
        if((setting.exists(VPORT_ASSOCIATED_PHYSICAL_PORT))){
            std::string pstr= setting[VPORT_ASSOCIATED_PHYSICAL_PORT];
            port= control_manager::Instance()->get_port_no(pstr);
            if (port < 0) {
                ROFL_ERR("xcpd cannot find physical port %s\n", pstr.c_str());
                throw eConfParseError(); 
            }
        } else {
            ROFL_ERR("xcpd must define physical port for vport %s\n", port_name.c_str());
            throw eConfParseError(); 
        }
        if((setting.exists(VPORT_ASSOCIATED_VLAN))){
            //vlan= setting[VPORT_ASSOCIATED_VLAN];
            vlan=1;
            vlan_set= true;
        }
        if (vlan_set) {
            virtual_port vport= virtual_port(port_name,port,(int)vlan);
            control_manager::Instance()->add_vport(vport);
        } else {
            virtual_port vport= virtual_port(port_name,port);
            control_manager::Instance()->add_vport(vport);
        }
    }
}
