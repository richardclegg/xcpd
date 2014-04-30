
#ifndef CONFIG_XCPD_VPORT_PLUGIN_H
#define CONFIG_XCPD_VPORT_PLUGIN_H 

#include "../plugins/config/scope.h"

/**
* @file virtual_port_scope.h 
* @author richard@richardclegg.org
*
* @brief Openflow libconfig file for configuring control path daemon
* 
*/


namespace xdpd {

// Top level config just says "This is part of config for virtual ports

class virtual_port_scope:public scope {
	
public:
	virtual_port_scope(std::string scope_name="virtual-ports", bool mandatory=false);

protected:
    virtual void pre_validate(libconfig::Setting& setting, bool dry_run);

};

// Next level config defines which switch within xdpd we define virtual
// ports for

class switch_vports_scope:public scope  {
    
    private:
        std::string switch_name;
    public:
        switch_vports_scope(std::string scope_name, bool mandatory=false);

    protected:
        virtual void pre_validate(libconfig::Setting& setting, bool dry_run);
    
};

// Lowest level config defines nature of virtual port

class one_port_scope:public scope {
    
    private:
        std::string port_name;
    public:
        one_port_scope(std::string scope_name, bool mandatory=false);

    protected:
        virtual void post_validate(libconfig::Setting& setting, bool dry_run);
    
};

}
#endif
