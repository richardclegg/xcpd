/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef HARDWARE_MANAGER_H
#define HARDWARE_MANAGER_H 

// Include ALL hardware manager header files here, do not give them
// their own

#include <string>
#include <vector>
#include <rofl/common/openflow/cofctlImpl.h>
#include <rofl/common/openflow/cofctl.h>


class hardware_manager
{
    public:
        virtual void init(std::vector<std::string>)= 0;
        virtual bool process_port_mod ( rofl::cofctl * const , 
            rofl::cofmsg_port_mod * const)= 0;
        virtual bool process_port_stats_request( rofl::cofctl * const, 
            rofl::cofmsg_port_stats_request * const)= 0;
};

class planet_gepon_manager : public hardware_manager
{
    public:
        planet_gepon_manager();
        ~planet_gepon_manager();
        void init(std::vector<std::string>);
        bool process_port_mod ( rofl::cofctl * const ctl, 
            rofl::cofmsg_port_mod * const msg);
        bool process_port_stats_request( rofl::cofctl * const, 
            rofl::cofmsg_port_stats_request * const);
};

#endif
