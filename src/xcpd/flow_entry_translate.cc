/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "flow_entry_translate.h"
#include <rofl/common/cerror.h>
#include <rofl/common/utils/c_logger.h>


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

flow_entry_translate::flow_entry_translate(morpheus * const m)
{
    m_parent= m;
    translate= std::vector < cflowentry >();
    untranslate= std::vector < cflowentry >();
}

rofl::cflowentry flow_entry_translate::get_flowentry_from_msg
    (rofl::cofmsg_flow_mod * const msg)
{
    // TODO -- other versions
    rofl::cflowentry entry(OFP10_VERSION);
    
	entry.set_command(msg->get_command());
	entry.set_idle_timeout(msg->get_idle_timeout());
	entry.set_hard_timeout(msg->get_hard_timeout());
	entry.set_cookie(msg->get_cookie());
	entry.set_priority(msg->get_priority());
	entry.set_buffer_id(msg->get_buffer_id());
	entry.set_out_port(msg->get_out_port());	
	entry.set_flags(msg->get_flags());
	entry.match = msg->get_match();
	entry.actions = msg->get_actions();
    return entry;
}


void flow_entry_translate::add_flow_entry( cflowentry &fe, cflowentry &trans) { 
    translate.push_back(fe);
    untranslate.push_back(trans);
}

void flow_entry_translate::del_flow_entry(cflowentry &fe) {
    unsigned int i;
    for (i= 0; i < translate.size(); i++) {
        if (match_fe(translate[i],fe)) {
            break;
        }
    }
    if (i == translate.size()) {
        ROFL_ERR
        ("Unable to find translation match for flow entry %s in %s\n", 
        fe.c_str(),__PRETTY_FUNCTION__);
        throw rofl::eInval();
    }
    translate.erase (translate.begin()+i);
    untranslate.erase (untranslate.begin()+i);
}

cflowentry flow_entry_translate::trans_flow_entry(cflowentry &fe) {

    rofl::cflowentry entry(fe);
    entry.actions= trans_actions(fe.actions, fe.match);
    entry.actions= fe.actions;
	entry.match = trans_match(fe.match);
    //entry.match= fe.match;
	ROFL_DEBUG("%s: Sending flow mod %s\n",__PRETTY_FUNCTION__,
        entry.c_str());
    return entry;
}



