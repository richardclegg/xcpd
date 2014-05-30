/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <rofl/common/openflow/cflowentry.h>
#include <map>

using namespace rofl;

class compare_fe {
    public:
      bool operator()(const cflowentry &, const cflowentry &);
};

class flow_entry_translate {
    
    private:
        std::map < cflowentry , cflowentry ,compare_fe> translate;
        std::map < cflowentry ,  cflowentry ,compare_fe> untranslate;
        
    public:
        flow_entry_translate();
        ~flow_entry_translate() {};
        
        void add_flow_entry( cflowentry &);
        void del_flow_entry( cflowentry &);
        cflowentry trans_flow_entry( cflowentry &);
        cflowentry untrans_flow_entry( cflowentry &);
    
};
