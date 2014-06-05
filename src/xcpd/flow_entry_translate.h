/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <rofl/common/openflow/cflowentry.h>
#include <vector>
#include <cportvlan_mapper.h>

using namespace rofl;


class flow_entry_translate {
    
    private:
        cportvlan_mapper mapper;
        std::vector < cflowentry> translate;
        std::vector < cflowentry> untranslate;
        bool match_fe (cflowentry &, cflowentry &);
    public:
        flow_entry_translate();
        ~flow_entry_translate() {};
        
        void add_flow_entry( cflowentry &, cflowentry &);
        void del_flow_entry( cflowentry &);
        cflowentry trans_flow_entry( cflowentry &);
        cflowentry untrans_flow_entry( cflowentry &);
    
};
