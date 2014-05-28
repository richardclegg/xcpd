
#ifndef CONFIG_XCPD_PLUGIN_H
#define CONFIG_XCPD_PLUGIN_H 
#include <libconfig.h++> 
#include <rofl/common/caddress.h>
#include "../../xdpd/management/plugins/config/scope.h"
//#include "../../xdpd/management/plugin_manager.h"
#include <string>

/**
* @file xcpd_scope.h 
* @author richard@richardclegg.org
*
* @brief Openflow libconfig file for configuring control path daemon
* 
*/
namespace xdpd {

class xcpd_scope:public scope {

public:
	xcpd_scope(std::string scope_name="xcpd", bool mandatory=false);
		
protected:
	virtual void post_validate(libconfig::Setting& setting, bool dry_run);

private:
    int parse_command_handling(libconfig::Setting& , std::string);
    void parse_hardware_manager(libconfig::Setting& , std::string);
};

}// namespace xdpd 

#endif /* CONFIG_XCPD_PLUGIN_H_ */
