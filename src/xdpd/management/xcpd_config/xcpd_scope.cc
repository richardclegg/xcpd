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

xcpd_scope::xcpd_scope(std::string name, bool mandatory):scope(name, mandatory){
	ROFL_INFO("Registering seven subscopes", name.c_str());
    register_parameter(XCPD_HIGHER_CONTROLLER_IP, true);
	register_parameter(XCPD_HIGHER_CONTROLLER_PORT, true);
    register_subscope(new virtual_port_scope());
}


void xcpd_scope::post_validate(libconfig::Setting& setting, bool dry_run){
    caddress higher_controller;
    std::string ip = setting[XCPD_HIGHER_CONTROLLER_IP];
    unsigned int port= setting[XCPD_HIGHER_CONTROLLER_PORT];
    try{
		//IPv4
		higher_controller = caddress(AF_INET, ip.c_str(), port); 
	}catch(...){
		//IPv6
		higher_controller = caddress(AF_INET6, ip.c_str(), port); 
	}
    control_manager::Instance()->set_higher_address(higher_controller);
    ROFL_INFO("setting address: %s\n", name.c_str());
}
