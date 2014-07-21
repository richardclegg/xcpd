/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "control_manager.h"
#include <iostream>
#include <iomanip>

using namespace std;

/**
* @file control_manager.cc
* @author Richard Clegg richard@richardclegg.org
*
* @brief Manages control path daemon parameters
*/

using namespace rofl;
using namespace xdpd;

int virtual_port::port_count= 1;

virtual_port::virtual_port() {
}

virtual_port::virtual_port(std::string n, int port) {
    name= n;
    real_port= port;
    vlan= NO_VLAN;
    init_port();
}

void virtual_port::init_port()
{
    std::stringstream stream("00:00:00:00:00:");
    stream.seekp(0, std::ios::end);
    stream << std::hex << std::setw(2) << std::setfill('0') 
		<< port_count << std::dec;
   
    mac= rofl::cmacaddr(stream.str().c_str());
    std::stringstream stream2("");
    stream2 << "vport_" << port_count;
    name= stream2.str();
    port_count++;
    //cout << "Mac is " << stream.str() << " " << mac << " name is " <<
	//	stream2.str() << endl;
    //cout << "New Port phys:" << port << " NO VLAN" << endl;
}

virtual_port::virtual_port(std::string n, int port,int v) {
    name= n;
    real_port= port;
    vlan= v;
    //cout << "New Port phys:" << port << " VLAN " << v << endl;
    init_port();
}

rofl::cmacaddr virtual_port::get_mac()
{
	return mac;
}

void virtual_port::set_mac(rofl::cmacaddr m)
{
	mac= m;
	//cout << "MAC is " << mac.c_str() << endl;
}

std::string virtual_port::get_name()
{
	//cout << "PORT NAME " << name << endl;
	return name;
}

void virtual_port::set_name(std::string n)
{
	name= n;
	//cout << "PORT NAME " << name << endl;
}

int virtual_port::get_real_port()
{
    return real_port;
}

int virtual_port::get_vlan()
{
    return vlan;
}


control_manager* control_manager::cm_instance= NULL;


control_manager* control_manager::Instance()
{
    if (!cm_instance) {
        cm_instance= new control_manager;
    }
    return cm_instance;
}

void control_manager::init()
// Set default 
{
    switch_ip= "127.0.0.1";
    switch_port= 6631;
    xcpd_ip="127.0.0.1";
    xcpd_port=6632;
    higher_ip="127.0.0.1";
    higher_port=6633;
    switch_addr= caddress(AF_INET, switch_ip.c_str(),switch_port);
    xcpd_addr= caddress(AF_INET, xcpd_ip.c_str(),xcpd_port);
    higher_addr= caddress(AF_INET, higher_ip.c_str(),higher_port);
    switch_name= std::string();
    port_names= std::vector<std::string>();
    switch_to_xcpd_conn= ACTIVE_CONNECTION;
    xcpd_to_control_conn= ACTIVE_CONNECTION;
    ports= std::vector<virtual_port>();
    queue_command_handling= PASSTHROUGH_COMMAND;
    port_stat_handling= PASSTHROUGH_COMMAND;
    port_config_handling= PASSTHROUGH_COMMAND;
    hm= NULL;
}


void control_manager::set_dpid(uint64_t d) 
{
    dpid= d;
}

uint64_t control_manager::get_dpid()
{
    return dpid;
}

void control_manager::set_higher_address(caddress &c)
{
    higher_addr=c;
}

caddress control_manager::get_higher_address()
{
    return higher_addr;
}


void control_manager::set_xcpd_address(caddress &c)
{
    xcpd_addr=c;
}

caddress control_manager::get_xcpd_address()
{
    return xcpd_addr;
}


void control_manager::set_switch_address(caddress &c)
{
    switch_addr=c;
}

caddress control_manager::get_switch_address()
{
    return switch_addr;
}

void control_manager::set_switch_ip(std::string ip)
{
    switch_ip= ip;
    switch_addr= caddress(AF_INET, switch_ip.c_str(),switch_port);
}
std::string control_manager::get_switch_ip()
{
    return switch_ip;
}

