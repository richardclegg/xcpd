
#include "morpheus.h"
#include <memory>	// for auto_ptr
#include <cassert>
#include <utility>
#include <rofl/common/caddress.h>
#include <rofl/common/crofbase.h>
#include <rofl/common/openflow/cofdpt.h>
#include <rofl/common/openflow/cofctl.h>
#include <rofl/common/openflow/openflow10.h>
#include <rofl/common/thread_helper.h>
#include "cportvlan_mapper.h"
#include <rofl/common/utils/c_logger.h>
#include <typeinfo>
// #include "morpheus_nested.h"
const PV_PORT_T PV_PORT_T::ANY = PV_PORT_T::make_ANY();
const PV_VLANID_T PV_VLANID_T::ANY = PV_VLANID_T::make_ANY();
const PV_VLANID_T PV_VLANID_T::NONE = PV_VLANID_T::make_NONE();
#define PROXYOFPVERSION OFP10_VERSION

std::ostream & operator<< (std::ostream & os, const morpheus & morph) {
	os << "morpheus configuration:\n" << morph.dump_config();
	os << "morpheus current sessions:" << morph.dump_sessions();
	os << "morpheus mappings:" << morph.m_mapper;
	os << std::endl;
	return os;
	}

// print out a hexdump of bytes
void dumpBytes (std::ostream & os, uint8_t * bytes, size_t n_bytes) {
	if (0==n_bytes) return;
	for(size_t i = 0; i < (n_bytes-1); ++i) printf("%02x ", bytes[i]);
	printf("%02x", bytes[n_bytes-1]);
}

std::string morpheus::dump_sessions() const {
	std::stringstream ss;
/*	std::cout << __FUNCTION__ << ": waiting for lock." << std::endl;
	rofl::RwLock lock(&m_sessions_lock, rofl::RwLock::RWLOCK_WRITE);
	std::cout << __FUNCTION__ << ": got lock." << std::endl; */
	for(xid_session_map_t::const_iterator it = m_sessions.begin(); it != m_sessions.end(); ++it)
		ss << "ctl xid " << it->first.first << " dpt xid " << it->first.second << ": " << it->second->asString() << "\n";
//		ss << ((it->first.first)?"ctl":"dpt") << " xid " << it->first.second << ": " << it->second->asString() << "\n";
	return ss.str();
}

std::string morpheus::dump_config() const {	// TODO
	return "";
}

bool morpheus::associate_xid( const uint32_t ctl_xid, const uint32_t dpt_xid, chandlersession_base * p ) {
	std::pair< uint32_t, uint32_t > e ( ctl_xid, dpt_xid );
	xid_session_map_t::iterator sit(m_sessions.find( e ));
	if(sit!=m_sessions.end()) {
        ROFL_ERR("Attempt was made to associate %d/%d with %s but pair exists\n",
            ctl_xid, dpt_xid, p->asString().c_str());
		return false;	// new_xid was already in the database
	}
    ROFL_DEBUG("Associated %d/%d with %s but pair exists\n",
            ctl_xid, dpt_xid, p->asString().c_str());
    return true;
}

// called to remove the association of the xid with a session_base - returns true if session_xid was found and removed, false otherwise
bool morpheus::remove_xid_association( const uint32_t ctl_xid, const uint32_t dpt_xid ) {
	// remove the xid, and check whether the pointed to session_base is associated anywhere else, if it isn't, delete it.
	std::pair< uint32_t, uint32_t > e ( ctl_xid, dpt_xid );
	xid_session_map_t::iterator sit(m_sessions.find(e));
	if(sit==m_sessions.end()) return false;
	m_sessions.erase(sit);

	return true;
}

flow_entry_translate *morpheus::get_fet()
{
    return fet;
}

// called to remove all associations to this session_base - returns the number of associations removed - p is not deleted and reamins in the ownership of the caller
unsigned morpheus::remove_session( chandlersession_base * p ) {
	unsigned tally = 0;
/*	std::cout << __FUNCTION__ << ": waiting for lock." << std::endl;
	rofl::RwLock lock(&m_sessions_lock, rofl::RwLock::RWLOCK_WRITE);
	std::cout << __FUNCTION__ << ": got lock." << std::endl; */
	xid_session_map_t::iterator it = m_sessions.begin();
	xid_session_map_t::iterator old_it;
	while(it!=m_sessions.end()) {
		if(it->second==p) {
			old_it = it++;
			m_sessions.erase(old_it);
			++tally;
			continue;
			}
		else ++it;
	}
//	if(tally) delete(p);
	return tally;
}

