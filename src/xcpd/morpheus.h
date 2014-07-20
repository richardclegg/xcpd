
#ifndef UCL_EE_MORPHEUS_H
#define UCL_EE_MORPHEUS_H

#include <sstream>
#include <string>
#include <map>
#include <rofl/common/caddress.h>
#include <rofl/common/crofbase.h>
#include <rofl/common/openflow/cofdpt.h>
#include <rofl/common/openflow/cofctl.h>
#include <rofl/common/openflow/openflow10.h>
#include <pthread.h>
#include <rofl/common/thread_helper.h>
#include "cportvlan_mapper.h"
#include "flow_entry_translate.h"

std::string action_mask_to_string(const uint32_t action_types);
std::string capabilities_to_string(const uint32_t capabilities);
std::string port_as_string(uint16_t p);

bool operator==(const rofl::cofaclist & a, const rofl::cofaclist & b);
bool operator==(const rofl::cofaction & a, const rofl::cofaction & b);
	
class flow_entry_translate;  // Forward declaration due to nested includes
    
class morpheus : public rofl::crofbase {



public:

// forward declare our nested classes which will encapsulate the transaction (composed of persistent data etc)
	class chandlersession_base;
	class csh_flow_mod;
	class csh_features_request;
	class csh_get_config;
	class csh_desc_stats;
	class csh_table_stats;
	class cport_stats_session;
	class csh_flow_stats;
    class csh_port_stats;
	class csh_aggregate_stats;
	class cqueue_stats_session;
	class csh_packet_in;
	class csh_packet_out;
	class csh_packet_in;
	class csh_barrier;
	class csh_table_mod;
	class csh_port_mod;
	class csh_set_config;

private:
    flow_entry_translate *fet; // Class used to translate flow entries
	std::vector <rofl::cofmsg *> ctlmsgqueue; //queue of messages going up to controller
	std::vector <rofl::cofmsg *> dptmsgqueue; // queue of messages going down to data path
	
	void check_locks();
	void process_ctlqueue();
	void process_dptqueue();
    void wait_for_slave();
    void wait_for_master();
	chandlersession_base * get_chandlersession(rofl::cofmsg *);

protected:
    
    typedef std::map < std::pair< uint32_t, uint32_t >, chandlersession_base * >        xid_session_map_t;
    
    xid_session_map_t m_sessions;
    mutable pthread_rwlock_t m_sessions_lock;	// a lock for m_sessions
    
    // xid_reverse_session_map_t m_reverse_sessions;
    
    struct port_config_t {
        
    };
    std::map<uint16_t, port_config_t> port_enabled;
    rofl::cofdpt * m_slave;		// the datapath device that we'll be misrepresenting
    rofl::cofctl * m_master;	// the OF controller.
    cportvlan_mapper m_mapper;
    uint64_t m_slave_dpid;
    uint64_t m_dpid;
    const uint32_t m_supported_features;
    const uint32_t m_supported_actions_mask;
    uint32_t m_supported_actions;
    uint32_t m_dpe_supported_actions;
    bool m_dpe_supported_actions_valid;
    
    bool indpt, inctl;
    rofl::caddress dptaddr, ctladdr;
    
    int dpt_state;
    int ctl_state;
    
    static const int PATH_CLOSED;
    static const int PATH_WAIT;
    static const int PATH_OPEN;

    const int m_crof_timer_opaque_offset;	// the minimum opaque value for the session timeout timers, so supplied to register_timer
    const int m_crof_timer_opaque_max;	// the largest number above offset for the opaque values - this will also be the largest size of m_session_timeout_timers
    int m_last_crof_timer_opaque;
    unsigned int max_session_lifetime;	// the maximum time, in seconds, that a session can live. If the 
    std::vector <class chandlersession_base *> m_session_timers;	// a vector containing pointers to active sessions. The index is the the opaque value passed to register_timer less m_crof_timer_opaque_offset
    mutable pthread_rwlock_t m_session_timers_lock;	// a lock for m_session_timers


// crofbase overrides
	virtual void handle_dpath_open (rofl::cofdpt *);
	virtual void handle_dpath_close (rofl::cofdpt *);
	virtual void handle_ctrl_open (rofl::cofctl *);
	virtual void handle_ctrl_close (rofl::cofctl *);
	virtual void handle_features_request(rofl::cofctl *ctl, rofl::cofmsg_features_request * msg );
	virtual void handle_features_reply(rofl::cofdpt * dpt, rofl::cofmsg_features_reply * msg );
//	virtual void handle_error (rofl::cofdpt *, rofl::cofmsg *msg);
	virtual void handle_get_config_request(rofl::cofctl *ctl, rofl::cofmsg_get_config_request * msg );
	virtual void handle_get_config_reply(rofl::cofdpt * dpt, rofl::cofmsg_get_config_reply * msg );
	
	

	virtual void handle_desc_stats_request(rofl::cofctl *ctl, rofl::cofmsg_desc_stats_request * msg );
	virtual void handle_desc_stats_reply(rofl::cofdpt * dpt, rofl::cofmsg_desc_stats_reply * msg );

