/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CONTROL_MANAGER_H
#define CONTROL_MANAGER_H 
#include <rofl/common/crofbase.h>

/**
* @file control_manager.h
* @author Richard Clegg richard@richardclegg.org
*
* @brief Manages control path daemon parameters
* Singleton contains configuration information
*/

using namespace rofl;

namespace xdpd {
    
class control_manager {
    
    private:
        static control_manager* cm_instance;
        bool code_is_xdpd;  // Is the program xdpd or xcpd
        caddress switch_addr;
        caddress higher_controller_addr;
        control_manager(){};  // Private constructor
        control_manager(control_manager const&){}; // private copy
        control_manager& operator=(control_manager const&); // private assign
        
    public:
        void init();                 // Initialise singleton
        bool is_data_path();         // Return true if xdpd
        void set_control_path();         // Set xcpd
        void set_data_path();         // Set xdpd
        static control_manager *Instance();
        void set_higher_address(caddress &);
        caddress get_higher_address();
        void set_switch_address(caddress &);
        caddress get_switch_address();
};

}

#endif
