
#ifndef UCL_EE_CSH_AGGREGATE_STATS
#define UCL_EE_CSH_AGGREGATE_STATS 1

#include "morpheus.h"
#include "morpheus_nested.h"

// class morpheus;
// class morpheus::chandlersession_base;

// TODO translation check
class morpheus::csh_aggregate_stats : public morpheus::chandlersession_base {

protected:
// uint32_t m_request_xid;
uint64_t pkt_count;
uint64_t byte_count;
uint32_t flow_count;
uint32_t replies_outstanding;
bool error_flagged;

public:
csh_aggregate_stats(morpheus * parent, const rofl::cofctl * const src, rofl::cofmsg_aggr_stats_request * const msg);
bool process_aggr_stats_request ( const rofl::cofctl * const src, rofl::cofmsg_aggr_stats_request * const msg );
bool process_aggr_stats_reply ( rofl::cofdpt * const src, rofl::cofmsg_aggr_stats_reply * const msg );
bool process_flow_stats_reply ( rofl::cofdpt * const src, rofl::cofmsg_flow_stats_reply * const msg );
~csh_aggregate_stats();
std::string asString() const;

};

#endif
