/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef HARDWARE_MANAGER_H
#define HARDWARE_MANAGER_H 
#include <string>
#include <vector>

class hardware_manager
{
    public:
        hardware_manager() {};
        ~hardware_manager() {};
        virtual void init(std::vector<std::string>) {};
};
#endif
