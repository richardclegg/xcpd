/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FLOW_ENTRY_TRANSLATE_H
#define FLOW_ENTRY_TRANSLATE_H

#include <rofl/common/openflow/cflowentry.h>
#include <vector>
#include "cportvlan_mapper.h"
#include "morpheus.h"

using namespace rofl;

class morpheus;     // Forward declaration necessary due to nested include

class flow_entry_translate {

    private:
        morpheus *m_parent;
        cportvlan_mapper mapper;
        std::vector < cflowentry> translate;
        std::vector < cflowentry> untranslate;
        bool match_fe (cflowentry &, cflowentry &);
    public:
        flow_entry_translate(morpheus * const);
        ~flow_entry_translate() {};
        rofl::cofaclist trans_actions(rofl::cofaclist &, 
            rofl::cofmatch &);
        rofl::cflowentry get_flowentry_from_msg(rofl::cofmsg_flow_mod * const);
        rofl::cofmatch trans_match(rofl::cofmatch &); 
        void add_flow_entry( cflowentry &, cflowentry &);
        void del_flow_entry( cflowentry &);
        cflowentry trans_flow_entry( cflowentry &);
        cflowentry untrans_flow_entry( cflowentry &);
        std::vector <cflowentry> get_translated_matches
            (rofl::cofmatch &, bool);
        std::vector <cflowentry> get_translated_matches_and_modify
            (rofl::cofmatch &, rofl::cofaclist &, bool);
        std::vector <cflowentry> get_translated_matches_and_delete
            (rofl::cofmatch &, uint32_t, bool);    
};

#endif
