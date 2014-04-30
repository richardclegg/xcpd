
#ifndef CONFIG_XCPD_PLUGIN_H
#define CONFIG_XCPD_PLUGIN_H 
#include <libconfig.h++> 
#include <rofl/common/caddress.h>
#include "../plugins/config/scope.h"
#include "../plugin_manager.h"

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
};

}// namespace xdpd 

#endif /* CONFIG_XCPD_PLUGIN_H_ */
