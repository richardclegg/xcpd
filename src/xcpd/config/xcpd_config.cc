#include "xcpd_config.h"
#include <rofl/platform/unix/cunixenv.h>
#include <rofl/common/utils/c_logger.h>
#include "xcpd_scope.h"
#include <iostream>


using namespace xdpd;
using namespace rofl;
using namespace libconfig; 

#define XCPD_MODE "mode"
#define XCPD_ACTIVE_MODE "active"
#define XCPD_PASSIVE_MODE "passive"
#define XCPD_MASTER_CONTROLLER_IP "master-controller-ip"
#define XCPD_MASTER_CONTROLLER_PORT "master-controller-port"
#define XCPD_SLAVE_CONTROLLER_IP "slave-controller-ip"
#define XCPD_SLAVE_CONTROLLER_PORT "slave-controller-port"
#define XCPD_BIND_ADDRESS_IP "bind-address-ip"
#define XCPD_BIND_ADDRESS_PORT "bind-address-port"
#define XCPD_PORTS "ports"
#define XCPD_DPID "dpid"

void xcpd_config::parse_config(Config* cfg, cunixenv& env_parser){

	std::string conf_file;

	if(!env_parser.is_arg_set("config-file")){
		ROFL_ERR("No configuration file specified either via -c or --config-file\n");	
		throw eConfParamNotFound();
	}

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
}

void xcpd_config::init(int args, char** argv){
	Config* cfg = new Config;
	xcpd_root_scope* root = new xcpd_root_scope();
	cunixenv env_parser(args, argv);

	//Add paramters
	//Not required

	//Parse
	env_parser.parse_args();

	//Dry run
	parse_config(cfg,env_parser);
	root->execute(*cfg,true);
	delete cfg;
	delete root;

	//Execute
	cfg = new Config;
	root = new xcpd_root_scope();

	parse_config(cfg,env_parser);
	root->execute(*cfg);
	delete cfg;
	delete root;
}




xcpd_root_scope::xcpd_root_scope():scope("root"){
	//config subhierarchy
	register_subscope(new xcpd_config_scope());
}

