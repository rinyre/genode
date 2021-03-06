/*
 * \brief  Simple UDP test client
 * \author Martin Stein
 * \date   2017-10-18
 */

/*
 * Copyright (C) 2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <timer_session/connection.h>
#include <base/attached_rom_dataspace.h>
#include <libc/component.h>
#include <nic/packet_allocator.h>
#include <util/string.h>

/* libc includes */
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

using namespace Genode;
using Ipv4_addr_str = Genode::String<16>;

void Libc::Component::construct(Libc::Env &env)
{
	/* wait a while for the server to come up */
	Timer::Connection timer(env);
	timer.msleep(4000);
	/* try to send and receive a message multiple times */
	for (unsigned trial_cnt = 0, success_cnt = 0; trial_cnt < 10; trial_cnt++)
	{
		timer.msleep(2000);

		/* create socket */
		int s = socket(AF_INET, SOCK_DGRAM, 0 );
		if (s < 0) {
			continue;
		}
		/* read server IP address and port */
		Ipv4_addr_str serv_addr;
		unsigned port = 0;
		Attached_rom_dataspace config(env, "config");
		Xml_node config_node = config.xml();
		try { config_node.attribute("server_ip").value(&serv_addr); }
		catch (...) {
		}
		try { config_node.attribute("server_port").value(&port); }
		catch (...) {
			continue;
		}
		/* create server socket address */
		struct sockaddr_in addr;
		socklen_t addr_sz = sizeof(addr);
		addr.sin_port = htons(port);
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr(serv_addr.string());

		/* send test message */
		enum { BUF_SZ = 1024 };
		char buf[BUF_SZ];
		::snprintf(buf, BUF_SZ, "UDP server at %s:%u", serv_addr.string(), port);
		if (sendto(s, buf, BUF_SZ, 0, (struct sockaddr*)&addr, addr_sz) != BUF_SZ) {
			continue;
		}
		/* receive and print what has been received */
		if (recvfrom(s, buf, BUF_SZ, 0, (struct sockaddr*)&addr, &addr_sz) != BUF_SZ) {
			continue;
		}
		log("Received \"", String<64>(buf), " ...\"");
		if (++success_cnt >= 5) {
			log("Test done");
			return;
		}
	}
	log("Test failed");
}
