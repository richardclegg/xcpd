/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CONTROL_MANAGER_H
#define CONTROL_MANAGER_H 
#include <rofl/common/crofbase.h>
#include <vector>
#include <string>
#include "hardware_management/hardware_manager.h"

/**
* @file control_manager.h
* @author Richard Clegg richard@richardclegg.org
*
* @brief Manages control path daemon parameters
* Singleton contains configuration information
* Accesses look like this
* control_manager.Instance()->get_higher_address();
*/

using namespace rofl;

namespace xdpd {

// Class represents a single port recognised by xcpd control    
class virtual_port {
    private:
        
        std::string name;
        int real_port;
        int vlan;
        rofl::cmacaddr mac;
        static int port_count;
        void init_port();
    
    public:
        static const int NO_VLAN= -1;
        virtual_port();
        virtual_port(std::string, int);
        virtual_port(std::string n, int,int);
        rofl::cmacaddr get_mac();
        void set_mac(rofl::cmacaddr);
        std::string get_name();
		void set_name(std::string);
        int get_real_port();
        int get_vlan();
};

// Class represents control_manager
class control_manager {
    
    private:
        // is a connection passive or active 
        static const int PASSIVE_CONNECTION=1;
        static const int ACTIVE_CONNECTION=2;
        

        static control_manager* cm_instance;
        
        uint64_t dpid;
        hardware_manager *hm;
        caddress switch_addr;
        std::string switch_ip;
        int switch_port;
        caddress higher_addr;
        std::string higher_ip;
        int higher_port;
        caddress xcpd_addr;
        std::string xcpd_ip;
        int xcpd_port;
        control_manager(){};  // Private constructor
        control_manager(control_manager const&){}; // private copy
        control_manager& operator=(control_manager const&); // private assign
        std::string switch_name;  // Name of switch to be used
        std::vector<std::string> port_names;  // Ports of switch
        int switch_to_xcpd_conn;
        int xcpd_to_control_conn;
        std::vector<virtual_port> ports;
        int queue_command_handling;
        int port_stat_handling;
        int port_config_handling;

        
    public:
        // Ways in which we can deal with "problem" commands
        static const int DROP_COMMAND=1;
        static const int PASSTHROUGH_COMMAND= 2;
        static const int HARDWARE_SPECIFIC_COMMAND= 3;
        static control_manager *Instance();
        void init();                 // Initialise singleton
        
        void set_dpid(uint64_t);
        uint64_t get_dpid();
        void set_higher_address(caddress &);
        caddress get_higher_address();
        void set_xcpd_address(caddress &);
        caddress get_xcpd_address();
        void set_switch_address(caddress &);
        caddress get_switch_address();
        void set_switch_ip(std::string);
        std::string get_switch_ip();
        void set_switch_port(int);
        int get_switch_port();
        void set_xcpd_ip(std::string);
        std::string get_xcpd_ip();
        void set_xcpd_port(int);
        int get_xcpd_port();
        void set_higher_ip(std::string);
        std::string get_higher_ip();
        void set_higher_port(int);
        int get_higher_port();       
        void set_switch_name(std::string);
        std::string get_switch_name();
        void add_port(std::string);
        int no_ports();
        int get_port_no(std::string);
        void set_switch_to_xcpd_conn_passive();
        void set_switch_to_xcpd_conn_active();        
        bool is_switch_to_xcpd_conn_active();
        void set_xcpd_to_control_conn_passive();
        void set_xcpd_to_control_conn_active();        
        bool is_xcpd_to_control_conn_active(); 
        int no_vports();
        void add_vport(virtual_port);
        virtual_port get_vport(int);
        hardware_manager *get_hardware_manager();
        void set_hardware_manager(hardware_manager *, std::vector<std::string>);
        int get_queue_command_handling();
        void set_queue_command_handling(int);
        int get_port_stat_handling();
        void set_port_stat_handling(int);
        int get_port_config_handling();
        void set_port_config_handling(int);
        
};


}

#endif
