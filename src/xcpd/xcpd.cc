/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <rofl/platform/unix/cunixenv.h>
#include <rofl/datapath/afa/fwd_module.h>
#include <rofl/common/utils/c_logger.h>
#include "config/xcpd_config.h"
#include "control_manager.h"
#include "morpheus.h"
#include "cportvlan_mapper.h"
#include <string>

using namespace rofl;
using namespace xdpd;

extern int optind;

//TODO: Redirect C loggers to the output log
#define XDPD_LOG_FILE "xcpd.log"

//Handler to stop ciosrv
void interrupt_handler(int dummy=0) {
	//Only stop ciosrv 
	ciosrv::stop();
}

//Prints version and build numbers and exits
void dump_version(){
	//Print version and exit
	ROFL_INFO("The eXtensible OpenFlow Control Path daemon (xCPd)\n");	
	ROFL_INFO("Version: %s\n",XCPD_VERSION);

#ifdef XDPD_BUILD
	ROFL_INFO("Build: %s\n",XCPD_BUILD);
	ROFL_INFO("Compiled in branch: %s\n",XCPD_BRANCH);
	ROFL_INFO("%s\n",XCPD_DESCRIBE);
#endif	
	ROFL_INFO("\n[Libraries: ROFL]\n");
	ROFL_INFO("ROFL version: %s\n",ROFL_VERSION);
	ROFL_INFO("ROFL build: %s\n",ROFL_BUILD_NUM);
	ROFL_INFO("ROFL compiled in branch: %s\n",ROFL_BUILD_BRANCH);
	ROFL_INFO("%s\n",ROFL_BUILD_DESCRIBE);
	
	exit(EXIT_SUCCESS);

}

/*
 * xCPd Main routine
 */
int main(int argc, char** argv){


	//Check for root privileges 
	if(geteuid() != 0){
		ROFL_ERR("ERROR: Root permissions are required to run %s\n",argv[0]);	
		exit(EXIT_FAILURE);	
	}

	//Capture control+C
	signal(SIGINT, interrupt_handler);

#if DEBUG && VERBOSE_DEBUG
	//Set verbose debug if necessary
	rofl_set_logging_level(/*cn,*/ DBG_VERBOSE_LEVEL);
#endif
	
	/* Add additional arguments */
	char s_dbg[32];
	memset(s_dbg, 0, sizeof(s_dbg));
	snprintf(s_dbg, sizeof(s_dbg)-1, "%d", (int)csyslog::DBG);


	{ //Make valgrind happy
		cunixenv env_parser(argc, argv);
		
		/* update defaults */
		env_parser.update_default_option("logfile", XDPD_LOG_FILE);
		env_parser.add_option(coption(true, NO_ARGUMENT, 'v', "version", "Retrieve xcPd version and exit", std::string("")));

		//Parse
		env_parser.parse_args();

		if (env_parser.is_arg_set("version")) {
			dump_version();
		}
		
		if (not env_parser.is_arg_set("daemonize")) {
			// only do this in non
			std::string ident(XDPD_LOG_FILE);

			csyslog::initlog(csyslog::LOGTYPE_FILE,
					static_cast<csyslog::DebugLevel>(atoi(env_parser.get_arg("debug").c_str())), // todo needs checking
					env_parser.get_arg("logfile"),
					ident.c_str());
		}
	}
    // Initialise control_manager
    
    control_manager *cm= control_manager::Instance();
    
    cm->init();
    
	ciosrv::init();

	//Don't need all plugins, just a variant of the config plugins
	optind=0;
    xcpd_config *c= new xcpd_config();
    ROFL_INFO("Reading config\n");
	c->init(argc, argv);
    
    cportvlan_mapper mapper;
    for (int i=0; i < cm->no_vports(); i++) {
        virtual_port vp= cm->get_vport(i);
        cportvlan_mapper::port_spec_t::PORT rp(vp.get_real_port()+1);
        if ( vp.get_vlan() == virtual_port::NO_VLAN) {
            mapper.add_virtual_port( cportvlan_mapper::port_spec_t( PV_PORT_T(rp),  
                PV_VLANID_T::NONE  ));
            
           // ROFL_INFO("Added vport, real port %d\n",vp.get_real_port());
        } else {
            cportvlan_mapper::port_spec_t::VLANID vl(vp.get_vlan());
            mapper.add_virtual_port( cportvlan_mapper::port_spec_t( PV_PORT_T(rp), 
               vl));
            //ROFL_INFO("Added vport, real port %d VLAN %d\n",vp.get_real_port(),
            //    vp.get_vlan());
        }
    }
    
    rofl::csyslog::set_all_debug_levels(rofl::csyslog::DBG);

//	rofl::csyslog::set_all_debug_levels(rofl::csyslog::INFO);
	rofl::csyslog::set_debug_level("ciosrv", "emergency");
	rofl::csyslog::set_debug_level("cthread", "emergency");
    rofl::ciosrv::init();
    ROFL_INFO("Running\n");
    ROFL_DEBUG("Debugging switched on for xcpd.\n");
    bool indpt= !cm->is_switch_to_xcpd_conn_active();
    bool inctl=  !cm->is_xcpd_to_control_conn_active();
    caddress dptaddr= cm->get_switch_address();
    caddress ctladdr= cm->get_higher_address();
    
    ROFL_DEBUG("Controller connections will be %s %s\n ", 
    (inctl?"PASSIVE at ":"ACTIVE to "), ctladdr.c_str());
    ROFL_DEBUG("DPE connections will be %s %s\n", 
        (indpt?"PASSIVE at ":"ACTIVE to "),  dptaddr.c_str());
    
    morpheus morph (mapper, indpt, dptaddr, inctl, ctladdr);
    
    ROFL_INFO("Connecting to switch and controller\n");
    morph.initialiseConnections();
	//ciosrv run. Only will stop in Ctrl+C
	ciosrv::run();

	//Printing nice trace
	ROFL_INFO("\nCleaning the house...\n");	

	//Let plugin manager destroy all registered plugins
	delete c;
	
	//ciosrv destroy
	ciosrv::destroy();
	
	ROFL_INFO("House cleaned!\nGoodbye\n");
	
	exit(EXIT_SUCCESS);
}




