/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PLANET_GEPON_MANAGER_H
#define PLANET_GEPON_MANAGER_H 

#include <string>
#include "hardware_manager.h"

class planet_gepon_manager : public hardware_manager
{
    public:
        planet_gepon_manager();
        ~planet_gepon_manager();
        void init(std::string);
};

#endif
