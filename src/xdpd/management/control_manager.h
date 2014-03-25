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
*/

using namespace rofl;

namespace xdpd {
    
class control_manager {
    
    private:
        static bool code_is_xdpd;  // Is the program xdpd or xcpd
    public:
        
        static bool is_data_path();         // Return true if xdpd
        static void set_control_path();         // Set xcpd
        static void set_data_path();         // Set xdpd
    
};

}

#endif
