#include "test_datapath.h"
#include <string.h>
#include <rofl/platform/unix/cunixenv.h>
#include <rofl/datapath/afa/fwd_module.h>
#include <rofl/common/utils/c_logger.h>
#include <rofl/common/ciosrv.h>
int main()
{
	std::string switch_ip= "127.0.0.1";
    int switch_port= 6633;
    rofl::caddress switch_addr= rofl::caddress(AF_INET, switch_ip.c_str(),switch_port);

	test_datapath tdp=test_datapath();
	try{
		tdp.connect(switch_addr);
		rofl::ciosrv::init();
		rofl::ciosrv::run();
	} catch (rofl::eSocketBindFailed e) {
		std::cerr << "Socket bind failed\n" << std::endl;
		return 0;
	}
	
}


void test_datapath::connect(rofl::caddress &a)
{
	rpc_listen_for_dpts(a);
}


