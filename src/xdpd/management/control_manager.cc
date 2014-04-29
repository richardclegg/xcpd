/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "control_manager.h"

/**
* @file control_manager.cc
* @author Richard Clegg richard@richardclegg.org
*
* @brief Manages control path daemon parameters
*/

using namespace rofl;
using namespace xdpd;

control_manager* control_manager::cm_instance= NULL;

bool control_manager::is_data_path()
{
    return code_is_xdpd;
}

void control_manager::set_control_path()
{
    code_is_xdpd= false;
}

void control_manager::set_data_path()
{
    code_is_xdpd= true;
}

control_manager* control_manager::Instance()
{
    if (!cm_instance) {
        cm_instance= new control_manager;
    }
    return cm_instance;
}

void control_manager::init()
// Set default 
{
    switch_addr= caddress(AF_INET, "127.0.0.1",6632);
    higher_controller_addr= caddress(AF_INET, "127.0.0.1",6633);
}

void control_manager::set_higher_address(caddress &c)
{
    higher_controller_addr=c;
}

caddress control_manager::get_higher_address()
{
    return higher_controller_addr;
}

void control_manager::set_switch_address(caddress &c)
{
    switch_addr=c;
}

caddress control_manager::get_switch_address()
{
    return switch_addr;
}


