
#ifndef UCL_EE_CSH_AGGREGATE_STATS
#define UCL_EE_CSH_AGGREGATE_STATS 1

#include "morpheus.h"
#include "morpheus_nested.h"

class morpheus::csh_aggregate_stats : public morpheus::chandlersession_base {

private:
    uint64_t pkt_count;
    uint64_t byte_count;
    uint32_t flow_count;
    uint32_t outstanding_replies;
    bool error_flagged;

public:
    csh_aggregate_stats(morpheus * parent, const rofl::cofctl * const src, rofl::cofmsg_aggr_stats_request * const msg);
    bool process_aggr_stats_request ( const rofl::cofctl * const src, rofl::cofmsg_aggr_stats_request * const msg );
    bool process_aggr_stats_reply ( rofl::cofdpt * const src, rofl::cofmsg_aggr_stats_reply * const msg );
    bool process_flow_stats_reply ( rofl::cofdpt * const src, rofl::cofmsg_flow_stats_reply * const msg );
    void send_reply();
    ~csh_aggregate_stats();
    std::string asString() const;

};

#endif
