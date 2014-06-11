#ifndef UCL_EE_CSH_PORT_STATS
#define UCL_EE_CSH_PORT_STATS 1

#include "morpheus.h"
#include "morpheus_nested.h"

class morpheus::csh_port_stats : public morpheus::chandlersession_base {

private:
    uint32_t port;
protected:
    uint32_t m_request_xid;

public: 
    csh_port_stats(morpheus * parent, rofl::cofctl * const src, rofl::cofmsg_port_stats_request * const msg);
    bool process_port_stats_request ( rofl::cofctl * const src, rofl::cofmsg_port_stats_request * const msg );
    bool process_port_stats_reply ( rofl::cofdpt * const src, rofl::cofmsg_port_stats_reply * const msg );
    ~csh_port_stats();
    std::string asString() const;
    uint32_t get_port();
    void set_port(uint32_t);

};

#endif