void morpheus::set_supported_dpe_features (uint32_t new_capabilities, uint32_t new_actions) {
	// TODO new_capabilities are ignored, befause, well, we don't support any of them.
	// m_supported_features = 0;
	m_dpe_supported_actions = new_actions;
	m_supported_actions = new_actions & m_supported_actions_mask;
	m_dpe_supported_actions_valid = true;
}

uint32_t morpheus::get_supported_actions() {
	if(!m_dpe_supported_actions_valid) {	// for when get_supported_actions is called before set_supported_features
		// we have no information on supported actions from the DPE, so we're going to have to ask ourselves.
//		{ rofl::RwLock lock(&m_session_timers_lock, rofl::RwLock::RWLOCK_WRITE); timeout_opaque_value = register_lifetime_session_timer(max_session_lifetime); }
		std::auto_ptr < morpheus::csh_features_request > s ( new morpheus::csh_features_request ( this ) );
		{ rofl::RwLock lock(&m_session_timers_lock, rofl::RwLock::RWLOCK_WRITE); register_lifetime_session_timer(s.get(), max_session_lifetime); }
		std::cout << __FUNCTION__ << ": sent request for features. Waiting..";
		unsigned wait_time = 5;
		while(wait_time && !s->isCompleted()) {
			sleep(1);	// TODO possible error - is crofbase a single thread, or is every call to a crofbase handler a new thread?
			std::cout << ".";
			--wait_time;
		}
		std::cout << std::endl;
		if(!s->isCompleted()) {
			s.release();
//			throw rofl::eInval(std::string("Request features in morpheus::get_supported_actions but never got a response."));
			throw rofl::eInval();
		}
	}
	return m_supported_actions;
}


morpheus::morpheus(const cportvlan_mapper & mapper_, const bool indpt_, const rofl::caddress dptaddr_, const bool inctl_, const rofl::caddress ctladdr_ ): \
		rofl::crofbase (1 <<  PROXYOFPVERSION),
		m_slave(0),
		m_master(0),
		m_mapper(mapper_),
		m_supported_features(0),
		m_supported_actions_mask( (1<<OFP10AT_OUTPUT)|(1<<OFP10AT_SET_DL_SRC)|(1<<OFP10AT_SET_DL_DST)|(1<<OFP10AT_SET_NW_SRC)|(1<<OFP10AT_SET_NW_DST)|(1<<OFP10AT_SET_NW_TOS)|(1<<OFP10AT_SET_TP_SRC)|(1<<OFP10AT_SET_TP_DST) ),
		m_supported_actions(0),
		m_dpe_supported_actions(0),
		m_dpe_supported_actions_valid(false),
		indpt(indpt_),
		inctl(inctl_),
		dptaddr(dptaddr_),
		ctladdr(ctladdr_),
		m_crof_timer_opaque_offset(0x10000000),
		m_crof_timer_opaque_max(0x00000fff),
//		m_crof_timer_opaque_max(10),
		m_last_crof_timer_opaque(m_crof_timer_opaque_offset+1),
		max_session_lifetime(6),
		m_session_timers(m_crof_timer_opaque_max+1)
		{
    fet= new flow_entry_translate(this);
	// TODO validate actual ports in port map against interrogated ports from DPE? if actual ports aren't available then from the interface as adminisrtatively down?
	pthread_rwlock_init(&m_sessions_lock, 0);
	pthread_rwlock_init(&m_session_timers_lock, 0);
//	init_dpe();
//	std::cout << "Size of m_session_timers is " << m_session_timers.size() << std::endl;
}

morpheus::~morpheus() {
	// rpc_close_all();
    ROFL_DEBUG("%s called.\n",__PRETTY_FUNCTION__);
	pthread_rwlock_destroy(&m_sessions_lock);
	pthread_rwlock_destroy(&m_session_timers_lock);
    delete(fet);
}
/*
void morpheus::init_dpe(){
	if(indpt) rpc_listen_for_dpts(dptaddr);
	else rpc_connect_to_dpt(PROXYOFPVERSION, 5, dptaddr);
}
*/

