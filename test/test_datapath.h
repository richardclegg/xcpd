#include <rofl/common/caddress.h>
#include <rofl/common/crofbase.h>
#include <rofl/common/openflow/cofdpt.h>
#include <rofl/common/openflow/cofctl.h>

class test_datapath : public rofl::crofbase {
	private:
		rofl::caddress dpt;

	public:
		void connect(rofl::caddress &);
};
