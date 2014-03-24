#include "virtual_port_scope.h"
#include <vector>
#include <stdlib.h>
#include <inttypes.h>

using namespace xdpd;
using namespace rofl;

#define VPORT_ASSOCIATED_PHYSICAL_PORT "physical"

virtual_port_scope::virtual_port_scope(std::string name, bool mandatory):scope(name, mandatory){

}

void virtual_port_scope::post_validate(libconfig::Setting& setting, bool dry_run){
//Detect existing subscopes (for virtual ports) and register
 	for(int i = 0; i<setting.getLength(); ++i){
		register_subscope(std::string(setting[i].getName()), new one_port_scope(setting[i].getName()));
	}
}



one_port_scope::one_port_scope(std::string name, bool mandatory):scope(name, mandatory){
    port_name= name;
    register_parameter(VPORT_ASSOCIATED_PHYSICAL_PORT, true);
    ROFL_WARN("Found virtual port switch: %s\n", name.c_str());
}

void one_port_scope::post_validate(libconfig::Setting& setting, bool dry_run){

}
