#include "xcpd_config.h"
#include <rofl/platform/unix/cunixenv.h>
#include <rofl/common/utils/c_logger.h>


using namespace xdpd;
using namespace rofl;
using namespace libconfig; 

void xcpd_config::init(int args, char** argv){
	Config* cfg = new Config;
	xcpd_root_scope* root = new xcpd_root_scope();
	cunixenv env_parser(args, argv);

	//Add paramters
	//Not required

	//Parse
	env_parser.parse_args();


	//Execute
	cfg = new Config;
	root = new xcpd_root_scope();

    if(!env_parser.is_arg_set("config-file")){
		ROFL_ERR("No configuration file specified either via -c or --config-file\n");	
		throw eConfParamNotFound();
	}
    std::string conf_file;

	try{
		conf_file = env_parser.get_arg("config-file").c_str();
		cfg->readFile(conf_file.c_str());
	}catch(const FileIOException &fioex){
		ROFL_ERR("Config file %s not found. Aborting...\n",conf_file.c_str());	
		throw eConfFileNotFound();
		throw fioex;
	}catch(ParseException &pex){
		ROFL_ERR("Error while parsing file %s at line: %u \nAborting...\n",conf_file.c_str(),pex.getLine());
		throw eConfParseError();
	}
	root->execute(*cfg);
	delete cfg;
	delete root;
}



xcpd_root_scope::xcpd_root_scope():scope("root"){
	//config subhierarchy
	register_subscope(new xcpd_config_scope());
}

xcpd_config_scope::xcpd_config_scope():scope("config", true){

	//Openflow subhierarchy
	register_subscope(new xcpd_openflow_scope());
	
	//Interfaces subhierarchy
	register_subscope(new xcpd_interfaces_scope());
    
}

xcpd_interfaces_scope::xcpd_interfaces_scope(std::string name, bool mandatory):scope(name, mandatory){
	
	//Register subscopes
	//Subscopes are logical switch elements so will be captured on pre_validate hook
	register_subscope(new xcpd_virtual_ifaces_scope());	

}


xcpd_virtual_ifaces_scope::xcpd_virtual_ifaces_scope(std::string name, bool mandatory):scope(name, mandatory){
	
}


void xcpd_virtual_ifaces_scope::post_validate(libconfig::Setting& setting, bool dry_run){

}

xcpd_openflow_scope::xcpd_openflow_scope(std::string name, bool mandatory):scope(name, mandatory){
	
	//Register parameters
	//None for the moment

	//Register subscopes
	//Subscopes are logical switch elements so will be captured on pre_validate hook
	register_subscope(new xcpd_of_lsis_scope());	

}


xcpd_of_lsis_scope::xcpd_of_lsis_scope(std::string name, bool mandatory):scope(name, mandatory){
	
	//Register subscopes
	//Subscopes are logical switch elements so will be captured on pre_validate hook
	
}


void xcpd_of_lsis_scope::pre_validate(libconfig::Setting& setting, bool dry_run){

		
}

xcpd_lsi_scope::xcpd_lsi_scope(std::string name, bool mandatory):scope(name, mandatory){
}

void xcpd_lsi_scope::post_validate(libconfig::Setting& setting, bool dry_run){
    
}
