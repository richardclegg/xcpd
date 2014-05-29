/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "hardware_manager.h"
#include <rofl/common/utils/c_logger.h>
#include <rofl/common/cerror.h>
#include <string>


//Specific hardware manager for planet gepon

planet_gepon_manager::planet_gepon_manager()
{
}

planet_gepon_manager::~planet_gepon_manager()
{
}

void planet_gepon_manager::init(std::vector<std::string> parms)
{
}

bool planet_gepon_manager::process_port_mod 
    ( rofl::cofctl * const ctl, 
            rofl::cofmsg_port_mod * const msg)
{
    ROFL_ERR("Process_port_mod not written for planet GEPON %s\n",
         __PRETTY_FUNCTION__);
    throw rofl::eInval();
}
