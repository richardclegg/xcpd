/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "flow_entry_translate.h"

bool flow_entry_translate::match_fe(cflowentry &f1, cflowentry &f2)
{
    if (f1.get_command() != f2.get_command())
		return false;
	if (f1.get_table_id() != f2.get_table_id())
		return false;
	if (f1.get_idle_timeout() != f2.get_idle_timeout())
		return false;
	if (f1.get_hard_timeout() != f2.get_hard_timeout())
		return false;
	if (f1.get_cookie() != f2.get_cookie())
		return false;
	if (f1.get_cookie_mask() != f2.get_cookie_mask())
		return false;
	if (f1.get_priority() != f2.get_priority())
		return false;
	if (f1.get_buffer_id() != f2.get_buffer_id())
		return false;
	if (f1.get_out_port() != f2.get_out_port())
		return false;
	if (f1.get_out_group() != f2.get_out_group())
		return false;
	if (f1.get_flags() != f2.get_flags())
		return false;
    return f1.match == f2.match;
}

flow_entry_translate::flow_entry_translate()
{
    translate= std::vector < cflowentry >();
    untranslate= std::vector < cflowentry >();
}

void flow_entry_translate::add_flow_entry( cflowentry &fe) {
    cflowentry trans= trans_flow_entry(fe);
    translate.push_back(fe);
    untranslate.push_back(trans);
}

void flow_entry_translate::del_flow_entry(cflowentry &fe) {
    
    for (unsigned int i= 0; i < translate.size(); i++) {
        if (match_fe(translate[i],fe)) {
            std::cout << "match";
        }
    }
    
}

cflowentry flow_entry_translate::trans_flow_entry(cflowentry &fe) {
    return fe;
}

cflowentry flow_entry_translate::untrans_flow_entry(cflowentry &fe) {
    return fe;
}
