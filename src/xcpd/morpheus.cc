
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
#include "control_manager.h"

// #include "morpheus_nested.h"
const PV_PORT_T PV_PORT_T::ANY = PV_PORT_T::make_ANY();
const PV_VLANID_T PV_VLANID_T::ANY = PV_VLANID_T::make_ANY();
const PV_VLANID_T PV_VLANID_T::NONE = PV_VLANID_T::make_NONE();
#define PROXYOFPVERSION OFP10_VERSION

const int morpheus::PATH_CLOSED= 0;
const int morpheus::PATH_WAIT= 1;
const int morpheus::PATH_OPEN= 2;

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
		m_last_crof_timer_opaque(m_crof_timer_opaque_offset+1),
		max_session_lifetime(6),
		m_session_timers(m_crof_timer_opaque_max+1)
{
    fet= new flow_entry_translate(this);
	pthread_rwlock_init(&m_sessions_lock, 0);
	pthread_rwlock_init(&m_session_timers_lock, 0);
	ctlmsgqueue= std::vector <rofl::cofmsg *> ();
	dptmsgqueue= std::vector <rofl::cofmsg *> ();

    
    dpt_state= PATH_CLOSED;
    ctl_state= PATH_CLOSED;
}

morpheus::~morpheus() {
	// rpc_close_all();
    ROFL_DEBUG("%s:  called.\n",__PRETTY_FUNCTION__);
	pthread_rwlock_destroy(&m_sessions_lock);
	pthread_rwlock_destroy(&m_session_timers_lock);
    delete(fet);
}


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
	for(size_t i = 0; i < (n_bytes-1); ++i) {
		printf("%02x ", bytes[i]);
	}
	printf("%02x", bytes[n_bytes-1]);
}

std::string morpheus::dump_sessions() const {
	std::stringstream ss;
	for(xid_session_map_t::const_iterator it = m_sessions.begin();
		it != m_sessions.end(); ++it)
		ss << "ctl xid " << it->first.first << " dpt xid " << 
			it->first.second << ": " << 
			it->second->asString() << "\n";
	return ss.str();
}

std::string morpheus::dump_config() const {	// TODO
	return "Morpheus Configuration";
}

bool morpheus::associate_xid( const uint32_t ctl_xid, const uint32_t dpt_xid, chandlersession_base * p ) {
	std::pair< uint32_t, uint32_t > e ( ctl_xid, dpt_xid );
	xid_session_map_t::iterator sit(m_sessions.find( e ));
	if(sit!=m_sessions.end()) {
        ROFL_ERR("%s: Attempt was made to associate %ld/%ld with %s but pair exists\n",
            __PRETTY_FUNCTION__,
            ctl_xid, dpt_xid, p->asString().c_str());
		return false;	// new_xid was already in the database
	}
    ROFL_DEBUG("%s: Associated %ld/%ld with %s\n",
			__PRETTY_FUNCTION__,
            ctl_xid, dpt_xid, p->asString().c_str());
    m_sessions[e]=p;
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
	// TODO new_capabilities are ignored, because we don't support any of them.
	// m_supported_features = 0;
	m_dpe_supported_actions = new_actions;
	m_supported_actions = new_actions & m_supported_actions_mask;
	m_dpe_supported_actions_valid = true;
}

uint32_t morpheus::get_supported_actions() {
	if(!m_dpe_supported_actions_valid) {	// for when get_supported_actions is called before set_supported_features
		// we have no information on supported actions from the DPE, so we're going to have to ask ourselves.
		std::auto_ptr < morpheus::csh_features_request > s ( new morpheus::csh_features_request ( this ) );
		{ rofl::RwLock lock(&m_session_timers_lock, rofl::RwLock::RWLOCK_WRITE); register_lifetime_session_timer(s.get(), max_session_lifetime); }
		ROFL_DEBUG("%s: sent request for features. Waiting.\n");
		unsigned wait_time = 5;
		while(wait_time && !s->isCompleted()) {
			sleep(1);	// TODO possible error - is crofbase a single thread, or is every call to a crofbase handler a new thread?
			std::cout << ".";
			--wait_time;
		}
		std::cout << std::endl;
		if(!s->isCompleted()) {
			s.release();
			throw rofl::eInval();
		}
	}
	return m_supported_actions;
}

