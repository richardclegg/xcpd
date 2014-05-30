/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "flow_entry_translate.h"


bool compare_fe::operator()(const cflowentry &fe1,const  cflowentry &fe2) 
{
    if (fe1.get_command() != fe2.get_command())
		return (fe1.get_command() < fe2.get_command());
	if (fe1.get_table_id() != fe2.get_table_id())
		return (fe1.get_table_id() < fe2.get_table_id());
	if (fe1.get_idle_timeout() != fe2.get_idle_timeout())
		return (fe1.get_idle_timeout() < fe2.get_idle_timeout());
	if (fe1.get_hard_timeout() != fe2.get_hard_timeout())
		return (fe1.get_hard_timeout() < fe2.get_hard_timeout());
	if (fe1.get_cookie() != fe2.get_cookie())
		return (fe1.get_cookie() < fe2.get_cookie());
	if (fe1.get_cookie_mask() != fe2.get_cookie_mask())
		return (fe1.get_cookie_mask() < fe2.get_cookie_mask());
	if (fe1.get_priority() != fe2.get_priority())
		return (fe1.get_priority() < fe2.get_priority());
	if (fe1.get_buffer_id() != fe2.get_buffer_id())
		return (fe1.get_buffer_id() < fe2.get_buffer_id());
	if (fe1.get_out_port() != fe2.get_out_port())
		return (fe1.get_out_port() < fe2.get_out_port());
	if (fe1.get_out_group() != fe2.get_out_group())
		return (fe1.get_out_group() < fe2.get_out_group());
	if (fe1.get_flags() != fe2.get_flags())
		return (fe1.get_flags() < fe2.get_flags());
    //if (fe1.get_match().get != fe2.get_version()) 
    //   return (fe1.get_version() != fe2.get_version());
	//return fe1.match.get_const_match() < 
    //    fe2.match.get_const_match();
    return true;
}

flow_entry_translate::flow_entry_translate()
{
    translate= std::map < cflowentry ,  cflowentry ,compare_fe>();
    untranslate= std::map < cflowentry ,  cflowentry ,compare_fe>();
}

void flow_entry_translate::add_flow_entry( cflowentry &fe) {
    cflowentry trans= trans_flow_entry(fe);
    translate.insert(std::pair<cflowentry ,cflowentry>(fe,trans));
    untranslate.insert(std::pair<cflowentry ,cflowentry>(trans,fe));
}

void flow_entry_translate::del_flow_entry(cflowentry &fe) {
    std::map<cflowentry ,  cflowentry>::iterator it;
    std::map<cflowentry ,  cflowentry>::iterator it2;
    it= translate.find(fe);
    cflowentry trans= translate.at(fe);
    it2= untranslate.find(trans);
    translate.erase(it);
    untranslate.erase(it2);
    
}

cflowentry flow_entry_translate::trans_flow_entry(cflowentry &fe) {
    return fe;
}

cflowentry flow_entry_translate::untrans_flow_entry(cflowentry &fe) {
    return untranslate.at(fe);
}