void morpheus::initialiseConnections(){
	// set up connection to controller (active or passive)
	if(inctl) rpc_listen_for_ctls(ctladdr);
	else rpc_connect_to_ctl(PROXYOFPVERSION,5,ctladdr);
}

rofl::cofdpt * morpheus::get_dpt() const { return m_slave; }
rofl::cofctl * morpheus::get_ctl() const { return m_master; }

void morpheus::handle_error (rofl::cofdpt *src, rofl::cofmsg_error *msg) {
	std::cout << std::endl << "handle_error from " << src->c_str() << " : " << msg->c_str() << std::endl;
	rofl::RwLock lock(&m_sessions_lock, rofl::RwLock::RWLOCK_WRITE);
//	xid_session_map_t::iterator sit(m_sessions.find(std::pair<bool, uint32_t>(false,msg->get_xid())));
	// xid_session_map_t::iterator sit(m_sessions.find(std::pair<bool, uint32_t>(false,msg->get_xid())));
	xid_session_map_t::iterator sit;
	for(sit = m_sessions.begin() ; sit != m_sessions.end(); ++sit) if(sit->first.second==msg->get_xid()) break;
	if(sit!=m_sessions.end()) {
		sit->second->handle_error( src, msg);
	} else std::cout << "**** received error message ( " << msg->c_str() << " ) from dpt ( " << std::hex << src << std::dec << " ) for unknown xid ( " << msg->get_xid() << " ). Dropping." << std::endl;
	delete(msg);
}

void morpheus::handle_ctrl_open (rofl::cofctl *src) {
	// should be called automatically after call to rpc_connect_to_dpt in connect_to_slave
	std::cout << std::endl << "morpheus::handle_ctrl_open called with " << (src?src->c_str():"NULL") << std::endl;
	if (m_master == src) { std::cout << __FUNCTION__ << ": duplicate ctl received. Ignoring." << std::endl; return; }	// this is a fiddle - ROFL send spurious duplicate handle_ctrl_open sometimes.
	m_master = src;	// TODO - what to do with previous m_master?
		try {
			if(indpt) rpc_listen_for_dpts(dptaddr);
			else rpc_connect_to_dpt(PROXYOFPVERSION, 5, dptaddr);
		} catch (rofl::eSocketBindFailed & e) {
			std::cout << __FUNCTION__ << " << caught and ignored rofl::eSocketBindFailed: " << e << std::endl;
		} catch (rofl::cerror & e) {
			std::cout << __FUNCTION__ << " << caught and ignored rofl::cerror: " << e << std::endl;
		} catch (...) {
			std::cout << __FUNCTION__ << " << caught something. " << std::endl;
		}
}

// TODO are all transaction IDs invalidated by a connection reset??
void morpheus::handle_ctrl_close (rofl::cofctl *src) {
	std::cout << "morpheus::handle_ctrl_close called with " << (src?src->c_str():"NULL") << std::endl;
	// controller disconnected - now disconnect from switch.
	rpc_disconnect_from_dpt(m_slave);
	// hopefully dpt will reconnect to us, causing us in turn to reconnect to ctl.
	
	// this socket disconnecting could just be a temporary thing - mark it is dead, but expect a possible auto reconnect
	if(src!=m_master) std::cout << "morpheus::handle_ctrl_close: was expecting " << (m_master?m_master->c_str():"NULL") << " but got " << (src?src->c_str():"NULL") << std::endl;
	m_master=0;	// TODO - m_naster ownership?
	// TODO make attempt to re-establish CTL connection?
}

void morpheus::handle_dpath_open (rofl::cofdpt *src) {
	// should be called automatically after call to rpc_connect_to_dpt in connect_to_slave
	std::cout << std::endl << "morpheus::handle_dpath_open called with " << (src?src->c_str():"NULL") << std::endl;
	m_slave = src;	// TODO - what to do with previous m_slave?
	m_slave_dpid=src->get_dpid();	// TODO - check also get_config, get_capabilities etc
	m_dpid = m_slave_dpid + 1;
/*	if(!m_master){
		if(inctl) rpc_listen_for_ctls(ctladdr);
		else rpc_connect_to_ctl(PROXYOFPVERSION,5,ctladdr);
	} */
}