rofl::cofaclist flow_entry_translate::trans_actions(
    rofl::cofaclist  &inlist, rofl::cofmatch &match)
{
    const cportvlan_mapper & mapper = m_parent->get_mapper();
    rofl::cofaclist outlist= rofl::cofaclist(OFP10_VERSION);
//	bool already_set_vlan = false;
	bool already_did_output = false;
// now translate the action and the match
	for(rofl::cofaclist::iterator a = inlist.begin(); a != inlist.end(); ++ a) {
		uint32_t supported_actions = m_parent->get_supported_actions();
		ROFL_DEBUG("%s supported actions by underlying switch found to be: %s.\n",
            __PRETTY_FUNCTION__,action_mask_to_string(supported_actions).c_str());
		if( ! ((1<<(be16toh(a->oac_header->type))) & supported_actions )) {
			// the action isn't supported by the underlying switch - complain - send an error message, then write to your MP. Start a Tea Party movement. Then start an Occupy OpenFlow group. If that still doesn't help become a recluse and blame the system.
			ROFL_DEBUG("%s: Received a flow-mod with an unsupported action: %s %d\n",
             action_mask_to_string((1<<(be16toh(a->oac_header->type)))).c_str(),
             (1<<(be16toh(a->oac_header->type))));
			throw rofl::eInval();
		}
		ROFL_DEBUG("%s: Processing incoming action %s.\n",
            __PRETTY_FUNCTION__,
            action_mask_to_string(1<<(be16toh(a->oac_header->type))).c_str());
		switch(be16toh(a->oac_header->type)) {
			case OFP10AT_OUTPUT: {
				uint16_t oport = be16toh(a->oac_10output->port);
				if ( ( oport == OFPP10_FLOOD ) || ( oport == OFPP10_ALL) ) {	// TODO check that the match isn't ALL
					// ALL is all except input port
					// FLOOD is all except input port and those disabled by STP.. which we don't support anyway - so I'm going to treat them the same way.
					// we need to generate a list of untagged output actions, then a list of tagged output actions for all interfaces except the input interface.
					rofl::cofaclist taggedoutputs;
					for(oport = 1; oport <= mapper.get_number_virtual_ports(); ++oport) {
						cportvlan_mapper::port_spec_t outport_spec = mapper.get_actual_port( oport );
						if(outport_spec.port == match.get_in_port()) {
							// this is the input port - skipping
							continue;
						}
						ROFL_DEBUG("Generating output action from virtual port %ld to actual %ld\n", oport, outport_spec.port);
						if(outport_spec.vlanid_is_none())
							outlist.next() =  rofl::cofaction_output( OFP10_VERSION, outport_spec.port, be16toh(a->oac_10output->max_len) );
						else {
							taggedoutputs.next() = rofl::cofaction_set_vlan_vid( OFP10_VERSION, outport_spec.vlan);
							taggedoutputs.next() = rofl::cofaction_output( OFP10_VERSION, outport_spec.port, be16toh(a->oac_10output->max_len) );
						}
					}
					for(rofl::cofaclist::iterator toi = taggedoutputs.begin(); toi != taggedoutputs.end(); ++toi) outlist.next() = *toi;
					if(taggedoutputs.begin() != taggedoutputs.end()) outlist.next() = rofl::cofaction_strip_vlan( OFP10_VERSION );	// the input output action may not be the last such action so we need to clean up the VLAN that we've left on the "stack"
				} else {	// not FLOOD
					if(oport > mapper.get_number_virtual_ports() ) {
						// invalid virtual port number
                        ROFL_DEBUG ("%s invalid virtual port %d.", 
                            __PRETTY_FUNCTION__, oport);
						 rofl::eInval();
					}
					cportvlan_mapper::port_spec_t real_port = mapper.get_actual_port( oport );
					if(!real_port.vlanid_is_none()) {	// add a vlan tagger before an output if necessary
						outlist.next() = rofl::cofaction_set_vlan_vid( OFP10_VERSION, real_port.vlan );
//						already_set_vlan = true;
					}
					outlist.next() =  rofl::cofaction_output( OFP10_VERSION, real_port.port, be16toh(a->oac_10output->max_len) );	// add translated output action
				}
				already_did_output = true;
			} break;
			case OFP10AT_SET_VLAN_VID: {
				// VLAN-in-VLAN is not supported - return with error.
                ROFL_DEBUG ("%s requires VLAN-in-VLAN not in OF1.0.", __PRETTY_FUNCTION__);
                throw rofl::eInval();
			} break;
			case OFP10AT_SET_VLAN_PCP: {
				// VLAN-in-VLAN is not supported - return with error.
                ROFL_DEBUG ("%s requires VLAN-in-VLAN not in OF1.0.", __PRETTY_FUNCTION__);
				throw rofl::eInval();
			} break;
			case OFP10AT_STRIP_VLAN: {
				if(already_did_output) {
					ROFL_ERR ("%s attempt was made to strip VLAN after an OFP10AT_OUTPUT action. Rejecting flow-mod.", __PRETTY_FUNCTION__);
				}
                throw rofl::eInval();
			} break;
			case OFP10AT_ENQUEUE: {
				// Queues not supported for now.
                ROFL_DEBUG ("%s attempt was made to set queues in flowmod.", __PRETTY_FUNCTION__);
				throw rofl::eInval();
			} break;
			case OFP10AT_SET_DL_SRC:
			case OFP10AT_SET_DL_DST:
			case OFP10AT_SET_NW_SRC:
			case OFP10AT_SET_NW_DST:
			case OFP10AT_SET_NW_TOS:
			case OFP10AT_SET_TP_SRC:
			case OFP10AT_SET_TP_DST: {
				// just pass the message through
				outlist.next() = *a;
			} break;
			case OFP10AT_VENDOR:
				// We have no idea what could be in the vendor message, so we can't translate, so we kill it. 
                
                ROFL_DEBUG("%s Vendor actions are unsupported. Sending error and dropping message.",
				__PRETTY_FUNCTION__);
                throw rofl::eInval();
                break;
			default: 
                
                ROFL_ERR("%s unknown action type (%d) Sending error and dropping message.\n",
                __PRETTY_FUNCTION__,
                (unsigned)be16toh(a->oac_header->type));
                throw rofl::eInval();
                break;
            
		}
    }
    return outlist;
}
    