void morpheus::set_ctl_watcher() {
    if (ctl_state != PATH_CLOSED) {
        ROFL_DEBUG("%s: called but ctl not closed\n");
        return;
    }
    ctl_state= PATH_WAIT;
	if(inctl) {
		ROFL_DEBUG("Connecting to controller %s xcpd in listening mode\n",
			ctladdr.c_str());
		rpc_listen_for_ctls(ctladdr);
	}
	else {
		ROFL_DEBUG("Connecting to controller %s xcpd in active mode\n",
			ctladdr.c_str());
		rpc_connect_to_ctl(PROXYOFPVERSION,1,ctladdr);
	}
	ROFL_DEBUG ("Waiting for control path\n");
}

void morpheus::set_dpt_watcher() {
    if (dpt_state != PATH_CLOSED) {
        ROFL_DEBUG("%s: called but dpt not closed\n",
			__PRETTY_FUNCTION__);
        return;
    }
    dpt_state= PATH_WAIT;
    ROFL_DEBUG("%s called.\n",__PRETTY_FUNCTION__);
	if (!indpt) {
		ROFL_DEBUG("Connecting to switch %s switch in active mode\n",
			dptaddr.c_str());
		rpc_listen_for_dpts(dptaddr);
	} else {
		ROFL_DEBUG("Connecting to switch %s switch in listening mode\n",
			dptaddr.c_str());
		rpc_connect_to_dpt(PROXYOFPVERSION,1,dptaddr);
	}
	ROFL_DEBUG("%s exiting.\n",__PRETTY_FUNCTION__);
}

void morpheus::initialiseConnections(){
	// set up connection to controller (active or passive)
	set_ctl_watcher();
}

rofl::cofdpt * morpheus::get_dpt() const { return m_slave; }
rofl::cofctl * morpheus::get_ctl() const { return m_master; }

void morpheus::handle_error (rofl::cofdpt *src, rofl::cofmsg_error *msg) {
	ROFL_DEBUG("handle_error from %s:%s\n ",src->c_str(),
		msg->c_str());
	rofl::RwLock lock(&m_sessions_lock, rofl::RwLock::RWLOCK_WRITE);
	xid_session_map_t::iterator sit;
	// Look for xid
	for(sit = m_sessions.begin() ; sit != m_sessions.end(); ++sit) {
		if(sit->first.second==msg->get_xid()) 
			break;
	}
	if(sit!=m_sessions.end()) {
		sit->second->handle_error(src, msg);
	} else {
		ROFL_ERR("Received error message (%s) from dpt (%xd) for unknown xid %d.  Dropping!\n",
			msg->c_str(), src, msg->get_xid());
	}
	delete(msg);
}

void morpheus::process_ctlqueue()
{
	for(std::vector<rofl::cofmsg *>::iterator it = ctlmsgqueue.begin();
		it != ctlmsgqueue.end(); ++it) {
		cofmsg_features_request *fr=  dynamic_cast<cofmsg_features_request *> (*it);
		if (fr != 0) {
			handle_features_request(m_master, fr);
			continue;
		}
		ROFL_DEBUG("%s: popping ctl message queue message not handled %s\n", 
			__PRETTY_FUNCTION__,(*it)->c_str());
		delete(*it);
		
	}
	ctlmsgqueue= std::vector<rofl::cofmsg *> ();
}

void morpheus::process_dptqueue()
{
	for(std::vector<rofl::cofmsg *>::iterator it = dptmsgqueue.begin();
		it != dptmsgqueue.end(); ++it) {
		
		cofmsg_get_config_reply *cr= dynamic_cast<cofmsg_get_config_reply *> (*it);
		if (cr != 0) {
			handle_get_config_reply(m_slave,cr);
			continue;
		}
		cofmsg_features_reply *fr=  dynamic_cast<cofmsg_features_reply *> (*it);
		if (fr != 0) {
			handle_features_reply(m_slave, fr);
			continue;
		}
		ROFL_DEBUG("%s: popping dpt message queue -- message not handled %s\n", 
			__PRETTY_FUNCTION__,(*it)->c_str());
		delete(*it);
	}
	dptmsgqueue= std::vector<rofl::cofmsg *> ();
}


void morpheus::handle_ctrl_open (rofl::cofctl *src) {
	ROFL_DEBUG("%s called with %s\n",__PRETTY_FUNCTION__,
		(src?src->c_str():"NULL"));
    ctl_state= PATH_OPEN;
	if (m_master == src) { 
		ROFL_DEBUG("%s : received contact from old controller, keeping it.\n",
			__PRETTY_FUNCTION__);
		return;
	}	
	else if (m_master != 0) {
		ROFL_DEBUG("%s : received replacement controller.\n");
		process_ctlqueue();
		return;
	}
	m_master= src;
	try {
		if (m_slave == 0) {
			set_dpt_watcher();
		}
	} catch (rofl::eSocketBindFailed & e) {
		ROFL_ERR("%s threw rofl::eSocketBind error\n",
			__PRETTY_FUNCTION__);
	} catch (rofl::cerror & e) {
		ROFL_ERR("%s threw rofl::cerror\n",
			__PRETTY_FUNCTION__);
	}
	ROFL_DEBUG("%s finished.\n",__PRETTY_FUNCTION__);
}