// TODO are all transaction IDs invalidated by a connection reset??
void morpheus::handle_dpath_close (rofl::cofdpt *src) {
	std::cout << std::endl << "handle_dpath_close called with " << (src?src->c_str():"NULL") << std::endl;
//	assert(src==m_slave);
	if(src!=m_slave) std::cout << "morpheus::handle_dpath_close: Was expecting " << (m_slave?m_slave->c_str():"NULL") << " but got " << (src?src->c_str():"NULL") << std::endl;
	// this socket disconnecting could just be a temporary thing - mark it is dead, but expect a possible auto reconnect
	m_slave=0;	// TODO - m_slave ownership?

	rpc_disconnect_from_ctl(m_master);
//	init_dpe();
// no need - we're still istening, apparently
}


void morpheus::register_lifetime_session_timer(morpheus::chandlersession_base * s, unsigned seconds) {
	int opaque = register_session_timer(s,seconds);
	s->setLifetimeTimerOpaque(opaque);
}

int morpheus::register_session_timer(morpheus::chandlersession_base * s, unsigned seconds) {
// int morpheus::register_session_timer(unsigned seconds) {
	int opaque = m_last_crof_timer_opaque - m_crof_timer_opaque_offset;
	int off;
//	std::cout << "TP" << std::dec << __LINE__ << std::endl;
	for(off = 0; off < m_crof_timer_opaque_max; ++off) {
		if(!m_session_timers[(opaque+off)%m_crof_timer_opaque_max]) { break; }	// found a null entry - it can be our next opaque
	}
//	std::cout << "TP" << std::dec << __LINE__ << std::endl;
	if(off==m_crof_timer_opaque_max) throw std::range_error("Ran out of session timer opaque values.");
	opaque = (opaque+off)%m_crof_timer_opaque_max;
//	std::cout << "TP" << std::dec << __LINE__ << std::endl;
	std::cout << "About to add opaque " << std::hex << opaque << std::dec << " to m_session_timers." << std::endl;
	if(m_session_timers[opaque]) std::cout << "DAFAQ! it's already there!" << std::endl;
	m_session_timers[opaque] = s;
	opaque += m_crof_timer_opaque_offset;
//	std::cout << "TP" << std::dec << __LINE__ << std::endl;
	register_timer(opaque, seconds);
//	std::cout << "TP" << std::dec << __LINE__ << std::endl;
///	s->setLifetimeTimerOpaque(opaque);
	std::cout << "Just registered session " << std::hex << s << " with opaque " << std::hex << opaque << std::dec << std::endl;
	m_last_crof_timer_opaque = opaque;
	return opaque;
}


// this method handles all the timing events - these can be either session lifetime evenets, or some secondary event created by a session.
// ** THIS METHOD IS THE ONE THAT REMOVES AN EXPIRED SESSION FROM m_session_timers and the sessions database.
void morpheus::handle_timeout ( int opaque ) {
	std::cout << "****" << __FUNCTION__ << " called with opaque = " << std::hex << opaque << "/" << (opaque-m_crof_timer_opaque_offset) << std::dec << std::endl;
	// TODO make sure to remove *all* instances of the pointer to the session from the m_session_timers array (scan for value in array, swap with 0, delete whats pointed to by array
	// check that this is a session_timer
	int opaque_off = opaque - m_crof_timer_opaque_offset;
// std::cout << "TP" << std::dec << __LINE__ << std::endl;
	rofl::RwLock session_timers_lock(&m_session_timers_lock, rofl::RwLock::RWLOCK_WRITE);
	if(!m_session_timers[opaque_off]) {
		std::cout << __FUNCTION__ << " called with unknwon opaque " << std::hex << opaque << std::dec << "." << std::endl;
		assert(false);
		}
// std::cout << "TP" << std::dec << __LINE__ << std::endl;
	morpheus::chandlersession_base * s = m_session_timers[opaque_off];

//	std::cout << " Recovered opaque offset " << opaque_off << " as (" << std::hex << s << std::dec << ")" << std::endl;
	if(s->getLifetimeTimerOpaque()==opaque) {
		// this is a session lifetime timer - find all mentions of this session and delete it.
		
		// first remove from sessions_timer
		std::replace(m_session_timers.begin(), m_session_timers.end(), s, (morpheus::chandlersession_base *) 0);
		// now remove from sessions database and delete
		if(!s->isCompleted()) {
			std::cout << "ERROR: timeout occured on session " << std::hex << s << std::dec << " (" << typeid(*s).name() << ") which wasn't completed." << std::endl;
			// TODO send error?
			}
		rofl::RwLock lock(&m_sessions_lock, rofl::RwLock::RWLOCK_WRITE);
		std::cout << "About to delete session " << s->asString() << std::endl;
//		remove_session(s);
//		std::cout << "Session removed." << std::endl;
		delete(s);
	} else {
		// this is a timer belonging to some other session function, so tell the session
//		std::cout << "About to call s->handle_timeout on " << s->asString() << std::endl;
		s->handle_timeout(opaque);
//		std::cout << "After s->handle_timeout." << std::endl;
	}
//	std::cout << __FUNCTION__ << ": ~FIN~ " << std::endl;
}

