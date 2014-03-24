
#ifndef CONFIG_XCPD_VPORT_PLUGIN_H
#define CONFIG_XCPD_VPORT_PLUGIN_H 

#include "../scope.h"

/**
* @file virtual_port_scope.h 
* @author richard@richardclegg.org
*
* @brief Openflow libconfig file for configuring control path daemon
* 
*/


namespace xdpd {

class virtual_port_scope:public scope {
	
public:
	virtual_port_scope(std::string scope_name="virtual-ports", bool mandatory=false);

protected:
    virtual void post_validate(libconfig::Setting& setting, bool dry_run);

};

class one_port_scope:public scope {
    
    std::string port_name;
    
    public:
        one_port_scope(std::string scope_name, bool mandatory=false);

    protected:
        virtual void post_validate(libconfig::Setting& setting, bool dry_run);
    
};

}
#endif