// TODO are all transaction IDs invalidated by a connection reset??
void morpheus::handle_ctrl_close (rofl::cofctl *src) {
	ROFL_INFO("morpheus::handle_ctrl_close called with %s\n",
		(src?src->c_str():"NULL"));
    ctl_state= PATH_CLOSED;
	// controller disconnected - now disconnect from switch.
	rpc_disconnect_from_dpt(m_slave);
	
	// this socket disconnecting could just be a temporary thing - mark it is dead, but expect a possible auto reconnect
	if(src!=m_master) {
		ROFL_ERR("In %s closing control path does not match that opened %s\n",
		(m_master?m_master->c_str():"NULL"),
		(src?src->c_str():"NULL")
		);
	}
	m_master=0;	
}

void morpheus::handle_dpath_open (rofl::cofdpt *src) {
	// should be called automatically after call to rpc_connect_to_dpt in connect_to_slave
    dpt_state= PATH_OPEN;
	ROFL_DEBUG("%s called with %s\n", __PRETTY_FUNCTION__,
		(src?src->c_str():"NULL"));
	m_slave = src;
	m_slave_dpid= xdpd::control_manager::Instance()->get_dpid();
	m_dpid = m_slave_dpid;
	process_ctlqueue();
	process_dptqueue();
}

// TODO are all transaction IDs invalidated by a connection reset??
void morpheus::handle_dpath_close (rofl::cofdpt *src) {
    
	ROFL_DEBUG("%s called with %s\n",__PRETTY_FUNCTION__,
		(src?src->c_str():"NULL"));
	if(src!=m_slave) {
		ROFL_ERR("In %s closing datapath does not match that opened %s\n",
		(m_slave?m_slave->c_str():"NULL"),
		(src?src->c_str():"NULL")
		);
	}
	m_slave=0;	
	// If master is still open try to reconnect
	if (m_master != 0) {
		set_dpt_watcher();
	}
    dpt_state= PATH_CLOSED;
}


void morpheus::register_lifetime_session_timer(morpheus::chandlersession_base * s, unsigned seconds) {
	int opaque = register_session_timer(s,seconds);
	s->setLifetimeTimerOpaque(opaque);
}

int morpheus::register_session_timer(morpheus::chandlersession_base * s, unsigned seconds) {
// int morpheus::register_session_timer(unsigned seconds) {
	int opaque = m_last_crof_timer_opaque - m_crof_timer_opaque_offset;
	int off;
	for(off = 0; off < m_crof_timer_opaque_max; ++off) {
		if(!m_session_timers[(opaque+off)%m_crof_timer_opaque_max]) { break; }	// found a null entry - it can be our next opaque
	}
	if(off==m_crof_timer_opaque_max) 
		throw std::range_error("Ran out of session timer opaque values.");
	opaque = (opaque+off)%m_crof_timer_opaque_max;
	if(m_session_timers[opaque]) {
        ROFL_ERR("Timer added to session already present in %s\n",__PRETTY_FUNCTION__);
    } 
	m_session_timers[opaque] = s;
	opaque += m_crof_timer_opaque_offset;
	register_timer(opaque, seconds);
    ROFL_DEBUG("Registered session %s with opaque %d\n",
		s->asString().c_str(), opaque);
	m_last_crof_timer_opaque = opaque;
	return opaque;
}