#undef STRINGIFY
#undef TOSTRING
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

// this is from a ctl if CTL_DPT is true, false otherwise
#define HANDLE_REQUEST_WITH_REPLY_TEMPLATE(CTL_DPT, MSG_TYPE, SESSION_TYPE) { \
	std::cout << std::endl << func << " from " << src->c_str() << " : " << msg->c_str() << std::endl; \
	if((!m_slave)||(!m_master)) { std::cout << "Dropping message due to lack of CTL/DPT connectivity." << std::endl; delete(msg); return; } \
	rofl::RwLock session_lock(&m_sessions_lock, rofl::RwLock::RWLOCK_WRITE); \
	rofl::RwLock session_timers_lock(&m_session_timers_lock, rofl::RwLock::RWLOCK_WRITE); \
	try { \
		SESSION_TYPE * s = new SESSION_TYPE ( this, src, msg ); /* this isn't a leak as the constructor of the session should register the xids and the pointer will stay in the  */ \
		if(!s) {} \
	} catch(rofl::cerror &e) { std::cout << "unhandled rofl::cerror: " << e.desc << std::endl; assert(false); } \
	catch (...) { std::cout << __FUNCTION__ << ": unhandled exception"; assert(false); } \
	delete(msg); \
}

#define HANDLE_REPLY_AFTER_REQUEST_TEMPLATE(CTL_DPT, MSG_TYPE, SESSION_TYPE, REPLY_FN) { \
	std::cout << std::endl << func << " from " << src->c_str() << " : " << msg->c_str() << std::endl; \
	rofl::RwLock session_lock(&m_sessions_lock, rofl::RwLock::RWLOCK_WRITE); \
	rofl::RwLock session_timers_lock(&m_session_timers_lock, rofl::RwLock::RWLOCK_WRITE); \
	if(CTL_DPT) { std::cout << "Unhandled case in " << __FUNCTION__ << std::endl; /* no messages do this */ } else { \
	xid_session_map_t::iterator it; \
	for(it = m_sessions.begin() ; it != m_sessions.end(); ++it) if(it->first.second==msg->get_xid()) break; \
	if(it == m_sessions.end()) { std::cout << func << ": Unexpected " << TOSTRING(MSG_TYPE) << " received with xid " << msg->get_xid() << ". Dropping new message." << std::endl; return; } \
	if((!m_slave)||(!m_master)) { std::cout << "Dropping message due to lack of CTL/DPT connectivity." << std::endl; delete(msg); return; } \
	SESSION_TYPE * s = dynamic_cast<SESSION_TYPE *>(it->second); \
	if(!s) { std::cout << func << ": xid (" << msg->get_xid() << ") maps to existing session of wrong type." << std::endl; } \
	try { \
		s->REPLY_FN( src, msg ); \
		if(s->isCompleted()) { remove_session(s); delete(s); } \
	} catch(rofl::cerror &e) { std::cout << "unhandled rofl::cerror: " << e.desc << std::endl; assert(false); } \
	catch (...) { std::cout << __FUNCTION__ << ": unhandled exception"; assert(false); } \
	delete(msg); \
	} \
}