rofl::cofmatch flow_entry_translate::trans_match(
    rofl::cofmatch &oldmatch) 
{
    const cportvlan_mapper & mapper = m_parent->get_mapper();
	rofl::cofmatch newmatch= oldmatch;
	//check that VLANs are wildcarded (i.e. not being matched on)
	// TODO we *could* theoretically support incoming VLAN iff they are coming in on an port-translated-only port (i.e. a virtual port that doesn't map to a port+vlan, only a phsyical port), and that VLAN si then stripped in the action.
	try {
		oldmatch.get_vlan_vid_mask();	
        ROFL_DEBUG("%s: Received a match which didn't have VLAN wildcarded. Sending error and dropping message. match: %s\n", __PRETTY_FUNCTION__,
            oldmatch.c_str());
       throw rofl::eInval();
	} catch ( rofl::eOFmatchNotFound & ) {
		// do nothing - there was no vlan_vid_mask
	}
	// make sure this is a valid port
	// TODO check whether port is ANY/ALL
    uint32_t old_inport= 0;
	try {
        old_inport = newmatch.get_in_port();
    } catch (rofl::cerror &e) {
         ROFL_DEBUG("%s: caught error %s: %s\n",__PRETTY_FUNCTION__, 
         typeid(e).name(),
         e.desc.c_str());
    }
	try {
        std::cout << "GET PORT " << old_inport << std::endl;
		cportvlan_mapper::port_spec_t real_port = mapper.get_actual_port( old_inport ); // could throw std::out_of_range
        
		if(!real_port.vlanid_is_none()) {
			// vlan is set in actual port - update the match
            std::cout << "SET VLAN " << real_port.vlan << std::endl;
			newmatch.set_vlan_vid( real_port.vlan );
		}
		// update port
        std::cout << "SET PORT " << real_port.port << std::endl;
		newmatch.set_in_port( real_port.port );
	} catch (std::out_of_range &) {
		ROFL_DEBUG("%s: received a match request for an unknown port (%d) There are %d ports.  Sending error and dropping message. match:%s \n", 
            oldmatch.get_in_port(), mapper.get_number_virtual_ports() , oldmatch.c_str());
        throw rofl::eInval();
	}
    std::cout << "TRANS MATCH" << std::endl;
    return newmatch;
}


cflowentry flow_entry_translate::untrans_flow_entry(cflowentry &fe) {
    for (unsigned int i= 0; i < translate.size(); i++) {
        if (match_fe(translate[i],fe)) {
            return untranslate[i];
        }
    }
    ROFL_ERR
    ("Unable to find translation match for flow entry %s in %s\n", 
        fe.c_str(),__PRETTY_FUNCTION__);
    throw rofl::eInval();
}

std::vector <cflowentry> flow_entry_translate::get_translated_matches
    (rofl::cofmatch &match, bool strict) /** Construct a list of flowmods
matching a given match and return their translation */
{
    std::vector <cflowentry> translations=  std::vector <cflowentry>();
    for (unsigned int i= 0; i < translate.size(); i++) {
        if (match.contains(translate[i].match,strict)) {
            translations.push_back(untranslate[i]);
        }
    }
    
    return translations;
}

std::vector <cflowentry> flow_entry_translate::get_translated_matches_and_modify
    (rofl::cofmatch &match, rofl::cofaclist & actions, bool strict) 
    /** Construct a list of flowmods matching a given match and return their translation and modify the action list */
{
    std::vector <cflowentry> translations=  std::vector <cflowentry>();
    for (unsigned int i= 0; i < translate.size(); i++) {
        if (match.contains(translate[i].match,strict)) {
            translate[i].actions= actions;
            untranslate[i].actions= actions;
            translations.push_back(untranslate[i]);
        }
    }
    return translations;
}

std::vector <cflowentry> flow_entry_translate::get_translated_matches_and_delete
    (rofl::cofmatch &match, uint32_t out_port, bool strict) /** Construct a list of flowmods
matching a given match and return their translation while deleting them */
{
    std::vector <cflowentry> translations=  std::vector <cflowentry>();
    std::vector <int> dellist=  std::vector <int>();
    for (unsigned int i= 0; i < translate.size(); i++) {
        if (match.contains(translate[i].match,strict)) {
            switch (out_port) {
                case OFPP10_NONE:
                case OFPP10_ALL:
                case OFPP10_FLOOD:
                    translations.push_back(untranslate[i]);
                    break;
                default:
                    if (out_port == translate[i].get_out_port()) {
                        translations.push_back(untranslate[i]);
                    }
            }
        }
    }
    for (int i= dellist.size()-1; i>= 0; i--) {
        translate.erase(translate.begin()+i);
        untranslate.erase(untranslate.begin()+i);
    }
    return translations;
}