// this method handles all the timing events - these can be either session lifetime evenets, or some secondary event created by a session.
// ** THIS METHOD IS THE ONE THAT REMOVES AN EXPIRED SESSION FROM m_session_timers and the sessions database.
void morpheus::handle_timeout ( int opaque ) {
	ROFL_DEBUG("%s : called with opaque = %xd / %xd \n",
		__PRETTY_FUNCTION__, opaque, (opaque-m_crof_timer_opaque_offset));
	// TODO make sure to remove *all* instances of the pointer to the session from the m_session_timers array (scan for value in array, swap with 0, delete whats pointed to by array
	// check that this is a session_timer
	int opaque_off = opaque - m_crof_timer_opaque_offset;
	rofl::RwLock session_timers_lock(&m_session_timers_lock, rofl::RwLock::RWLOCK_WRITE);
	if(!m_session_timers[opaque_off]) {
		ROFL_ERR("%s : called with  unknown opaque = %xd / %xd \n",
		__PRETTY_FUNCTION__, opaque, (opaque-m_crof_timer_opaque_offset));
		assert(false);
    }
	morpheus::chandlersession_base * s = m_session_timers[opaque_off];
	if(s->getLifetimeTimerOpaque()==opaque) {
		// this is a session lifetime timer - find all mentions of this session and delete it.
		
		// first remove from sessions_timer
		std::replace(m_session_timers.begin(), m_session_timers.end(), s, (morpheus::chandlersession_base *) 0);
		// now remove from sessions database and delete
		if(!s->isCompleted()) {
			ROFL_DEBUG("%s : called on session %xd %s which wasn't completed\n",
		__PRETTY_FUNCTION__, s,typeid(*s).name());
			// TODO send error?
			}
		rofl::RwLock lock(&m_sessions_lock, rofl::RwLock::RWLOCK_WRITE);
		ROFL_DEBUG("%s : deleting session %s. \n",
			__PRETTY_FUNCTION__, s->asString().c_str());
		delete(s);
	} else {
		// this is a timer belonging to some other session function, so tell the session
//		std::cout << "About to call s->handle_timeout on " << s->asString() << std::endl;
		s->handle_timeout(opaque);
	}
}

void morpheus::check_locks() 
{
	rofl::RwLock session_lock(&m_sessions_lock, rofl::RwLock::RWLOCK_WRITE); 
	rofl::RwLock session_timers_lock(&m_session_timers_lock, rofl::RwLock::RWLOCK_WRITE);
}

#undef STRINGIFY
#undef TOSTRING
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

// this is from a ctl if CTL_DPT is true, false otherwise
#define HANDLE_REQUEST_WITH_REPLY_TEMPLATE(CTL_DPT, MSG_TYPE, SESSION_TYPE) { \
	ROFL_DEBUG("%s from %s : %s\n", func, src->c_str(), msg->c_str()); \
	if((!m_slave)||(!m_master)) { \
		ROFL_DEBUG("%s: queueing message due to lack of dpt/ctl\n",func); \
		if (CTL_DPT) ctlmsgqueue.push_back(msg);   \
		else dptmsgqueue.push_back(msg);   \
		return; \
	} \
	check_locks(); \
	try { \
		new SESSION_TYPE ( this, src, msg ); /* this isn't a leak as the constructor of the session should register the xids and the pointer will stay in the  */ \
	} catch(rofl::cerror &e) { \
		ROFL_ERR("%s: Unhandled cerror %s\n",func, e.desc.c_str()); assert(false); } \
	delete(msg); \
}

#define HANDLE_REPLY_AFTER_REQUEST_TEMPLATE(CTL_DPT, MSG_TYPE, SESSION_TYPE, REPLY_FN) { \
	if(CTL_DPT) { \
		ROFL_ERR("%s: message unexpectedly marked as control type\n",func); \
		return; \
	} \
	if((!m_slave)||(!m_master)) { \
		ROFL_DEBUG("%s: No control/datapath queueing message\n",func); \
		dptmsgqueue.push_back(msg); \
		return; \
	} \
	check_locks(); \
	chandlersession_base *b= get_chandlersession(msg); \
	if (b == 0) { \
		ROFL_ERR("%s: cannot find xid for %s\n",func, msg->c_str()); \
		delete(msg); \
		return; \
	} \
	SESSION_TYPE * s = dynamic_cast<SESSION_TYPE *>(b); \
	if(!s) { \
		ROFL_ERR("%s : Session resolved to wrong type discarded %s\n", \
			func, b); \
	} \
	try { \
		s->REPLY_FN( src, msg ); \
		if(s->isCompleted()) { remove_session(s); delete(s); } \
	} catch(rofl::cerror &e) { \
		ROFL_ERR("%s: Unhandled error %s\n",func, e.desc.c_str()); assert(false); } \
	delete(msg); \
}

#define HANDLE_MESSAGE_FORWARD_TEMPLATE(CTL_DPT, SESSION_TYPE) { \
	ROFL_DEBUG("%s: from %s message %s\n", func, src->c_str(), msg->c_str()); \
	if((!m_slave)||(!m_master)) { \
		ROFL_DEBUG("%s: queueing message due to lack of dpt/ctl\n",func); \
		if (CTL_DPT) ctlmsgqueue.push_back(msg);   \
		else dptmsgqueue.push_back(msg);   \
		return; \
	} \
	check_locks(); \
	try { \
		new SESSION_TYPE ( this, src, msg ); \
	} catch(rofl::cerror &e) { \
		ROFL_ERR("%s: unhandled cerror %s\n",func,e.desc.c_str()); assert(false); \
	} \
	delete(msg); \
}