#define HANDLE_MESSAGE_FORWARD_TEMPLATE(CTL_DPT, SESSION_TYPE) { \
	std::cout << std::endl << func << " from " << src->c_str() << " : " << msg->c_str() << "." << std::endl; \
	if((!m_slave)||(!m_master)) { std::cout << "Dropping message due to lack of CTL/DPT connectivity." << std::endl; delete(msg); return; } \
	try { \
		SESSION_TYPE * s = new SESSION_TYPE ( this, src, msg ); \
		{ rofl::RwLock session_timers_lock(&m_session_timers_lock, rofl::RwLock::RWLOCK_WRITE); register_lifetime_session_timer(s, max_session_lifetime); } /* timer takes ownership of session */ \
	} catch(rofl::cerror &e) { std::cout << "unhandled rofl::cerror: " << e.desc << std::endl; assert(false); } \
	catch (...) { std::cout << __FUNCTION__ << ": unhandled exception"; assert(false); } \
	delete(msg); \
}


void morpheus::handle_flow_mod(rofl::cofctl * src, rofl::cofmsg_flow_mod *msg) {
	static const char * func = __FUNCTION__;
	HANDLE_MESSAGE_FORWARD_TEMPLATE(true, morpheus::csh_flow_mod)
//	HANDLE_MESSAGE_FORWARD_TEMPLATE(true, morpheus::DEBUG_csh_flow_mod)
}
void morpheus::handle_features_request(rofl::cofctl *src, rofl::cofmsg_features_request * msg ) {
	static const char * func = __FUNCTION__;
	HANDLE_REQUEST_WITH_REPLY_TEMPLATE( true, cofmsg_features_request, morpheus::csh_features_request )
}
void morpheus::handle_features_reply(rofl::cofdpt * src, rofl::cofmsg_features_reply * msg ) {
	static const char * func = __FUNCTION__;
	HANDLE_REPLY_AFTER_REQUEST_TEMPLATE( false, cofmsg_features_reply, morpheus::csh_features_request, process_features_reply )
}

void morpheus::handle_get_config_request(rofl::cofctl *src, rofl::cofmsg_get_config_request *msg) {
	static const char * func = __FUNCTION__;
	HANDLE_REQUEST_WITH_REPLY_TEMPLATE( true, cofmsg_get_config_request, morpheus::csh_get_config )
}
void morpheus::handle_get_config_reply(rofl::cofdpt * src, rofl::cofmsg_get_config_reply * msg ) {
	static const char * func = __FUNCTION__;
	HANDLE_REPLY_AFTER_REQUEST_TEMPLATE( false, cofmsg_get_config_reply, morpheus::csh_get_config, process_config_reply )
}

void morpheus::handle_desc_stats_request(rofl::cofctl *src, rofl::cofmsg_desc_stats_request *msg) {
	static const char * func = __FUNCTION__;
	HANDLE_REQUEST_WITH_REPLY_TEMPLATE( true, cofmsg_desc_stats_request, morpheus::csh_desc_stats )
}
void morpheus::handle_desc_stats_reply(rofl::cofdpt * src, rofl::cofmsg_desc_stats_reply * msg) {
	static const char * func = __FUNCTION__;
	HANDLE_REPLY_AFTER_REQUEST_TEMPLATE( false, cofmsg_desc_stats_reply, morpheus::csh_desc_stats, process_desc_stats_reply )
}

void morpheus::handle_port_stats_request(rofl::cofctl *src, rofl::cofmsg_port_stats_request *msg) {
	static const char * func = __FUNCTION__;
	HANDLE_REQUEST_WITH_REPLY_TEMPLATE( true, cofmsg_port_stats_request, morpheus::csh_port_stats )
}
void morpheus::handle_port_stats_reply(rofl::cofdpt * src, rofl::cofmsg_port_stats_reply * msg) {
	static const char * func = __FUNCTION__;
	HANDLE_REPLY_AFTER_REQUEST_TEMPLATE( false, cofmsg_port_stats_reply, morpheus::csh_port_stats, process_port_stats_reply )
}



