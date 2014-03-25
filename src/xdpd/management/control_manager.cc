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

bool control_manager::code_is_xdpd= true;

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