/**Find which chandler session is in map*/
morpheus::chandlersession_base * morpheus::get_chandlersession(rofl::cofmsg *msg)
{
	xid_session_map_t::iterator it; 
	for(it = m_sessions.begin() ; 
		it != m_sessions.end(); ++it) {
		if(it->first.second == msg->get_xid()) 
			break;
	}
	if(it == m_sessions.end()) { 
		ROFL_ERR("%s: cannot find session for %s with xid %ld\n",
			__PRETTY_FUNCTION__,msg->c_str(),msg->get_xid());
		/*std::cout << func << ": Unexpected " << TOSTRING(MSG_TYPE) << " received with xid " << msg->get_xid() << ". Dropping new message." << std::endl; return; } */
		return 0;
	}
	return it->second;
}

void morpheus::wait_for_slave()
{
	for (unsigned int i= 0; i < 3; i++) {
		if (!m_slave) {
			ROFL_DEBUG("%s: No datapath sleep.\n",__PRETTY_FUNCTION__);
			sleep(3);
		}
	}
}

void morpheus::wait_for_master()
{
	for (unsigned int i= 0; i < 3; i++) {
		if (!m_master) {
			ROFL_DEBUG("%s: No control path sleep.\n",__PRETTY_FUNCTION__);
			sleep(3);
		}
	}
}


void morpheus::handle_flow_mod(rofl::cofctl * src, rofl::cofmsg_flow_mod *msg) {
	static const char * func = __FUNCTION__;
	HANDLE_MESSAGE_FORWARD_TEMPLATE(true, morpheus::csh_flow_mod)
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
	static const char * func = __FUNCTION__;
	HANDLE_REQUEST_WITH_REPLY_TEMPLATE( true, cofmsg_flow_stats_request, morpheus::csh_flow_stats );
}

void morpheus::handle_flow_stats_reply(rofl::cofdpt *src, rofl::cofmsg_flow_stats_reply *msg) {
	static const char * func = __FUNCTION__;
    xid_session_map_t::iterator it; 
	for(it = m_sessions.begin() ; it != m_sessions.end(); ++it) {
        if(it->first.second==msg->get_xid()) break; 
    }
	if(it == m_sessions.end()) { 
        ROFL_ERR("Unexpected message type %s received in %s with xid %d\n",
            msg->c_str(), __PRETTY_FUNCTION__, msg->get_xid());
        delete(msg);
        return;
    }
    
	// Casts just check if those things can be cast
    if (dynamic_cast<csh_flow_stats *>(it->second)) {
        HANDLE_REPLY_AFTER_REQUEST_TEMPLATE( false, cofmsg_flow_stats_reply, morpheus::         csh_flow_stats, process_flow_stats_reply );
    } else if (dynamic_cast<csh_aggregate_stats *>(it->second)) {
        HANDLE_REPLY_AFTER_REQUEST_TEMPLATE( false, cofmsg_flow_stats_reply, morpheus::         csh_aggregate_stats, process_flow_stats_reply );
    } else {
        ROFL_ERR("Could not identify message in %s\n",__PRETTY_FUNCTION__);
    }
}

void morpheus::handle_queue_stats_request(rofl::cofctl *src, rofl::cofmsg_queue_stats_request *msg) {
	//static const char * func = __FUNCTION__;
	ROFL_WARN("%s: not implemented\n",__PRETTY_FUNCTION__);
	delete(msg);
}
void morpheus::handle_experimenter_stats_request(rofl::cofctl *src, rofl::cofmsg_stats_request *msg) {
	
	//static const char * func = __FUNCTION__;
	
	ROFL_WARN("%s:  not implemented\n",__PRETTY_FUNCTION__);
	delete(msg);
}

void morpheus::handle_port_mod(rofl::cofctl *src, rofl::cofmsg_port_mod *msg) {
	static const char * func = __FUNCTION__;
	HANDLE_MESSAGE_FORWARD_TEMPLATE(true, morpheus::csh_port_mod)
}
void morpheus::handle_queue_get_config_request(rofl::cofctl *src, rofl::cofmsg_queue_get_config_request *msg) {
	ROFL_WARN("%s: not implemented\n",__PRETTY_FUNCTION__);
	delete(msg);
}
void morpheus::handle_experimenter_message(rofl::cofctl *src, rofl::cofmsg_features_request *msg) {
	ROFL_WARN("%s:  not implemented\n",__PRETTY_FUNCTION__);
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