void morpheus::handle_set_config(rofl::cofctl *src, rofl::cofmsg_set_config *msg) {
	static const char * func = __FUNCTION__;
	HANDLE_MESSAGE_FORWARD_TEMPLATE(true, morpheus::csh_set_config)
}

void morpheus::handle_table_stats_request(rofl::cofctl *src, rofl::cofmsg_table_stats_request *msg) {
	static const char * func = __FUNCTION__;
	HANDLE_REQUEST_WITH_REPLY_TEMPLATE( true, cofmsg_table_stats_request, morpheus::csh_table_stats )
}
void morpheus::handle_table_stats_reply(rofl::cofdpt *src, rofl::cofmsg_table_stats_reply *msg) {
	static const char * func = __FUNCTION__;
	HANDLE_REPLY_AFTER_REQUEST_TEMPLATE( false, cofmsg_table_stats_reply, morpheus::csh_table_stats, process_table_stats_reply )
}

void morpheus::handle_aggregate_stats_request(rofl::cofctl *src, rofl::cofmsg_aggr_stats_request *msg) {
	static const char * func = __FUNCTION__;
	HANDLE_REQUEST_WITH_REPLY_TEMPLATE( true, cofmsg_aggr_stats_request, morpheus::csh_aggregate_stats )
}

void morpheus::handle_aggregate_stats_reply(rofl::cofdpt *src, rofl::cofmsg_aggr_stats_reply *msg) {
	static const char * func = __FUNCTION__;
	HANDLE_REPLY_AFTER_REQUEST_TEMPLATE( false, cofmsg_aggr_stats_reply, morpheus::csh_aggregate_stats, process_aggr_stats_reply )
}

void morpheus::handle_packet_in(rofl::cofdpt *src, rofl::cofmsg_packet_in * msg) {
	static const char * func = __FUNCTION__;
	HANDLE_MESSAGE_FORWARD_TEMPLATE(false, morpheus::csh_packet_in)
}
void morpheus::handle_packet_out(rofl::cofctl *src, rofl::cofmsg_packet_out *msg) {
	static const char * func = __FUNCTION__;
	HANDLE_MESSAGE_FORWARD_TEMPLATE(true, morpheus::csh_packet_out)
}

void morpheus::handle_barrier_request(rofl::cofctl *src, rofl::cofmsg_barrier_request *msg) {
	static const char * func = __FUNCTION__;
	HANDLE_REQUEST_WITH_REPLY_TEMPLATE( true, cofmsg_barrier_request, morpheus::csh_barrier )
}
void morpheus::handle_barrier_reply ( rofl::cofdpt * src, rofl::cofmsg_barrier_reply * msg) {
	static const char * func = __FUNCTION__;
	HANDLE_REPLY_AFTER_REQUEST_TEMPLATE( false, cofmsg_barrier_reply, morpheus::csh_barrier, process_barrier_reply )
}

void morpheus::handle_flow_stats_request(rofl::cofctl *src, rofl::cofmsg_flow_stats_request *msg) {
// see ./examples/etherswitch/etherswitch.cc:95
	static const char * func = __FUNCTION__;
//	std::cout << std::endl << func << " from " << src->c_str() << " : " << msg->c_str() << std::endl;
//	delete(msg);
	HANDLE_REQUEST_WITH_REPLY_TEMPLATE( true, cofmsg_flow_stats_request, morpheus::csh_flow_stats );
}

void morpheus::handle_flow_stats_reply(rofl::cofdpt *src, rofl::cofmsg_flow_stats_reply *msg) {
	static const char * func = __FUNCTION__;
    
	HANDLE_REPLY_AFTER_REQUEST_TEMPLATE( false, cofmsg_flow_stats_reply, morpheus::         csh_flow_stats, process_flow_stats_reply );
    HANDLE_REPLY_AFTER_REQUEST_TEMPLATE( false, cofmsg_flow_stats_reply, morpheus::         csh_aggregate_stats, process_flow_stats_reply );
}

