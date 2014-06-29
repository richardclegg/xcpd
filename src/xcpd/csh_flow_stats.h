
#ifndef UCL_EE_CSH_FLOW_STATS
#define UCL_EE_CSH_FLOW_STATS 1

#include "morpheus.h"
#include "morpheus_nested.h"

class morpheus::csh_flow_stats : public morpheus::chandlersession_base {

private:
    int outstanding_replies;
    std::vector< rofl::cofflow_stats_reply> replies;
    std::vector<rofl::cflowentry> reply_map;
    std::vector<uint32_t> reply_xid;
public:
    csh_flow_stats(morpheus * parent, rofl::cofctl * const src, rofl::cofmsg_flow_stats_request * const msg);
    bool process_flow_stats_request ( rofl::cofctl * const src, rofl::cofmsg_flow_stats_request * const msg );
    bool process_flow_stats_reply ( rofl::cofdpt * const src, rofl::cofmsg_flow_stats_reply * const msg );
    bool handle_error (rofl::cofdpt *src, rofl::cofmsg_error *msg);
    ~csh_flow_stats();
    std::string asString() const;

};

#endif
