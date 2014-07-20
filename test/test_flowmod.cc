
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
    morph.set_supported_dpe_features(0,0xffff);
    rofl::cofaclist outlist= rofl::cofaclist(OFP10_VERSION);
    for (int i=1; i<=4; i++) {
        outlist.next() =  rofl::cofaction_output(OFP10_VERSION, 
            i);
    }
    //outlist.next()= rofl::cofaction_set_vlan_vid( OFP10_VERSION, 11);
    std::cout << outlist.c_str() << std::endl;
    std::cout << "Translate called\n" << std::endl;
    try {
        rofl::cofaclist translist= morph.get_fet()->trans_actions(outlist, match);
        std::cout << translist.c_str() << std::endl;
    } catch (...) {
        std::cout << "Translate threw error when it should not\n" << std::endl;
        return 0;
    }
    for (int i=1; i <= 4; i++) {
        match.set_in_port(i);
        rofl::cofmsg_flow_mod fm= rofl::cofmsg_flow_mod(OFP10_VERSION,
            0,0,0,0,0,0,0,0,0,outlist, match);
        std::cout << "Translate and add flow entry\n" << std::endl;
        rofl::cflowentry entry= morph.get_fet()->get_flowentry_from_msg(&fm);
        rofl::cflowentry trans(OFP10_VERSION);
        trans= morph.get_fet()->trans_flow_entry(entry);
        morph.get_fet()->add_flow_entry(entry,trans);
    }
    match.set_in_port(1);
    rofl::cofmsg_flow_mod fm2= rofl::cofmsg_flow_mod(OFP10_VERSION,
        0,0,0,0,0,0,0,0,0,outlist, match);
    std::vector <cflowentry> fes= morph.get_fet()-> get_translated_matches_and_delete
        (match,0,true);
    std::cout << "Got translated " << fes.size() << "(should be 1)" <<
        std::endl;
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
		ROFL_ERR("Problem with match in %s\n",__PRETTY_FUNCTION__);
    }
    std::cout << "translated match port:vlan " << newmatch.get_in_port() <<
        ":" << vid << std::endl;
}