void morpheus::handle_queue_stats_request(rofl::cofctl *src, rofl::cofmsg_queue_stats_request *msg) {
	static const char * func = __FUNCTION__;
	std::cout << std::endl << func << " from " << src->c_str() << " : " << msg->c_str() << std::endl;
	delete(msg);
}
void morpheus::handle_experimenter_stats_request(rofl::cofctl *src, rofl::cofmsg_stats_request *msg) {
	static const char * func = __FUNCTION__;
	std::cout << std::endl << func << " from " << src->c_str() << " : " << msg->c_str() << std::endl;
	delete(msg);
}

void morpheus::handle_port_mod(rofl::cofctl *src, rofl::cofmsg_port_mod *msg) {
	static const char * func = __FUNCTION__;
	HANDLE_MESSAGE_FORWARD_TEMPLATE(true, morpheus::csh_port_mod)
}
void morpheus::handle_queue_get_config_request(rofl::cofctl *src, rofl::cofmsg_queue_get_config_request *msg) {
	static const char * func = __FUNCTION__;
	std::cout << std::endl << func << " from " << src->c_str() << " : " << msg->c_str() << std::endl;
	delete(msg);
}
void morpheus::handle_experimenter_message(rofl::cofctl *src, rofl::cofmsg_features_request *msg) {
	static const char * func = __FUNCTION__;
	std::cout << std::endl << func << " from " << src->c_str() << " : " << msg->c_str() << std::endl;
	delete(msg);
}

std::string action_mask_to_string(const uint32_t action_types) {
	std::string out;
	static const uint32_t vals[] = { OFP10AT_OUTPUT, OFP10AT_SET_VLAN_VID, OFP10AT_SET_VLAN_PCP, OFP10AT_STRIP_VLAN, OFP10AT_SET_DL_SRC, OFP10AT_SET_DL_DST, OFP10AT_SET_NW_SRC, OFP10AT_SET_NW_DST, OFP10AT_SET_NW_TOS, OFP10AT_SET_TP_SRC, OFP10AT_SET_TP_DST, OFP10AT_ENQUEUE };
	static const std::string names[] = { "OFP10AT_OUTPUT", "OFP10AT_SET_VLAN_VID", "OFP10AT_SET_VLAN_PCP", "OFP10AT_STRIP_VLAN", "OFP10AT_SET_DL_SRC", "OFP10AT_SET_DL_DST", "OFP10AT_SET_NW_SRC", "OFP10AT_SET_NW_DST", "OFP10AT_SET_NW_TOS", "OFP10AT_SET_TP_SRC", "OFP10AT_SET_TP_DST", "OFP10AT_ENQUEUE" };
	static const size_t N = sizeof(vals)/sizeof(vals[0]);
	for(size_t i=0;i<N;++i) {
		if(action_types & (1<<vals[i])) out += names[i] + " ";
	}
	return out;
}

std::string port_as_string(uint16_t p) {	// maps a ofp10_port_no port number to a string if it has a meaning (like FLOOD), or just to a number.
	static const uint32_t vals[] = { OFPP10_IN_PORT, OFPP10_TABLE, OFPP10_NORMAL, OFPP10_FLOOD, OFPP10_ALL, OFPP10_CONTROLLER, OFPP10_LOCAL, OFPP10_NONE };
	static const std::string names[] = { "OFPP10_IN_PORT", "OFPP10_TABLE", "OFPP10_NORMAL", "OFPP10_FLOOD", "OFPP10_ALL", "OFPP10_CONTROLLER", "OFPP10_LOCAL", "OFPP10_NONE" };
	static const size_t N = sizeof(vals)/sizeof(vals[0]);
	for(size_t i=0;i<N;++i) {
		if(vals[i]==p) return names[i];
	}
	std::stringstream s;
	if(p <= OFPP10_MAX) s << "PORT_NO:";
	else s << "INVALID_PORT_NO:";
	s << p;
	return s.str();
	}

std::string capabilities_to_string(uint32_t capabilities) {
	static const std::string capabilities_sz [] = { "OFPC_FLOW_STATS", "OFPC_TABLE_STATS", "OFPC_PORT_STATS", "OFPC_STP", "OFPC_RESERVED", "OFPC_IP_REASM", "OFPC_QUEUE_STATS", "OFPC_ARP_MATCH_IP" };
	std::string out;
	for(size_t index=0; capabilities!=0; ++index, capabilities >>= 1) if(capabilities & 0x00000001) out += capabilities_sz[index] + " ";
	return out;
}



