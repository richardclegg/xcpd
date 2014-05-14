#include "virtual_port_scope.h"
#include <vector>
#include <stdlib.h>
#include <inttypes.h>

using namespace xdpd;
using namespace rofl;



#define VPORT_ASSOCIATED_PHYSICAL_PORT "physical"
#define VPORT_ASSOCIATED_VLAN "vlan"

virtual_port_scope::virtual_port_scope(std::string name, bool mandatory):scope(name, mandatory){

}

void virtual_port_scope::pre_validate(libconfig::Setting& setting, bool dry_run){
//Detect existing subscopes (for virtual ports) and register
 	for(int i = 0; i<setting.getLength(); ++i){
        ROFL_INFO("Subscope for switch %s\n", setting[i].getName());
		register_subscope(std::string(setting[i].getName()), new switch_vports_scope(setting[i].getName()));
	}
}

switch_vports_scope::switch_vports_scope(std::string name, bool mandatory):scope(name, mandatory){
    switch_name= name;
    ROFL_INFO("Registering new switch vports scope %s\n", name.c_str());
}

void switch_vports_scope::pre_validate(libconfig::Setting& setting, bool dry_run){
//Detect existing subscopes (for virtual ports) and register
 	for(int i = 0; i<setting.getLength(); ++i){
        one_port_scope *ops=  new one_port_scope(setting[i].getName());
        ROFL_INFO("Sub-Subscope for port %s\n", setting[i].getName());
		register_subscope(std::string(setting[i].getName()), ops);
	}
}

one_port_scope::one_port_scope(std::string name, bool mandatory):scope(name, mandatory){
    port_name= name;
    register_parameter(VPORT_ASSOCIATED_PHYSICAL_PORT, true);
    register_parameter(VPORT_ASSOCIATED_VLAN, false);
    ROFL_INFO("Found virtual port switch: %s\n", name.c_str());
}

void one_port_scope::post_validate(libconfig::Setting& setting, bool dry_run){

}