	virtual void handle_table_stats_request(rofl::cofctl *ctl, rofl::cofmsg_table_stats_request * msg );
	virtual void handle_table_stats_reply(rofl::cofdpt *dpt, rofl::cofmsg_table_stats_reply * msg );

	virtual void handle_set_config(rofl::cofctl *ctl, rofl::cofmsg_set_config * msg );

	virtual void handle_port_stats_request(rofl::cofctl *ctl, rofl::cofmsg_port_stats_request * msg );
    virtual void handle_port_stats_reply(rofl::cofdpt *dpt, rofl::cofmsg_port_stats_reply * msg );
	virtual void handle_flow_stats_request(rofl::cofctl *ctl, rofl::cofmsg_flow_stats_request * msg );
    virtual void handle_flow_stats_reply(rofl::cofdpt *dpt, rofl::cofmsg_flow_stats_reply * msg );
	virtual void handle_aggregate_stats_request(rofl::cofctl *ctl, rofl::cofmsg_aggr_stats_request * msg );
	virtual void handle_aggregate_stats_reply(rofl::cofdpt *dpt, rofl::cofmsg_aggr_stats_reply * msg );
	
	virtual void handle_queue_stats_request(rofl::cofctl *ctl, rofl::cofmsg_queue_stats_request * msg );
	virtual void handle_experimenter_stats_request(rofl::cofctl *ctl, rofl::cofmsg_stats_request * msg );
	virtual void handle_packet_in(rofl::cofdpt *dpt, rofl::cofmsg_packet_in * msg); 	
	virtual void handle_packet_out(rofl::cofctl *ctl, rofl::cofmsg_packet_out * msg );
	virtual void handle_barrier_request(rofl::cofctl *ctl, rofl::cofmsg_barrier_request * msg );
	virtual void handle_barrier_reply ( rofl::cofdpt * dpt, rofl::cofmsg_barrier_reply * msg );
	// virtual void handle_table_mod(rofl::cofctl *ctl, rofl::cofmsg_table_mod * msg );
	virtual void handle_port_mod(rofl::cofctl *ctl, rofl::cofmsg_port_mod * msg );
	virtual void handle_queue_get_config_request(rofl::cofctl *ctl, rofl::cofmsg_queue_get_config_request * msg );
	virtual void handle_experimenter_message(rofl::cofctl *ctl, rofl::cofmsg_features_request * msg );
	virtual void handle_flow_mod(rofl::cofctl *ctl, rofl::cofmsg_flow_mod * msg );
// timeout methods
	virtual void handle_timeout ( int opaque );
	virtual void handle_error ( rofl::cofdpt * src, rofl::cofmsg_error * msg );

public:
// our transaction management methods - they are public because the nested classes have to call them

	bool associate_xid( const uint32_t ctl_xid, const uint32_t dpt_xid, chandlersession_base * p ); // tells the translator that ctl_xid was translated to dpt_xid is the translation of a message for session p.  Returns true if the the xid pair were successfully added to the database, false otherwise (.e.g they are already in the db)
	bool remove_xid_association( const uint32_t ctl_xid, const uint32_t dpt_xid ); // removes the ctl_xid/dpt_xid association from the database - returns true if xid pair was found and removed, false otherwise

    flow_entry_translate *get_fet();  /** Return pointer to translation table for flow entries */
    
	unsigned remove_session( chandlersession_base * p );	// called to remove all associations to this session_base - returns the number of associations removed
	rofl::cofdpt * get_dpt() const;
	rofl::cofctl * get_ctl() const;
	uint64_t get_dpid() const { return m_dpid; }
	morpheus(const cportvlan_mapper & mapper, bool indpt, rofl::caddress dptaddr, bool inctl, rofl::caddress ctladdr);	// if indpt is true then morpheus will listen on dtpaddr, otherwise it will connect to it.
	virtual ~morpheus();
	const cportvlan_mapper & get_mapper() const { return m_mapper; }

	void initialiseConnections();

    // void register_session_timer(int opaque, morpheus::chandlersession_base *, unsigned seconds);
    int register_session_timer(morpheus::chandlersession_base *, unsigned seconds);
    void register_lifetime_session_timer(morpheus::chandlersession_base * s, unsigned seconds);
    void cancel_session_timer(int opaque);
    void set_ctl_watcher();
    void set_dpt_watcher();
    
    uint32_t get_supported_actions();
    uint32_t get_supported_features() { return m_supported_features; }

    void set_supported_dpe_features (uint32_t new_capabilities, uint32_t new_actions);
    
    std::string dump_sessions() const;
    std::string dump_config() const;
    friend std::ostream & operator<< (std::ostream & os, const morpheus & morph);


};

#include "csh_aggregate_stats.h"
#include "csh_barrier.h"
#include "csh_desc_stats.h"
#include "csh_features_request.h"
#include "csh_flow_mod.h"
#include "csh_flow_stats.h"
#include "csh_get_config.h"
#include "csh_packet_in.h"
#include "csh_packet_out.h"
#include "csh_port_mod.h"
#include "csh_set_config.h"
#include "csh_table_mod.h"
#include "csh_table_stats.h"
#include "csh_port_stats.h"
#include "morpheus_nested.h"

#endif // UCL_EE_MORPHEUS_H

