
#ifndef UCL_EE_CSH_FLOW_MOD
#define UCL_EE_CSH_FLOW_MOD 1

#include "morpheus.h"
#include "morpheus_nested.h"
#include <rofl/common/openflow/openflow10.h>
#include <rofl/common/openflow/cofctl.h>

void dumpBytes (std::ostream & os, uint8_t * bytes, size_t n_bytes);

// TODO make sure that incoming VLAN is stripped
class morpheus::csh_flow_mod : public morpheus::chandlersession_base {

public:

    csh_flow_mod(morpheus * parent, rofl::cofctl * const src, 
        rofl::cofmsg_flow_mod * const msg );
    bool process_flow_mod ( rofl::cofctl * const src, 
        rofl::cofmsg_flow_mod * const msg );
    bool handle_error (rofl::cofdpt *src, rofl::cofmsg_error *msg);
    ~csh_flow_mod();
    std::string asString() const;
    
private:
    rofl::cflowentry get_flowentry_from_msg(rofl::cofmsg_flow_mod * const);
    bool process_add_flow ( rofl::cofctl * const src, 
        rofl::cofmsg_flow_mod * const msg );

    bool process_modify_flow ( rofl::cofctl * const src, 
        rofl::cofmsg_flow_mod * const msg );
    bool process_modify_strict_flow ( rofl::cofctl * const src, 
        rofl::cofmsg_flow_mod * const msg );
    bool process_delete_flow ( rofl::cofctl * const src, 
        rofl::cofmsg_flow_mod * const msg );    
    bool process_delete_strict_flow ( rofl::cofctl * const src, 
        rofl::cofmsg_flow_mod * const msg );    

};

#endif
