
#include <rofl/platform/unix/cunixenv.h>
#include <rofl/datapath/afa/fwd_module.h>
#include <rofl/common/crofbase.h>
#include <rofl/common/openflow/cofmatch.h>
#include <rofl/common/utils/c_logger.h>
#include "../src/xcpd/morpheus.h"

void print_translate_match(rofl::cofmatch &, morpheus &morph);

int main(int argc, char** argv){
    cportvlan_mapper mapper;
    mapper.add_virtual_port(  cportvlan_mapper::port_spec_t(
        PV_PORT_T(2),      
        PV_VLANID_T(10)));
    mapper.add_virtual_port(  cportvlan_mapper::port_spec_t(
        PV_PORT_T(2),      
        PV_VLANID_T(11)));
    mapper.add_virtual_port(  cportvlan_mapper::port_spec_t(
        PV_PORT_T(2),      
        PV_VLANID_T(12)));
    mapper.add_virtual_port( cportvlan_mapper::port_spec_t(
        PV_PORT_T(1),  
        PV_VLANID_T::make_NONE()));
    std::cout << mapper << std::endl;
    caddress fakeaddr= caddress(AF_INET,"127.0.0.1",80);
    morpheus morph (mapper, true, fakeaddr, true, fakeaddr);
    rofl::cofmatch match= rofl::cofmatch(OFP10_VERSION);
    for (int i=1 ; i <=4; i++) {
        match.set_in_port(i);
        print_translate_match(match, morph);
        std::cout << std::endl;
    }
    return 0;
}

void print_translate_match(rofl::cofmatch &match, morpheus & morph)
{
    rofl::cofmatch newmatch= morph.get_fet()->trans_match(match);
    int vid= -1;
    try {
        vid= match.get_vlan_vid();
    } catch (...) {
    }
    std::cout << "Original match port:vlan " << match.get_in_port() <<
        ":" << vid << std::endl;
    try {
        vid= newmatch.get_vlan_vid();
    } catch (...) {
    }
    std::cout << "translated match port:vlan " << newmatch.get_in_port() <<
        ":" << vid << std::endl;
}