void control_manager::set_switch_port(int p)
{
    switch_port=p;
    switch_addr= caddress(AF_INET, switch_ip.c_str(),switch_port);
}

int control_manager::get_switch_port()
{
    return switch_port;
}


void control_manager::set_xcpd_ip(std::string ip)
{
    xcpd_ip= ip;
    xcpd_addr= caddress(AF_INET, xcpd_ip.c_str(),xcpd_port);
}
std::string control_manager::get_xcpd_ip()
{
    return xcpd_ip;
}

void control_manager::set_xcpd_port(int p)
{
    xcpd_port=p;
    xcpd_addr= caddress(AF_INET, xcpd_ip.c_str(),xcpd_port);
}

int control_manager::get_xcpd_port()
{
    return xcpd_port;
}


void control_manager::set_higher_ip(std::string ip)
{
    higher_ip= ip;
    higher_addr= caddress(AF_INET, higher_ip.c_str(),higher_port);
}

std::string control_manager::get_higher_ip() {
    return higher_ip;
}

void control_manager::set_higher_port(int port) 
{
    higher_port= port;
    higher_addr= caddress(AF_INET, higher_ip.c_str(),higher_port);
}

int control_manager::get_higher_port() 
{
    return higher_port;
}

void control_manager::set_switch_name(std::string n)
{
    switch_name= n;
}
    
std::string control_manager::get_switch_name()
{
    return switch_name;
}

void control_manager::add_port(std::string n)
{
    port_names.push_back(n);
}

int control_manager::no_ports()
{
    return port_names.size();
}

int control_manager::get_port_no(std::string n)
{
    for (int i= 0; i < (int)port_names.size(); i++) {
        if (port_names[i] == n) {
            return i;
        }
    }
    return -1;
}

void control_manager::set_switch_to_xcpd_conn_passive()
{
	//cout << "Switch passive" <<endl;
    switch_to_xcpd_conn= PASSIVE_CONNECTION;
}
void control_manager::set_switch_to_xcpd_conn_active()
{
	//cout << "Switch active" <<endl;
    switch_to_xcpd_conn= ACTIVE_CONNECTION;
}
     
bool control_manager::is_switch_to_xcpd_conn_active()
{
	//cout << "switch is active "<< (switch_to_xcpd_conn == ACTIVE_CONNECTION) << endl;
    return (switch_to_xcpd_conn == ACTIVE_CONNECTION);
}

void control_manager::set_xcpd_to_control_conn_passive()
{
    //cout << "Controller connection PASSIVE" << endl;
    xcpd_to_control_conn= PASSIVE_CONNECTION;
}
    
void control_manager::set_xcpd_to_control_conn_active()
{
    //cout << "Controller connection ACTIVE" << endl;
    xcpd_to_control_conn= ACTIVE_CONNECTION;
}
        
bool control_manager::is_xcpd_to_control_conn_active()
{
    //cout << "Controller connection " << xcpd_to_control_conn << endl;
    //cout << "Which is " <<  (xcpd_to_control_conn == ACTIVE_CONNECTION) << endl;
    return (xcpd_to_control_conn == ACTIVE_CONNECTION);
}


int control_manager::no_vports()
{
    return ports.size();
}

void control_manager::add_vport(virtual_port vp) {
    ports.push_back(vp);
}

virtual_port control_manager::get_vport(int p) 
{
    return ports[p];
}

hardware_manager *control_manager::get_hardware_manager()
{
    return hm;
}

void control_manager::set_hardware_manager(hardware_manager *h,std::vector<std::string> parms)
{
    hm= h;
    h->init(parms);
}

int control_manager::get_queue_command_handling()
{
    return queue_command_handling;
}

void control_manager::set_queue_command_handling(int h)
{
    queue_command_handling= h;
}

int control_manager::get_port_stat_handling()
{
    return port_stat_handling;
}

void control_manager::set_port_stat_handling(int h) 
{
    port_stat_handling= h;
}

int control_manager::get_port_config_handling()
{
    return port_config_handling;
}

void control_manager::set_port_config_handling(int h)
{
    port_config_handling= h;
}
