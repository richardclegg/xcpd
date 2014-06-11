
#include <rofl/common/openflow/openflow_rofl_exceptions.h>
#include <rofl/common/openflow/openflow10.h>
#include <rofl/common/cerror.h>
#include <rofl/common/openflow/cofaction.h>
#include <rofl/common/utils/c_logger.h>

#include "csh_flow_mod.h"

morpheus::csh_flow_mod::csh_flow_mod(morpheus * parent, rofl::cofctl * const src, rofl::cofmsg_flow_mod * const msg ):chandlersession_base(parent, msg->get_xid()) {
	ROFL_DEBUG("%s called.\n", __PRETTY_FUNCTION__);
	process_flow_mod(src, msg);
}

bool morpheus::csh_flow_mod::process_flow_mod ( rofl::cofctl * const src, rofl::cofmsg_flow_mod * const msg ) {
	if(msg->get_version() != OFP10_VERSION) throw rofl::eBadVersion();
	ROFL_DEBUG("Incoming msg match: %s actions %s\n",
         msg->get_match().c_str(), msg->get_actions().c_str());
    switch (msg->get_command()) {
        case OFPFC_ADD:
            m_completed= morpheus::csh_flow_mod::process_add_flow(src,msg);
            return m_completed;
        case OFPFC_MODIFY:
            m_completed= morpheus::csh_flow_mod::process_modify_flow(src,msg);
            return m_completed;
        case OFPFC_MODIFY_STRICT:
            m_completed= morpheus::csh_flow_mod::process_modify_strict_flow(src,msg);
            return m_completed;
        case OFPFC_DELETE:
            m_completed= morpheus::csh_flow_mod::process_delete_flow(src,msg);
            return m_completed;
        case OFPFC_DELETE_STRICT:
            m_completed= morpheus::csh_flow_mod::process_delete_strict_flow(src,msg);
            return m_completed;           
        default:
            m_completed= true;
            return m_completed;
    }
    ROFL_ERR ("FLOW_MOD command %s not supported. Dropping message\n",
            msg->c_str());
    m_completed = true;
    return m_completed;
}
    

rofl::cflowentry morpheus::csh_flow_mod::get_flowentry_from_msg
    (rofl::cofmsg_flow_mod * const msg)
{
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

bool morpheus::csh_flow_mod::process_add_flow
    ( rofl::cofctl * const src, rofl::cofmsg_flow_mod * const msg )
{

    rofl::cflowentry entry= get_flowentry_from_msg(msg);
    rofl::cflowentry trans(OFP10_VERSION);
    try {
        trans= m_parent->get_fet()->trans_flow_entry(entry);
        // TODO check for overlap
        m_parent->get_fet()->add_flow_entry(entry,trans);
        m_parent->send_flow_mod_message( m_parent->get_dpt(), trans);
        return false;
    } catch (rofl::eInval &e) {
        m_parent->send_error_message( src, msg->get_xid(),  OFP10ET_FLOW_MOD_FAILED, OFP10FMFC_UNSUPPORTED, msg->soframe(), 
            msg->framelen() );
        return true;
    }
}

bool morpheus::csh_flow_mod::process_modify_flow
    ( rofl::cofctl * const src, rofl::cofmsg_flow_mod * const msg )
{
    std::vector <cflowentry> translations= 
        m_parent->get_fet()->get_translated_matches_and_modify
            (msg->get_match(),msg->get_actions(),false);
    // Without a flow to modify then mod acts like an add
    if (translations.size() == 0) {
        return process_add_flow(src,msg);
    }
    // Otherwise loop around and do a modify
    for (unsigned int i= 0; i < translations.size(); i++) {
        cflowentry cfe= get_flowentry_from_msg(msg);
        cfe.match= translations[i].match;
        m_parent->send_flow_mod_message( m_parent->get_dpt(), cfe);
    }
    return false;
}

bool morpheus::csh_flow_mod::process_modify_strict_flow
    ( rofl::cofctl * const src, rofl::cofmsg_flow_mod * const msg )
{
    std::vector <cflowentry> translations= 
        m_parent->get_fet()->get_translated_matches_and_modify
            (msg->get_match(),msg->get_actions(),true);
    // Without a flow to modify then mod acts like an add
    if (translations.size() == 0) {
        return process_add_flow(src,msg);
    }
    if (translations.size() != 1) {
        ROFL_ERR ("FLOW_MOD strict modify should not match more than one\n",
            msg->c_str());
    }
    // Otherwise loop around and do a modify
    for (unsigned int i= 0; i < translations.size(); i++) {
        cflowentry cfe= get_flowentry_from_msg(msg);
        cfe.match= translations[i].match;
        m_parent->send_flow_mod_message( m_parent->get_dpt(), cfe);
    }
    return false;
}

bool morpheus::csh_flow_mod::process_delete_flow
    ( rofl::cofctl * const src, rofl::cofmsg_flow_mod * const msg )
{
    std::vector <cflowentry> translations= 
        m_parent->get_fet()->get_translated_matches_and_delete
            (msg->get_match(),msg->get_out_port(),false);
    
    // Otherwise loop around and send a delete
    for (unsigned int i= 0; i < translations.size(); i++) {
        cflowentry cfe= get_flowentry_from_msg(msg);
        cfe.match= translations[i].match;
        m_parent->send_flow_mod_message( m_parent->get_dpt(), cfe);
    }
    return false;
}

bool morpheus::csh_flow_mod::process_delete_strict_flow
    ( rofl::cofctl * const src, rofl::cofmsg_flow_mod * const msg )
{
    std::vector <cflowentry> translations= 
        m_parent->get_fet()->get_translated_matches_and_delete
            (msg->get_match(),msg->get_out_port(),true);
    if (translations.size() != 1) {
        ROFL_ERR ("FLOW_MOD strict delete should not match more than one\n",
            msg->c_str());
    }    
    // Otherwise loop around and send a delete
    for (unsigned int i= 0; i < translations.size(); i++) {
        cflowentry cfe= get_flowentry_from_msg(msg);
        cfe.match= translations[i].match;
        m_parent->send_flow_mod_message( m_parent->get_dpt(), cfe);
    }
    return false;
    return true;
}

bool morpheus::csh_flow_mod::handle_error (rofl::cofdpt *src, rofl::cofmsg_error *msg) {
	ROFL_DEBUG("Warning: %s has received an error message: %s\n", __PRETTY_FUNCTION__, msg->c_str());
	m_completed = true;
	return m_completed;
}

morpheus::csh_flow_mod::~csh_flow_mod() { 
    ROFL_DEBUG ("%s called.\n", __PRETTY_FUNCTION__);
}


// std::string morpheus::csh_flow_mod::asString() const { return "csh_flow_mod {no xid}"; }
std::string morpheus::csh_flow_mod::asString() const { std::stringstream ss; ss << "csh_flow_mod {request_xid=" << m_request_xid << "}"; return ss.str(); }