xcpd_config_scope::xcpd_config_scope():scope("config", true){
	
      //xcpd config
    register_subscope(new xcpd_scope());
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

xcpd_openflow_scope::xcpd_openflow_scope(std::string name, bool mandatory):scope(name, mandatory){
	register_subscope(new xcpd_of_lsis_scope());	

}


xcpd_of_lsis_scope::xcpd_of_lsis_scope(std::string name, bool mandatory):scope(name, mandatory){
    
	
}

void xcpd_of_lsis_scope::pre_validate(libconfig::Setting& setting, bool dry_run){
    
	if(setting.getLength() == 0){
		ROFL_ERR("%s: No logical switches found!\n", setting.getPath().c_str());
		throw eConfParseError(); 	
		
	}
    if (setting.getLength() != 1) {
        ROFL_ERR("xcpd can only deal with a single switch\n", setting.getPath().c_str());
		throw eConfParseError(); 
    }
	//Detect existing subscopes (logical switches) and register
    register_subscope(std::string(setting[0].getName()), new xcpd_lsi_scope(setting[0].getName()));
    if (dry_run) {
        control_manager::Instance()->set_switch_name(setting[0].getName());
    }
}


xcpd_lsi_scope::xcpd_lsi_scope(std::string name, bool mandatory):scope(name, mandatory){
    register_parameter(XCPD_MODE); 
    register_parameter(XCPD_MASTER_CONTROLLER_IP); 
	register_parameter(XCPD_MASTER_CONTROLLER_PORT); 
	register_parameter(XCPD_SLAVE_CONTROLLER_IP); 
	register_parameter(XCPD_SLAVE_CONTROLLER_PORT); 
    register_parameter(XCPD_PORTS); 
    register_parameter(XCPD_DPID,true);
}

/* Case insensitive */
void xcpd_lsi_scope::post_validate(libconfig::Setting& setting, bool dry_run){
    bool active= true;
    if((setting.exists(XCPD_MODE))){
		std::string mode= setting[XCPD_MODE];
		if( mode == XCPD_PASSIVE_MODE){
            active= false;
			control_manager::Instance()->set_switch_to_xcpd_conn_passive();
            
		} else if(mode == XCPD_ACTIVE_MODE){	
             control_manager::Instance()->set_switch_to_xcpd_conn_active();
             
        } else if(dry_run) {
            ROFL_WARN("%s: Unable to parse mode.. assuming ACTIVE\n", 
                setting.getPath().c_str()); 
            control_manager::Instance()->set_switch_to_xcpd_conn_active();
		} 
    }

    if (!dry_run && setting.exists(XCPD_DPID)) {
		 std::string dpid_s = setting[XCPD_DPID];
         uint64_t d= strtoull(dpid_s.c_str(),NULL,0);
         control_manager::Instance()->set_dpid(d);
    }
    if (active) {
        if (setting.exists(XCPD_BIND_ADDRESS_PORT) ||
           setting.exists(XCPD_BIND_ADDRESS_IP) ) {
            ROFL_ERR("bind-address settings ignored in active mode -- use master-controller\n");
                throw eConfParseError(); 	
        }
        //Parse master controller IP if it exists
        if(setting.exists(XCPD_MASTER_CONTROLLER_IP)){
            std::string ip = setting[XCPD_MASTER_CONTROLLER_IP];
            control_manager::Instance()->set_switch_ip(ip);
        }
    
        //Parse master controller port if it exists 
        if(setting.exists(XCPD_MASTER_CONTROLLER_PORT)){
            int port = setting[XCPD_MASTER_CONTROLLER_PORT];
            if(port < 1 || port > 65535){
                ROFL_ERR("%s: invalid Master controller TCP port number %u. Must be [1-65535]\n", setting.getPath().c_str(), port);
                throw eConfParseError(); 	
                    
            }
            control_manager::Instance()->set_switch_port(port);
            //std::cout << "Setting port to "<< port << std::endl;
        }
    } else {
		 ROFL_ERR("Passive connections from xdpd to xCPd are not necessarily well supported.\n This may cause the controller to be out of sync.\n");
        if (setting.exists(XCPD_MASTER_CONTROLLER_PORT) ||
           setting.exists(XCPD_MASTER_CONTROLLER_IP) ) {
            ROFL_ERR("master-controller settings ignored in active mode -- use bind-address\n");
                throw eConfParseError(); 	
        }
        if(setting.exists(XCPD_BIND_ADDRESS_IP)){
            std::string ip = setting[XCPD_BIND_ADDRESS_IP];
            control_manager::Instance()->set_switch_ip(ip);
        }

        if(setting.exists(XCPD_BIND_ADDRESS_PORT)){
            int port = setting[XCPD_BIND_ADDRESS_PORT];
            if(port < 1 || port > 65535){
                ROFL_ERR("%s: invalid bind address TCP port number %u. Must be [1-65535]\n", setting.getPath().c_str(), port);
                throw eConfParseError(); 	
				
            }
            control_manager::Instance()->set_switch_port(port);
            //std::cout << "Setting port to "<< port << std::endl;
        }
    }
    
	if(!setting.exists(XCPD_PORTS) || !setting[XCPD_PORTS].isList()){
 		ROFL_ERR("%s: missing or unable to parse port attachment list.\n", setting.getPath().c_str());
		throw eConfParseError(); 	
	
	}
    int port_count= 0;
	for(int i=0; i<setting[XCPD_PORTS].getLength(); ++i){
		std::string port = setting[XCPD_PORTS][i];
		if(port != ""){
            port_count++;
            if (dry_run) {
                control_manager::Instance()->add_port(port);
                //ROFL_INFO("%s added port\n",port.c_str());
            }	
		}
	}	

	if(port_count < 2 && dry_run){
 		ROFL_WARN("%s: WARNING the LSI has less than two ports attached.\n", setting.getPath().c_str());
	}
    
}
