/*  ES40 emulator.
 *
 *  This file is based upon GXemul.
 *
 *  Copyright (C) 2004-2007  Anders Gavare.  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright  
 *     notice, this list of conditions and the following disclaimer in the 
 *     documentation and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE   
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 *  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 *  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *  SUCH DAMAGE.
 */

/**
 * \file 
 * Contains the definitions for the emulated Network.
 *
 * X-1.2        Camiel Vanderhoeven                             15-NOV-2007
 *      Added newline at end to avoid warnings.
 *
 * X-1.1        Camiel Vanderhoeven                             14-NOV-2007
 *      Initial version for ES40 emulator.
 *
 * \author Camiel Vanderhoeven (camiel@camicom.com / http://www.camicom.com)
 **/

#include "StdAfx.h"
#include "telnet.h"
#include "Network.h"

/*
 *  net_init():
 *
 *  This function creates a network, and returns a pointer to it.
 *
 *  ipv4addr should be something like "10.0.0.0", netipv4len = 8.
 *
 *  On failure, exit() is called.
 */
CNetwork::CNetwork(int init_flags, const char *ipv4addr, int netipv4len)
{
	int res;

	/*  Sane defaults:  */
	timestamp = 0;
	first_ethernet_packet = last_ethernet_packet = NULL;

#ifdef HAVE_INET_PTON
	res = inet_pton(AF_INET, ipv4addr, &netmask_ipv4);
#else
	res = inet_aton(ipv4addr, &netmask_ipv4);
#endif
	if (res < 1) {
		fprintf(stderr, "net_init(): could not parse IPv4 address '%s'\n", ipv4addr);
		exit(1);
	}

	if (netipv4len < 1 || netipv4len > 30) {
		fprintf(stderr, "net_init(): extremely weird ipv4 network length (%i)\n", netipv4len);
		exit(1);
	}
	netmask_ipv4_len = netipv4len;

	if (init_flags & NET_INIT_FLAG_GATEWAY)
		net_gateway_init();

	net_dumpinfo();

	/*  This is necessary when using the real network:  */
#if !defined(_WIN32)
    signal(SIGPIPE, SIG_IGN);
#endif

}

CNetwork::~CNetwork(void)
{
}

/*
 *  net_ethernet_rx_avail():
 *
 *  Return 1 if there is a packet available , otherwise return 0.
 *
 *  Appart from actually checking for incoming packets from the outside world,
 *  this function basically works like net_ethernet_rx() but it only receives
 *  a return value telling us whether there is a packet or not, we don't
 *  actually get the packet.
 */
int CNetwork::net_ethernet_rx_avail()
{
	/*  IP protocol specific:  */
	net_udp_rx_avail();
	net_tcp_rx_avail();

	return net_ethernet_rx(NULL, NULL);
}

/*
 *  net_dumpinfo():
 *
 *  Called from the debugger's "machine" command, to print some info about
 *  a network.
 */
void CNetwork::net_dumpinfo()
{
	int iadd = DEBUG_INDENTATION;

	debug("net: simulating ");

	net_debugaddr(&netmask_ipv4, NET_ADDR_IPV4);
	debug("/%i", netmask_ipv4_len);

	debug(" (max outgoing: TCP=%i, UDP=%i)\n", MAX_TCP_CONNECTIONS, MAX_UDP_CONNECTIONS);

	debug_indentation(iadd);

	debug("simulated gateway: ");
	net_debugaddr(&gateway_ipv4_addr, NET_ADDR_IPV4);
	debug(" (");
	net_debugaddr(&gateway_ethernet_addr, NET_ADDR_ETHERNET);
	debug(")\n");

	debug_indentation(-iadd);
}

/*
 *  net_gateway_init():
 *
 *  This function creates a "gateway" machine (for example at IPv4 address
 *  10.0.0.254, if the net is 10.0.0.0/8), which acts as a gateway/router/
 *  nameserver etc.
 */
void CNetwork::net_gateway_init()
{
	unsigned char *p = (unsigned char *) &netmask_ipv4;
	uint32_t x;
	int xl;

	x = (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3];
	xl = 32 - netmask_ipv4_len;
	if (xl > 8)
		xl = 8;
	x |= ((1 << xl) - 1) & ~1;

	gateway_ipv4_addr[0] = (u8) (x >> 24);
	gateway_ipv4_addr[1] = (u8) (x >> 16);
	gateway_ipv4_addr[2] = (u8) (x >> 8);
	gateway_ipv4_addr[3] = (u8) x;

	gateway_ethernet_addr[0] = 0x60;
	gateway_ethernet_addr[1] = 0x50;
	gateway_ethernet_addr[2] = 0x40;
	gateway_ethernet_addr[3] = 0x30;
	gateway_ethernet_addr[4] = 0x20;
	gateway_ethernet_addr[5] = 0x10;
}

/*
 *  net_ethernet_tx():
 *
 *  Transmit an ethernet packet, as seen from the emulated ethernet controller.
 *  If the packet can be handled here, it will not necessarily be transmitted
 *  to the outside world.
 */
void CNetwork::net_ethernet_tx(unsigned char *packet, int len)
{
	int i, eth_type, for_the_gateway;

	for_the_gateway = !memcmp(packet, gateway_ethernet_addr, 6);

	/*  Drop too small packets:  */
	if (len < 20) {
		fatal("[ net_ethernet_tx: Warning: dropping tiny packet "
		    "(%i bytes) ]\n", len);
		return;
	}

	/*
	 *  The code below simulates the behaviour of a "NAT"-style gateway.
	 *
	 *  Packets that are not destined for the gateway are dropped first:
	 *  (DHCP packets are let through, though.)
	 */

	if (!for_the_gateway && packet[0] != 0xff && packet[0] != 0x00)
		return;

#if 0
	fatal("[ net: ethernet: ");
	for (i=0; i<6; i++)	fatal("%02x", packet[i]); fatal(" ");
	for (i=6; i<12; i++)	fatal("%02x", packet[i]); fatal(" ");
	for (i=12; i<14; i++)	fatal("%02x", packet[i]); fatal(" ");
	for (i=14; i<len; i++)	fatal("%02x", packet[i]); fatal(" ]\n");
#endif

	eth_type = (packet[12] << 8) + packet[13];

	/*  IP:  */
	if (eth_type == ETHERTYPE_IP) {
		/*  Routed via the gateway?  */
		if (for_the_gateway) {
			net_ip(packet, len);
			return;
		}

		/*  Broadcast? (DHCP does this.)  */
		if (packet[0] == 0xff && packet[1] == 0xff &&
		    packet[2] == 0xff && packet[3] == 0xff &&
		    packet[4] == 0xff && packet[5] == 0xff) {
			net_ip_broadcast(packet, len);
			return;
		}

		fatal("[ net_ethernet_tx: IP packet not for gateway, and not broadcast: ");
		for (i=0; i<14; i++)
			fatal("%02x", packet[i]);
		fatal(" ]\n");
		return;
	}

	/*  ARP:  */
	if (eth_type == ETHERTYPE_ARP) {
		if (len != 42 && len != 60)
			fatal("[ net_ethernet_tx: WARNING! unusual ARP len (%i) ]\n", len);
		net_arp(packet + 14, len - 14, 0);
		return;
	}

	/*  RARP:  */
	if (eth_type == ETHERTYPE_REVARP) {
		net_arp(packet + 14, len - 14, 1);
		return;
	}

	/*  Sprite:  */
	if (eth_type == ETHERTYPE_SPRITE) {
		/*  TODO.  */
		fatal("[ net: TX: UNIMPLEMENTED Sprite packet ]\n");
		return;
	}

	/*  IPv6:  */
	if (eth_type == ETHERTYPE_IPV6) {
		/*  TODO.  */
		fatal("[ net_ethernet_tx: IPv6 is not implemented yet! ]\n");
		return;
	}

	fatal("[ net_ethernet_tx: ethernet packet type 0x%04x not yet implemented ]\n", eth_type);
}

/*
 *  net_ethernet_rx():
 *
 *  Receive an ethernet packet. (This means handing over an already prepared
 *  packet from this module to a specific ethernet controller device.)
 *
 *  Return value is 1 if there was a packet available. *packetp and *lenp
 *  will be set to the packet's data pointer and length, respectively, and
 *  the packet will be removed from the linked list). If there was no packet
 *  available, 0 is returned.
 *
 *  If packetp is NULL, then the search is aborted as soon as a packet with
 *  the correct 'extra' field is found, and a 1 is returned, but as packetp
 *  is NULL we can't return the actual packet. (This is the internal form
 *  if net_ethernet_rx_avail().)
 */
int CNetwork::net_ethernet_rx(unsigned char **packetp, int *lenp)
{
	struct ethernet_packet_link *lp, *prev;

	lp = first_ethernet_packet;
	prev = NULL;
	while (lp != NULL) {
		if (packetp == NULL || lenp == NULL)
			return 1;

		/*  Let's return it:  */
		(*packetp) = lp->data;
		(*lenp) = lp->len;

		/*  Remove this link from the linked list:  */
		if (prev == NULL)
			first_ethernet_packet = lp->next;
		else
			prev->next = lp->next;

        if (lp->next == NULL)
			last_ethernet_packet = prev;
		else
			lp->next->prev = prev;

		free(lp);

		/*  ... and return successfully:  */
		return 1;
		
		prev = lp;
		lp = lp->next;
	}

	/*  No packet found. :-(  */
	return 0;
}

/*
 *  net_arp():
 *
 *  Handle an ARP (or RARP) packet, coming from the emulated NIC.
 *
 *  An ARP packet might look like this:
 *
 *	ARP header:
 *	    ARP hardware addr family:	0001
 *	    ARP protocol addr family:	0800
 *	    ARP addr lengths:		06 04
 *	    ARP request:		0001
 *	    ARP from:			112233445566 01020304
 *	    ARP to:			000000000000 01020301
 *
 *  An ARP request with a 'to' IP value of the gateway should cause an
 *  ARP response packet to be created.
 *
 *  An ARP request with the same from and to IP addresses should be ignored.
 *  (This would be a host testing to see if there is an IP collision.)
 */
void CNetwork::net_arp(unsigned char *packet, int len, int reverse)
{
	int q;
	int i;

	/*  TODO: This debug dump assumes ethernet->IPv4 translation:  */
	if (reverse)
		debug("[ net: RARP: ");
	else
		debug("[ net: ARP: ");
	for (i=0; i<2; i++)
		debug("%02x", packet[i]);
	debug(" ");
	for (i=2; i<4; i++)
		debug("%02x", packet[i]);
	debug(" ");
	debug("%02x", packet[4]);
	debug(" ");
	debug("%02x", packet[5]);
	debug(" req=");
	debug("%02x", packet[6]);	/*  Request type  */
	debug("%02x", packet[7]);
	debug(" from=");
	for (i=8; i<18; i++)
		debug("%02x", packet[i]);
	debug(" to=");
	for (i=18; i<28; i++)
		debug("%02x", packet[i]);
	debug(" ]\n");

	if (packet[0] == 0x00 && packet[1] == 0x01 &&
	    packet[2] == 0x08 && packet[3] == 0x00 &&
	    packet[4] == 0x06 && packet[5] == 0x04) {
		int r = (packet[6] << 8) + packet[7];
		struct ethernet_packet_link *lp;

		switch (r) {
		case 1:		/*  Request  */
			/*  Only create a reply if this was meant for the
			    gateway:  */
			if (memcmp(packet+24, gateway_ipv4_addr, 4) != 0)
				break;

			lp = net_allocate_ethernet_packet_link(60 + 14);

			/*  Copy the old packet first:  */
			memset(lp->data, 0, 60 + 14);
			memcpy(lp->data + 14, packet, len);

			/*  Add ethernet ARP header:  */
			memcpy(lp->data + 0, lp->data + 8 + 14, 6);
			memcpy(lp->data + 6, gateway_ethernet_addr, 6);
			lp->data[12] = 0x08; lp->data[13] = 0x06;

			/*  Address of the emulated machine:  */
			memcpy(lp->data + 18 + 14, lp->data + 8 + 14, 10);

			/*  Address of the gateway:  */
			memcpy(lp->data +  8 + 14, gateway_ethernet_addr, 6);
			memcpy(lp->data + 14 + 14, gateway_ipv4_addr, 4);

			/*  This is a Reply:  */
			lp->data[6 + 14] = 0x00; lp->data[7 + 14] = 0x02;

			break;
		case 3:		/*  Reverse Request  */
			lp = net_allocate_ethernet_packet_link(60 + 14);

			/*  Copy the old packet first:  */
			memset(lp->data, 0, 60 + 14);
			memcpy(lp->data + 14, packet, len);

			/*  Add ethernet RARP header:  */
			memcpy(lp->data + 0, packet + 8, 6);
			memcpy(lp->data + 6, gateway_ethernet_addr, 6);
			lp->data[12] = 0x80; lp->data[13] = 0x35;

			/*  This is a RARP reply:  */
			lp->data[6 + 14] = 0x00; lp->data[7 + 14] = 0x04;

			/*  Address of the gateway:  */
			memcpy(lp->data +  8 + 14, gateway_ethernet_addr, 6);
			memcpy(lp->data + 14 + 14, gateway_ipv4_addr, 4);

			/*  MAC address of emulated machine:  */
			memcpy(lp->data + 18 + 14, packet + 8, 6);

			/*
			 *  IP address of the emulated machine:  Automagically
			 *  generated from the MAC address. :-)
			 *
			 *  packet+8 points to the client's mac address,
			 *  for example 10:20:30:00:00:z0, where z is 0..15.
			 *  10:20:30:00:00:10 results in 10.0.0.1.
			 */
			/*  q = (packet[8 + 3]) >> 4;  */
			/*  q = q*15 + ((packet[8 + 4]) >> 4);  */
			q = (packet[8 + 5]) >> 4;
			lp->data[24 + 14] = 10;
			lp->data[25 + 14] =  0;
			lp->data[26 + 14] =  0;
			lp->data[27 + 14] =  q;
			break;
		case 2:		/*  Reply  */
		case 4:		/*  Reverse Reply  */
		default:
			fatal("[ net: ARP: UNIMPLEMENTED request type 0x%04x ]\n", r);
		}
	} else {
		fatal("[ net: ARP: UNIMPLEMENTED arp packet type: ");
		for (i=0; i<len; i++)
			fatal("%02x", packet[i]);
		fatal(" ]\n");
	}
}

/*
 *  net_allocate_ethernet_packet_link():
 *
 *  This routine allocates an ethernet_packet_link struct, and adds it at
 *  the end of the packet chain.  A data buffer is allocated, and the data,
 *  extra, and len fields of the link are set.
 *
 *  Note: The data buffer is not zeroed.
 *
 *  Return value is a pointer to the link on success. It doesn't return on
 *  failure.
 */
struct ethernet_packet_link *CNetwork::net_allocate_ethernet_packet_link(size_t len)
{
	struct ethernet_packet_link *lp;

	CHECK_ALLOCATION(lp = (struct ethernet_packet_link *)malloc(sizeof(struct ethernet_packet_link)));

	lp->len = len;
	CHECK_ALLOCATION(lp->data = (unsigned char *)malloc(len));

	lp->next = NULL;

	/*  Add last in the link chain:  */
	lp->prev = last_ethernet_packet;
	if (lp->prev != NULL)
		lp->prev->next = lp;
	else
		first_ethernet_packet = lp;
	last_ethernet_packet = lp;

	return lp;
}

/*
 *  net_udp_rx_avail():
 *
 *  Receive any available UDP packets (from the outside world).
 */
void CNetwork::net_udp_rx_avail()
{
	int received_packets_this_tick = 0;
	int max_packets_this_tick = 200;
	int con_id;

	for (con_id=0; con_id<MAX_UDP_CONNECTIONS; con_id++) {
		ssize_t res;
		unsigned char buf[66000];
		unsigned char udp_data[66008];
		struct sockaddr_in from;
		socklen_t from_len = sizeof(from);
		int ip_len, udp_len;
		struct ethernet_packet_link *lp;
		int max_per_packet;
		int bytes_converted = 0;
		int this_packets_data_length;
		int fragment_ofs = 0;

		if (received_packets_this_tick > max_packets_this_tick)
			break;

		if (!udp_connections[con_id].in_use)
			continue;

		if (udp_connections[con_id].socket < 0) {
			fatal("INTERNAL ERROR in net.c, udp socket < 0 "
			    "but in use?\n");
			continue;
		}

		res = recvfrom(udp_connections[con_id].socket, (char*)buf,
		    sizeof(buf), 0, (struct sockaddr *)&from, &from_len);

		/*  No more incoming UDP on this connection?  */
		if (res < 0)
			continue;

		timestamp ++;
		udp_connections[con_id].last_used_timestamp =
		    timestamp;

		udp_connections[con_id].udp_id ++;

		/*
		 *  Special case for the nameserver:  If a UDP packet is
		 *  received from the nameserver (if the nameserver's IP is
		 *  known), fake it so that it comes from the gateway instead.
		 */
		if (udp_connections[con_id].fake_ns)
			memcpy(((unsigned char *)(&from))+4,
			    &gateway_ipv4_addr[0], 4);

		/*
		 *  We now have a UDP packet of size 'res' which we need
		 *  turn into one or more ethernet packets for the emulated
		 *  operating system.  Ethernet packets are at most 1518
		 *  bytes long. With some margin, that means we can have
		 *  about 1500 bytes per packet.
		 *
		 *	Ethernet = 14 bytes
		 *	IP = 20 bytes
		 *	(UDP = 8 bytes + data)
		 *
		 *  So data can be at most max_per_packet - 34. For UDP
		 *  fragments, each multiple should (?) be a multiple of
		 *  8 bytes, except the last which doesn't have any such
		 *  restriction.
		 */
		max_per_packet = 1500;

		/*  UDP:  */
		udp_len = res + 8;
		/*  from[2..3] = outside_udp_port  */
		udp_data[0] = ((unsigned char *)&from)[2];
		udp_data[1] = ((unsigned char *)&from)[3];
		udp_data[2] = (udp_connections[con_id].
		    inside_udp_port >> 8) & 0xff;
		udp_data[3] = udp_connections[con_id].
		    inside_udp_port & 0xff;
		udp_data[4] = udp_len >> 8;
		udp_data[5] = udp_len & 0xff;
		udp_data[6] = 0;
		udp_data[7] = 0;
		memcpy(udp_data + 8, buf, res);
		/*
		 *  TODO:  UDP checksum, if necessary. At least NetBSD
		 *  and OpenBSD accept UDP packets with 0x0000 in the
		 *  checksum field anyway.
		 */

		while (bytes_converted < udp_len) {
			this_packets_data_length = udp_len - bytes_converted;

			/*  Do we need to fragment?  */
			if (this_packets_data_length > max_per_packet-34) {
				this_packets_data_length =
				    max_per_packet - 34;
				while (this_packets_data_length & 7)
					this_packets_data_length --;
			}

			ip_len = 20 + this_packets_data_length;

			lp = net_allocate_ethernet_packet_link(14 + 20 + this_packets_data_length);

			/*  Ethernet header:  */
			memcpy(lp->data + 0, udp_connections[con_id].ethernet_address, 6);
			memcpy(lp->data + 6, gateway_ethernet_addr, 6);
			lp->data[12] = 0x08;	/*  IP = 0x0800  */
			lp->data[13] = 0x00;

			/*  IP header:  */
			lp->data[14] = 0x45;	/*  ver  */
			lp->data[15] = 0x00;	/*  tos  */
			lp->data[16] = ip_len >> 8;
			lp->data[17] = ip_len & 0xff;
			lp->data[18] = udp_connections[con_id].udp_id >> 8;
			lp->data[19] = udp_connections[con_id].udp_id & 0xff;
			lp->data[20] = (fragment_ofs >> 8);
			if (bytes_converted + this_packets_data_length
			    < udp_len)
				lp->data[20] |= 0x20;	/*  More fragments  */
			lp->data[21] = fragment_ofs & 0xff;
			lp->data[22] = 0x40;	/*  ttl  */
			lp->data[23] = 17;	/*  p = UDP  */
			lp->data[26] = ((unsigned char *)&from)[4];
			lp->data[27] = ((unsigned char *)&from)[5];
			lp->data[28] = ((unsigned char *)&from)[6];
			lp->data[29] = ((unsigned char *)&from)[7];
			memcpy(lp->data + 30, udp_connections[con_id].
			    inside_ip_address, 4);
			net_ip_checksum(lp->data + 14, 10, 20);

			memcpy(lp->data+34, udp_data + bytes_converted,
			    this_packets_data_length);

			bytes_converted += this_packets_data_length;
			fragment_ofs = bytes_converted / 8;

			received_packets_this_tick ++;
		}

		/*  This makes sure we check this connection AGAIN
		    for more incoming UDP packets, before moving to the
		    next connection:  */
		con_id --;
	}
}

/*
 *  net_tcp_rx_avail():
 *
 *  Receive any available TCP packets (from the outside world).
 */
void CNetwork::net_tcp_rx_avail()
{
	int received_packets_this_tick = 0;
	int max_packets_this_tick = 200;
	int con_id;

	for (con_id=0; con_id<MAX_TCP_CONNECTIONS; con_id++) {
		unsigned char buf[66000];
		ssize_t res, res2;
		fd_set rfds;
		struct timeval tv;

		if (received_packets_this_tick > max_packets_this_tick)
			break;

		if (!tcp_connections[con_id].in_use)
			continue;

		if (tcp_connections[con_id].socket < 0) {
			fatal("INTERNAL ERROR in net.c, tcp socket < 0 but in use?\n");
			continue;
		}

		if (tcp_connections[con_id].incoming_buf == NULL)
			CHECK_ALLOCATION(tcp_connections[con_id].
			    incoming_buf = (unsigned char *) malloc(TCP_INCOMING_BUF_LEN));

		if (tcp_connections[con_id].state >=
		    TCP_OUTSIDE_DISCONNECTED)
			continue;

		/*  Is the socket available for output?  */
		FD_ZERO(&rfds);		/*  write  */
		FD_SET(tcp_connections[con_id].socket, &rfds);
		tv.tv_sec = tv.tv_usec = 0;
#if defined(_WIN32)
        WSASetLastError(0);
#else
		errno = 0;
#endif
		res = select(tcp_connections[con_id].socket+1,
		    NULL, &rfds, NULL, &tv);
#if defined(_WIN32)
        if (WSAGetLastError()==WSAECONNREFUSED) {
#else
		if (errno == ECONNREFUSED) {
#endif
			fatal("[ ECONNREFUSED: TODO ]\n");
			tcp_connections[con_id].state =
			    TCP_OUTSIDE_DISCONNECTED;
			fatal("CHANGING TO TCP_OUTSIDE_DISCONNECTED "
			    "(refused connection)\n");
			continue;
		}

#if defined(_WIN32)
        if (WSAGetLastError()==WSAETIMEDOUT) {
#else
		if (errno == ETIMEDOUT) {
#endif
			fatal("[ ETIMEDOUT: TODO ]\n");
			/*  TODO  */
			tcp_connections[con_id].state =
			    TCP_OUTSIDE_DISCONNECTED;
			fatal("CHANGING TO TCP_OUTSIDE_DISCONNECTED "
			    "(timeout)\n");
			continue;
		}

		if (tcp_connections[con_id].state == TCP_OUTSIDE_TRYINGTOCONNECT && res > 0) {
			tcp_connections[con_id].state = TCP_OUTSIDE_CONNECTED;
			debug("CHANGING TO TCP_OUTSIDE_CONNECTED\n");
			net_ip_tcp_connectionreply(con_id, 1, NULL, 0, 0);
		}

		if (tcp_connections[con_id].state == TCP_OUTSIDE_CONNECTED && res < 1) {
			continue;
		}

		/*
		 *  Does this connection have unacknowledged data?  Then, if
		 *  enough number of rounds have passed, try to resend it using
		 *  the old value of seqnr.
		 */
		if (tcp_connections[con_id].incoming_buf_len != 0) {
			tcp_connections[con_id].incoming_buf_rounds ++;
			if (tcp_connections[con_id].incoming_buf_rounds > 10000) {
				debug("  at seqnr %u but backing back to %u, resending %i bytes\n",
				    tcp_connections[con_id].outside_seqnr,
				    tcp_connections[con_id].incoming_buf_seqnr,
				    tcp_connections[con_id].incoming_buf_len);

				tcp_connections[con_id].incoming_buf_rounds = 0;
				tcp_connections[con_id].outside_seqnr = tcp_connections[con_id].incoming_buf_seqnr;

				net_ip_tcp_connectionreply(con_id, 0, tcp_connections[con_id].incoming_buf,
				    tcp_connections[con_id].incoming_buf_len, 0);
			}
			continue;
		}

		/*  Don't receive unless the guest OS is ready!  */
		if (((int32_t)tcp_connections[con_id].outside_seqnr - (int32_t)tcp_connections[con_id].inside_acknr) > 0) {
/*			fatal("YOYO 1! outside_seqnr - inside_acknr = %i\n",
			    tcp_connections[con_id].outside_seqnr -
			    tcp_connections[con_id].inside_acknr);  */
			continue;
		}

		/*  Is there incoming data available on the socket?  */
		FD_ZERO(&rfds);		/*  read  */
		FD_SET(tcp_connections[con_id].socket, &rfds);
		tv.tv_sec = tv.tv_usec = 0;
		res2 = select(tcp_connections[con_id].socket+1, &rfds, NULL, NULL, &tv);

		/*  No more incoming TCP data on this connection?  */
		if (res2 < 1)
			continue;

#if defined(_WIN32)
		res = recv(tcp_connections[con_id].socket, (char *)buf, 1400,0);
#else
		res = read(tcp_connections[con_id].socket, buf, 1400);
#endif
		if (res > 0) {
			/*  debug("\n -{- %lli -}-\n", (long long)res);  */
			tcp_connections[con_id].incoming_buf_len = res;
			tcp_connections[con_id].incoming_buf_rounds = 0;
			tcp_connections[con_id].incoming_buf_seqnr = 
			    tcp_connections[con_id].outside_seqnr;
			debug("  putting %i bytes (seqnr %u) in the incoming "
			    "buf\n", res, tcp_connections[con_id].
			    incoming_buf_seqnr);
			memcpy(tcp_connections[con_id].incoming_buf,
			    buf, res);

			net_ip_tcp_connectionreply(con_id, 0,
			    buf, res, 0);
		} else if (res == 0) {
			tcp_connections[con_id].state =
			    TCP_OUTSIDE_DISCONNECTED;
			debug("CHANGING TO TCP_OUTSIDE_DISCONNECTED, read"
			    " res=0\n");
			net_ip_tcp_connectionreply(con_id, 0,
			    NULL, 0, 0);
		} else {
			tcp_connections[con_id].state =
			    TCP_OUTSIDE_DISCONNECTED;
			fatal("CHANGING TO TCP_OUTSIDE_DISCONNECTED, "
			    "read res<=0, errno = %i\n", errno);
			net_ip_tcp_connectionreply(con_id, 0,
			    NULL, 0, 0);
		}

		timestamp ++;
		tcp_connections[con_id].last_used_timestamp =
		    timestamp;
	}
}

/*
 *  net_debugaddr():
 *
 *  Print an address using debug().
 */
void CNetwork::net_debugaddr(void *addr, int type)
{
	int i;
	unsigned char *p = (unsigned char *)addr;

	switch (type) {

	case NET_ADDR_IPV4:
		for (i=0; i<4; i++)
			debug("%s%i", i? "." : "", p[i]);
		break;

	case NET_ADDR_IPV6:
		for (i=0; i<16; i+=2)
			debug("%s%4x", i? ":" : "", p[i] * 256 + p[i+1]);
		break;

	case NET_ADDR_ETHERNET:
		for (i=0; i<6; i++)
			debug("%s%02x", i? ":" : "", p[i]);
		break;

	default:
		fatal("net_debugaddr(): UNIMPLEMTED type %i\n", type);
		exit(1);
	}
}

/*
 *  net_ip_broadcast():
 *
 *  Handle an IP broadcast packet, coming from the emulated NIC.
 *  (This is usually a DHCP request, or similar.)
 */
void CNetwork::net_ip_broadcast(unsigned char *packet, int len)
{
	unsigned char *p = (unsigned char *) &netmask_ipv4;
	uint32_t x, y;
	int i, xl, warning = 0, match = 0;

#if 0
	fatal("[ net: IP BROADCAST: ");
	fatal("ver=%02x ", packet[14]);
	fatal("tos=%02x ", packet[15]);
	fatal("len=%02x%02x ", packet[16], packet[17]);
	fatal("id=%02x%02x ",  packet[18], packet[19]);
	fatal("ofs=%02x%02x ", packet[20], packet[21]);
	fatal("ttl=%02x ", packet[22]);
	fatal("p=%02x ", packet[23]);
	fatal("sum=%02x%02x ", packet[24], packet[25]);
	fatal("src=%02x%02x%02x%02x ",
	    packet[26], packet[27], packet[28], packet[29]);
	fatal("dst=%02x%02x%02x%02x ",
	    packet[30], packet[31], packet[32], packet[33]);
	for (i=34; i<len; i++)
		fatal("%02x", packet[i]);
	fatal(" ]\n");
#endif

	/*  Check for 10.0.0.255 first, maybe some guest OSes think that
	    it's a /24 network, regardless of what it actually is.  */
	y = (packet[30] << 24) + (packet[31] << 16) +
	    (packet[32] << 8) + packet[33];

	x = (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3];
	/*  Example: x = 10.0.0.0  */
	x |= 255;

	if (x == y) {
		warning = 1;
		match = 1;
	}

	xl = 32 - netmask_ipv4_len;
	x |= (1 << xl) - 1;
	/*  x = 10.255.255.255  */

	if (x == y)
		match = 1;
	if (y == 0xffffffff)
		match = 1;

	if (warning)
		fatal("[ net_ip_broadcast(): warning: broadcast to "
		    "0x%08x, expecting broadcast to 0x%08x or "
		    "0xffffffff ]\n", y, x);

	/*  Cut off overflowing tail data:  */
	if (len > 14 + packet[16]*256 + packet[17])
		len = 14 + packet[16]*256 + packet[17];

	/*  Check for known packets:  */
	if (packet[14] == 0x45 &&			/*  IPv4  */
	    packet[23] == 0x11 &&			/*  UDP  */
	    packet[34] == 0 && packet[35] == 68 &&	/*  DHCP client  */
	    packet[36] == 0 && packet[37] == 67) {	/*  DHCP server  */
		net_ip_broadcast_dhcp(packet, len);
		return;
	}

	/*  Unknown packet:  */
	fatal("[ net: UNIMPLEMENTED IP BROADCAST: ");
	fatal("ver=%02x ", packet[14]);
	fatal("tos=%02x ", packet[15]);
	fatal("len=%02x%02x ", packet[16], packet[17]);
	fatal("id=%02x%02x ",  packet[18], packet[19]);
	fatal("ofs=%02x%02x ", packet[20], packet[21]);
	fatal("ttl=%02x ", packet[22]);
	fatal("p=%02x ", packet[23]);
	fatal("sum=%02x%02x ", packet[24], packet[25]);
	fatal("src=%02x%02x%02x%02x ",
	    packet[26], packet[27], packet[28], packet[29]);
	fatal("dst=%02x%02x%02x%02x ",
	    packet[30], packet[31], packet[32], packet[33]);
	for (i=34; i<len; i++)
		fatal("%02x", packet[i]);
	fatal(" ]\n");
}

/*
 *  net_ip():
 *
 *  Handle an IP packet, coming from the emulated NIC.
 */
void CNetwork::net_ip(unsigned char *packet, int len)
{
#if 1
	int i;

	debug("[ net: IP: ");
	debug("ver=%02x ", packet[14]);
	debug("tos=%02x ", packet[15]);
	debug("len=%02x%02x ", packet[16], packet[17]);
	debug("id=%02x%02x ",  packet[18], packet[19]);
	debug("ofs=%02x%02x ", packet[20], packet[21]);
	debug("ttl=%02x ", packet[22]);
	debug("p=%02x ", packet[23]);
	debug("sum=%02x%02x ", packet[24], packet[25]);
	debug("src=%02x%02x%02x%02x ",
	    packet[26], packet[27], packet[28], packet[29]);
	debug("dst=%02x%02x%02x%02x ",
	    packet[30], packet[31], packet[32], packet[33]);
	for (i=34; i<len; i++)
		debug("%02x", packet[i]);
	debug(" ]\n");
#endif

	/*  Cut off overflowing tail data:  */
	if (len > 14 + packet[16]*256 + packet[17])
		len = 14 + packet[16]*256 + packet[17];

	if (packet[14] == 0x45) {
		/*  IPv4:  */
		switch (packet[23]) {
		case 1:	/*  ICMP  */
			net_ip_icmp(packet, len);
			break;
		case 6:	/*  TCP  */
			net_ip_tcp(packet, len);
			break;
		case 17:/*  UDP  */
			net_ip_udp(packet, len);
			break;
		default:
			fatal("[ net: IP: UNIMPLEMENTED protocol %i ]\n",
			    packet[23]);
		}
	} else
		fatal("[ net: IP: UNIMPLEMENTED ip, first byte = 0x%02x ]\n",
		    packet[14]);
}

/*
 *  net_ip_broadcast_dhcp():
 *
 *  Handle an IPv4 DHCP broadcast packet, coming from the emulated NIC.
 *
 *  Read http://www.ietf.org/rfc/rfc2131.txt for details on DHCP.
 *  (And http://users.telenet.be/mydotcom/library/network/dhcp.htm.)
 */
void CNetwork::net_ip_broadcast_dhcp(unsigned char *packet, int len)
{
	/*
	 *  TODO
	 */
#if 1
	struct ethernet_packet_link *lp;
	int i;

	fatal("[ net: IPv4 DHCP: ");
#if 1
	fatal("ver=%02x ", packet[14]);
	fatal("tos=%02x ", packet[15]);
	fatal("len=%02x%02x ", packet[16], packet[17]);
	fatal("id=%02x%02x ",  packet[18], packet[19]);
	fatal("ofs=%02x%02x ", packet[20], packet[21]);
	fatal("ttl=%02x ", packet[22]);
	fatal("p=%02x ", packet[23]);
	fatal("sum=%02x%02x ", packet[24], packet[25]);
#endif
	fatal("src=%02x%02x%02x%02x ",
	    packet[26], packet[27], packet[28], packet[29]);
	fatal("dst=%02x%02x%02x%02x ",
	    packet[30], packet[31], packet[32], packet[33]);
#if 0
	for (i=34; i<len; i++)
		fatal("%02x", packet[i]);
#endif

	if (len < 34 + 8 + 236) {
		fatal("[ DHCP packet too short? Len=%i ]\n", len);
		return;
	}

	/*
	 *  UDP data (at offset 34):
	 *
	 *  srcport=0044 dstport=0043 length=0134 chksum=a973
	 *  data = 01010600d116d276000000000000000000000000000000
	 *         0000000000102030405060...0000...638253633501...000
	 */

	fatal("op=%02x ", packet[42]);
	fatal("htype=%02x ", packet[43]);
	fatal("hlen=%02x ", packet[44]);
	fatal("hops=%02x ", packet[45]);
	fatal("xid=%02x%02x%02x%02x ", packet[46], packet[47],
	    packet[48], packet[49]);
	fatal("secs=%02x%02x ", packet[50], packet[51]);
	fatal("flags=%02x%02x ", packet[52], packet[53]);
	fatal("ciaddr=%02x%02x%02x%02x ", packet[54], packet[55],
	    packet[56], packet[57]);
	fatal("yiaddr=%02x%02x%02x%02x ", packet[58], packet[59],
	    packet[60], packet[61]);
	fatal("siaddr=%02x%02x%02x%02x ", packet[62], packet[63],
	    packet[64], packet[65]);
	fatal("giaddr=%02x%02x%02x%02x ", packet[66], packet[67],
	    packet[68], packet[69]);
	fatal("chaddr=");
	for (i=70; i<70+16; i++)
		fatal("%02x", packet[i]);
	/*
   |                          sname   (64)                         |
   |                          file    (128)                        |
	 */
	fatal(" ]\n");

	lp = net_allocate_ethernet_packet_link(len);

	/*  Copy the old packet first:  */
	memcpy(lp->data, packet, len);

	/*  We are sending to the client, from the gateway:  */
	memcpy(lp->data + 0, packet + 6, 6);
	memcpy(lp->data + 6, gateway_ethernet_addr, 6);

	memcpy(lp->data + 26, &gateway_ipv4_addr[0], 4);
	lp->data[30] = 0xff;
	lp->data[31] = 0xff;
	lp->data[32] = 0xff;
	lp->data[33] = 0xff;

	/*  Switch src and dst ports:  */
	memcpy(lp->data + 34, packet + 36, 2);
	memcpy(lp->data + 36, packet + 34, 2);

	/*  Client's (yiaddr) IPv4 address:  */
	lp->data[58] = 10;
	lp->data[59] = 0;
	lp->data[60] = 0;
	lp->data[61] = 1;

	/*  Server's IPv4 address:  (giaddr)  */
	memcpy(lp->data + 66, &gateway_ipv4_addr[0], 4);

	/*  This is a Reply:  */
	lp->data[42] = 0x02;

	sprintf((char *)lp->data + 70+16+64, "es40em");

	/*  Recalculate IP header checksum:  */
	net_ip_checksum(lp->data + 14, 10, 20);

	/*  ... and the UDP checksum:  */
	net_ip_tcp_checksum(lp->data + 34, 6, len - 34 - 8,
	    lp->data + 26, lp->data + 30, 1);


    /*  Debug dump:  */
    packet = lp->data;
	fatal("[ net: IPv4 DHCP REPLY: ");
	for (i=0; i<14; i++)
		fatal("%02x", packet[i]);
	fatal("ver=%02x ", packet[14]);
	fatal("tos=%02x ", packet[15]);
	fatal("len=%02x%02x ", packet[16], packet[17]);
	fatal("id=%02x%02x ",  packet[18], packet[19]);
	fatal("ofs=%02x%02x ", packet[20], packet[21]);
	fatal("ttl=%02x ", packet[22]);
	fatal("p=%02x ", packet[23]);
	fatal("sum=%02x%02x ", packet[24], packet[25]);
	fatal("src=%02x%02x%02x%02x ",
	    packet[26], packet[27], packet[28], packet[29]);
	fatal("dst=%02x%02x%02x%02x ",
	    packet[30], packet[31], packet[32], packet[33]);
	fatal("op=%02x ", packet[42]);
	fatal("htype=%02x ", packet[43]);
	fatal("hlen=%02x ", packet[44]);
	fatal("hops=%02x ", packet[45]);
	fatal("xid=%02x%02x%02x%02x ", packet[46], packet[47],
	    packet[48], packet[49]);
	fatal("secs=%02x%02x ", packet[50], packet[51]);
	fatal("flags=%02x%02x ", packet[52], packet[53]);
	fatal("ciaddr=%02x%02x%02x%02x ", packet[54], packet[55],
	    packet[56], packet[57]);
	fatal("yiaddr=%02x%02x%02x%02x ", packet[58], packet[59],
	    packet[60], packet[61]);
	fatal("siaddr=%02x%02x%02x%02x ", packet[62], packet[63],
	    packet[64], packet[65]);
	fatal("giaddr=%02x%02x%02x%02x ", packet[66], packet[67],
	    packet[68], packet[69]);
	fatal("chaddr=");
	for (i=70; i<70+16; i++)
		fatal("%02x", packet[i]);
	fatal(" ]\n");

#endif
}

/*
 *  net_ip_udp():
 *
 *  Handle a UDP packet.
 *
 *  (See http://www.networksorcery.com/enp/protocol/udp.htm.)
 *
 *  The IP header (at offset 14) could look something like
 *
 *	ver=45 tos=00 len=003c id=0006 ofs=0000 ttl=40 p=11 sum=b798
 *	src=0a000001 dst=c1abcdef
 *
 *  and the UDP data (beginning at offset 34):
 *
 *	srcport=fffc dstport=0035 length=0028 chksum=76b6
 *	43e20100000100000000000003667470066e6574627364036f726700001c0001
 */
void CNetwork::net_ip_udp(unsigned char *packet, int len)
{
	int con_id, free_con_id, i, srcport, dstport, udp_len;
	ssize_t res;
	struct sockaddr_in remote_ip;

	if ((packet[20] & 0x3f) != 0) {
		fatal("[ net_ip_udp(): WARNING! fragmented UDP "
		    "packet, TODO ]\n");
		return;
	}

	srcport = (packet[34] << 8) + packet[35];
	dstport = (packet[36] << 8) + packet[37];
	udp_len = (packet[38] << 8) + packet[39];
	/*  chksum at offset 40 and 41  */

	debug("[ net: UDP: ");
	debug("srcport=%i dstport=%i len=%i ", srcport, dstport, udp_len);
	for (i=42; i<len; i++) {
		if (packet[i] >= ' ' && packet[i] < 127)
			debug("%c", packet[i]);
		else
			debug("[%02x]", packet[i]);
	}
	debug(" ]\n");

	/*  Is this "connection" new, or a currently ongoing one?  */
	con_id = free_con_id = -1;
	for (i=0; i<MAX_UDP_CONNECTIONS; i++) {
		if (!udp_connections[i].in_use)
			free_con_id = i;
		if (udp_connections[i].in_use &&
		    udp_connections[i].inside_udp_port == srcport &&
		    udp_connections[i].outside_udp_port == dstport &&
		    memcmp(udp_connections[i].inside_ip_address,
			packet + 26, 4) == 0 &&
		    memcmp(udp_connections[i].outside_ip_address,
			packet + 30, 4) == 0) {
			con_id = i;
			break;
		}
	}

	debug("&& UDP connection is ");
	if (con_id >= 0)
		debug("ONGOING");
	else {
		debug("NEW");
		if (free_con_id < 0) {
			int i;
			int64_t oldest = 
			    udp_connections[0].last_used_timestamp;
			free_con_id = 0;

			debug(", NO FREE SLOTS, REUSING OLDEST ONE");
			for (i=0; i<MAX_UDP_CONNECTIONS; i++)
				if (udp_connections[i].
				    last_used_timestamp < oldest) {
					oldest = udp_connections[i].
					    last_used_timestamp;
					free_con_id = i;
				}
#if defined(_WIN32)
			closesocket(udp_connections[free_con_id].socket);
#else
			close(udp_connections[free_con_id].socket);
#endif
		}
		con_id = free_con_id;
		memset(&udp_connections[con_id], 0,
		    sizeof(struct udp_connection));

		memcpy(udp_connections[con_id].ethernet_address,
		    packet + 6, 6);
		memcpy(udp_connections[con_id].inside_ip_address,
		    packet + 26, 4);
		udp_connections[con_id].inside_udp_port = srcport;
		memcpy(udp_connections[con_id].outside_ip_address,
		    packet + 30, 4);
		udp_connections[con_id].outside_udp_port = dstport;

		udp_connections[con_id].socket = socket(AF_INET,
		    SOCK_DGRAM, 0);
		if (udp_connections[con_id].socket < 0) {
			fatal("[ net: UDP: socket() returned %i ]\n",
			    udp_connections[con_id].socket);
			return;
		}

		debug(" {socket=%i}", udp_connections[con_id].socket);

		udp_connections[con_id].in_use = 1;

		/*  Set the socket to non-blocking:  */
#if defined(_WIN32)
        {
          unsigned long nb_argval = 1;
          ioctlsocket(udp_connections[con_id].socket,FIONBIO,&nb_argval);
        }
#else
		res = fcntl(udp_connections[con_id].socket, F_GETFL);
		fcntl(udp_connections[con_id].socket, F_SETFL, res | O_NONBLOCK);
#endif
    }

	debug(", connection id %i\n", con_id);

	timestamp ++;
	udp_connections[con_id].last_used_timestamp = timestamp;

	remote_ip.sin_family = AF_INET;
	memcpy((unsigned char *)&remote_ip.sin_addr,
	    udp_connections[con_id].outside_ip_address, 4);

	remote_ip.sin_port = htons(
	    udp_connections[con_id].outside_udp_port);

	res = sendto(udp_connections[con_id].socket, (char*)packet + 42,
	    len - 42, 0, (const struct sockaddr *)&remote_ip,
	    sizeof(remote_ip));

	if (res != len-42)
		debug("[ net: UDP: unable to send %i bytes ]\n", len-42);
	else
		debug("[ net: UDP: OK!!! ]\n");
}

/*
 *  net_ip_tcp():
 *
 *  Handle a TCP packet comming from the emulated OS.
 *
 *  The IP header (at offset 14) could look something like
 *
 *	ver=45 tos=00 len=003c id=0006 ofs=0000 ttl=40 p=11 sum=b798
 *	src=0a000001 dst=c1abcdef
 *
 *  TCP header, at offset 34:
 *
 *	srcport=fffe dstport=0015 seqnr=af419a1d acknr=00000000
 *	control=a002 window=4000 checksum=fe58 urgent=0000
 *	and then "options and padding" and then data.
 *	(020405b4010303000101080a0000000000000000)
 *
 *  See the following URLs for good descriptions of TCP:
 *
 *	http://www.networksorcery.com/enp/protocol/tcp.htm
 *	http://www.tcpipguide.com/free/t_TCPIPTransmissionControlProtocolTCP.htm
 */
void CNetwork::net_ip_tcp(unsigned char *packet, int len)
{
	int con_id, free_con_id, i, res;
	int srcport, dstport, data_offset, window, checksum, urgptr;
	int syn, ack, psh, rst, urg, fin;
	uint32_t seqnr, acknr;
	struct sockaddr_in remote_ip;
	fd_set rfds;
	struct timeval tv;
	int send_ofs;

#if 0
	fatal("[ net: TCP: ");
	for (i=0; i<26; i++)
		fatal("%02x", packet[i]);
	fatal(" ");
#endif

	srcport = (packet[34] << 8) + packet[35];
	dstport = (packet[36] << 8) + packet[37];

	seqnr   = (packet[38] << 24) + (packet[39] << 16)
		+ (packet[40] << 8) + packet[41];
	acknr   = (packet[42] << 24) + (packet[43] << 16)
		+ (packet[44] << 8) + packet[45];

#if 0
	fatal("%i.%i.%i.%i:%i -> %i.%i.%i.%i:%i, seqnr=%lli acknr=%lli ",
	    packet[26], packet[27], packet[28], packet[29], srcport,
	    packet[30], packet[31], packet[32], packet[33], dstport,
	    (long long)seqnr, (long long)acknr);
#endif

	data_offset = (packet[46] >> 4) * 4 + 34;
	/*  data_offset is now data offset within packet :-)  */

	urg = packet[47] & 32;
	ack = packet[47] & 16;
	psh = packet[47] &  8;
	rst = packet[47] &  4;
	syn = packet[47] &  2;
	fin = packet[47] &  1;
	window   = (packet[48] << 8) + packet[49];
	checksum = (packet[50] << 8) + packet[51];
	urgptr   = (packet[52] << 8) + packet[53];

#if 0
	fatal(urg? "URG " : "");
	fatal(ack? "ACK " : "");
	fatal(psh? "PSH " : "");
	fatal(rst? "RST " : "");
	fatal(syn? "SYN " : "");
	fatal(fin? "FIN " : "");

	fatal("window=0x%04x checksum=0x%04x urgptr=0x%04x ",
	    window, checksum, urgptr);

	fatal("options=");
	for (i=34+20; i<data_offset; i++)
		fatal("%02x", packet[i]);

	fatal(" data=");
	for (i=data_offset; i<len; i++)
		fatal("%02x", packet[i]);

	fatal(" ]\n");
#endif

	net_ip_tcp_checksum(packet + 34, 16, len - 34,
		packet + 26, packet + 30, 0);
	if (packet[50] * 256 + packet[51] != checksum) {
		debug("TCP: dropping packet because of checksum mismatch "
		    "(0x%04x != 0x%04x)\n", packet[50] * 256 + packet[51],
		    checksum);

		return;
	}

	/*  Does this packet belong to a current connection?  */
	con_id = free_con_id = -1;
	for (i=0; i<MAX_TCP_CONNECTIONS; i++) {
		if (!tcp_connections[i].in_use)
			free_con_id = i;
		if (tcp_connections[i].in_use &&
		    tcp_connections[i].inside_tcp_port == srcport &&
		    tcp_connections[i].outside_tcp_port == dstport &&
		    memcmp(tcp_connections[i].inside_ip_address,
			packet + 26, 4) == 0 &&
		    memcmp(tcp_connections[i].outside_ip_address,
			packet + 30, 4) == 0) {
			con_id = i;
			break;
		}
	}

	/*
	 *  Unknown connection, and not SYN? Then drop the packet.
	 *  TODO:  Send back RST?
	 */
	if (con_id < 0 && !syn) {
		debug("[ net: TCP: dropping packet from unknown connection,"
		    " %i.%i.%i.%i:%i -> %i.%i.%i.%i:%i %s%s%s%s%s]\n",
		    packet[26], packet[27], packet[28], packet[29], srcport,
		    packet[30], packet[31], packet[32], packet[33], dstport,
		    fin? "FIN ": "", syn? "SYN ": "", ack? "ACK ": "",
		    psh? "PSH ": "", rst? "RST ": "");
		return;
	}

	/*  Known connection, and SYN? Then ignore the packet.  */
	if (con_id >= 0 && syn) {
		debug("[ net: TCP: ignoring redundant SYN packet from known"
		    " connection, %i.%i.%i.%i:%i -> %i.%i.%i.%i:%i ]\n",
		    packet[26], packet[27], packet[28], packet[29], srcport,
		    packet[30], packet[31], packet[32], packet[33], dstport);
		return;
	}

	/*
	 *  A new outgoing connection?
	 */
	if (con_id < 0 && syn) {
		debug("[ net: TCP: new outgoing connection, %i.%i.%i.%i:%i"
		    " -> %i.%i.%i.%i:%i ]\n",
		    packet[26], packet[27], packet[28], packet[29], srcport,
		    packet[30], packet[31], packet[32], packet[33], dstport);

		/*  Find a free connection id to use:  */
		if (free_con_id < 0) {
#if 1
			/*
			 *  TODO:  Reuse the oldest one currently in use, or
			 *  just drop the new connection attempt? Drop for now.
			 */
			fatal("[ TOO MANY TCP CONNECTIONS IN USE! "
			    "Increase MAX_TCP_CONNECTIONS! ]\n");
			return;
#else
			int i;
			int64_t oldest = 
			    tcp_connections[0].last_used_timestamp;
			free_con_id = 0;

			fatal("[ NO FREE TCP SLOTS, REUSING OLDEST ONE ]\n");
			for (i=0; i<MAX_TCP_CONNECTIONS; i++)
				if (tcp_connections[i].
				    last_used_timestamp < oldest) {
					oldest = tcp_connections[i].
					    last_used_timestamp;
					free_con_id = i;
				}
			tcp_closeconnection(free_con_id);
#endif
		}

		con_id = free_con_id;
		memset(&tcp_connections[con_id], 0,
		    sizeof(struct tcp_connection));

		memcpy(tcp_connections[con_id].ethernet_address,
		    packet + 6, 6);
		memcpy(tcp_connections[con_id].inside_ip_address,
		    packet + 26, 4);
		tcp_connections[con_id].inside_tcp_port = srcport;
		memcpy(tcp_connections[con_id].outside_ip_address,
		    packet + 30, 4);
		tcp_connections[con_id].outside_tcp_port = dstport;

		tcp_connections[con_id].socket =
		    socket(AF_INET, SOCK_STREAM, 0);
		if (tcp_connections[con_id].socket < 0) {
			fatal("[ net: TCP: socket() returned %i ]\n",
			    tcp_connections[con_id].socket);
			return;
		}

		debug("[ new tcp outgoing socket=%i ]\n",
		    tcp_connections[con_id].socket);

		tcp_connections[con_id].in_use = 1;

		/*  Set the socket to non-blocking:  */
#if defined(_WIN32)
        {
          unsigned long nb_argval = 1;
          ioctlsocket(tcp_connections[con_id].socket,FIONBIO,&nb_argval);
        }
#else
		res = fcntl(tcp_connections[con_id].socket, F_GETFL);
		fcntl(tcp_connections[con_id].socket, F_SETFL, res | O_NONBLOCK);
#endif

		remote_ip.sin_family = AF_INET;
		memcpy((unsigned char *)&remote_ip.sin_addr,
		    tcp_connections[con_id].outside_ip_address, 4);
		remote_ip.sin_port = htons(
		    tcp_connections[con_id].outside_tcp_port);

		res = connect(tcp_connections[con_id].socket,
		    (struct sockaddr *)&remote_ip, sizeof(remote_ip));

		/*  connect can return -1, and errno = EINPROGRESS
		    as we might not have connected right away.  */

		tcp_connections[con_id].state = TCP_OUTSIDE_TRYINGTOCONNECT;

		tcp_connections[con_id].outside_acknr = 0;
		tcp_connections[con_id].outside_seqnr =((rand() & 0xffff) << 16) + (rand() & 0xffff);
	}

	if (rst) {
		debug("[ 'rst': disconnecting TCP connection %i ]\n", con_id);
		net_ip_tcp_connectionreply(con_id, 0, NULL, 0, 1);
		tcp_closeconnection(con_id);
		return;
	}

	if (ack && tcp_connections[con_id].state
	    == TCP_OUTSIDE_DISCONNECTED2) {
		debug("[ 'ack': guestOS's final termination of TCP connection %i ]\n", con_id);

		/*  Send an RST?  (TODO, this is wrong...)  */
		net_ip_tcp_connectionreply(con_id, 0, NULL, 0, 1);

		/*  ... and forget about this connection:  */
		tcp_closeconnection(con_id);
		return;
	}

	if (fin && tcp_connections[con_id].state
	    == TCP_OUTSIDE_DISCONNECTED) {
		debug("[ 'fin': response to outside's disconnection of TCP connection %i ]\n", con_id);

		/*  Send an ACK:  */
		tcp_connections[con_id].state = TCP_OUTSIDE_CONNECTED;
		net_ip_tcp_connectionreply(con_id, 0, NULL, 0, 0);
		tcp_connections[con_id].state = TCP_OUTSIDE_DISCONNECTED2;
		return;
	}

	if (fin) {
		debug("[ 'fin': guestOS disconnecting TCP connection %i ]\n",
		    con_id);

		/*  Send ACK:  */
		net_ip_tcp_connectionreply(con_id, 0, NULL, 0, 0);
		tcp_connections[con_id].state = TCP_OUTSIDE_DISCONNECTED2;

		/*  Return and send FIN:  */
		goto ret;
	}

	if (ack) {
debug("ACK %i bytes, inside_acknr=%u outside_seqnr=%u\n",
 tcp_connections[con_id].incoming_buf_len,
 tcp_connections[con_id].inside_acknr,
 tcp_connections[con_id].outside_seqnr);
		tcp_connections[con_id].inside_acknr = acknr;
		if (tcp_connections[con_id].inside_acknr ==
		    tcp_connections[con_id].outside_seqnr &&
		    tcp_connections[con_id].incoming_buf_len != 0) {
debug("  all acked\n");
			tcp_connections[con_id].incoming_buf_len = 0;
		}
	}

	tcp_connections[con_id].inside_seqnr = seqnr;

	/*  TODO: This is hardcoded for a specific NetBSD packet:  */
	if (packet[34 + 30] == 0x08 && packet[34 + 31] == 0x0a)
		tcp_connections[con_id].inside_timestamp =
		    (packet[34 + 32 + 0] << 24) +
		    (packet[34 + 32 + 1] << 16) +
		    (packet[34 + 32 + 2] <<  8) +
		    (packet[34 + 32 + 3] <<  0);


	timestamp ++;
	tcp_connections[con_id].last_used_timestamp = timestamp;


	if (tcp_connections[con_id].state != TCP_OUTSIDE_CONNECTED) {
		debug("[ not connected to outside ]\n");
		return;
	}


	if (data_offset >= len)
		return;


	/*
	 *  We are here if this is a known connection, and data is to be
	 *  transmitted to the outside world.
	 */

	send_ofs = data_offset;
	send_ofs += ((int32_t)tcp_connections[con_id].outside_acknr
	    - (int32_t)seqnr);
#if 1
	debug("[ %i bytes of tcp data to be sent, beginning at seqnr %u, ",
	    len - data_offset, seqnr);
	debug("outside is at acknr %u ==> %i actual bytes to be sent ]\n",
	    tcp_connections[con_id].outside_acknr, len - send_ofs);
#endif

	/*  Drop outgoing packet if the guest OS' seqnr is not
	    the same as we have acked. (We have missed something, perhaps.)  */
	if (seqnr != tcp_connections[con_id].outside_acknr) {
		debug("!! outgoing TCP packet dropped (seqnr = %u, "
		    "outside_acknr = %u)\n", seqnr,
		    tcp_connections[con_id].outside_acknr);
		goto ret;
	}

	if (len - send_ofs > 0) {
		/*  Is the socket available for output?  */
		FD_ZERO(&rfds);		/*  write  */
		FD_SET(tcp_connections[con_id].socket, &rfds);
		tv.tv_sec = tv.tv_usec = 0;
		errno = 0;
		res = select(tcp_connections[con_id].socket+1,
		    NULL, &rfds, NULL, &tv);
		if (res < 1) {
			tcp_connections[con_id].state =
			    TCP_OUTSIDE_DISCONNECTED;
			debug("[ TCP: disconnect on select for writing ]\n");
			goto ret;
		}

#if defined(_WIN32)
        res = send(tcp_connections[con_id].socket,(char*)packet + send_ofs, len - send_ofs, 0);
#else
		res = write(tcp_connections[con_id].socket, packet + send_ofs, len - send_ofs);
#endif

		if (res > 0) {
			tcp_connections[con_id].outside_acknr += res;
		} else if (errno == EAGAIN) {
			/*  Just ignore this attempt.  */
			return;
		} else {
			debug("[ error writing %i bytes to TCP connection %i:"
			    " errno = %i ]\n", len - send_ofs, con_id, errno);
			tcp_connections[con_id].state =
			    TCP_OUTSIDE_DISCONNECTED;
			debug("[ TCP: disconnect on write() ]\n");
			goto ret;
		}
	}

ret:
	/*  Send an ACK (or FIN) to the guest OS:  */
	net_ip_tcp_connectionreply(con_id, 0, NULL, 0, 0);
}

/*
 *  net_ip_tcp_connectionreply():
 *
 *  When changing from state _TRYINGTOCONNECT to _CONNECTED, then this
 *  function should be called with connecting set to 1.
 *
 *  To send a generic ack reply, set connecting to 0.
 *
 *  To send data (PSH), set data to non-NULL and datalen to the length.
 *
 *  This creates an ethernet packet for the guest OS with an ACK to the
 *  initial SYN packet.
 */
void CNetwork::net_ip_tcp_connectionreply(int con_id, int connecting, unsigned char *data, int datalen, int rst)
{
	struct ethernet_packet_link *lp;
	int tcp_length, ip_len, option_len = 20;

	if (connecting)
		tcp_connections[con_id].outside_acknr = tcp_connections[con_id].inside_seqnr + 1;

	tcp_connections[con_id].tcp_id ++;
	tcp_length = 20 + option_len + datalen;
	ip_len = 20 + tcp_length;
	lp = net_allocate_ethernet_packet_link(14 + ip_len);

	/*  Ethernet header:  */
	memcpy(lp->data + 0, tcp_connections[con_id].ethernet_address, 6);
	memcpy(lp->data + 6, gateway_ethernet_addr, 6);
	lp->data[12] = 0x08;	/*  IP = 0x0800  */
	lp->data[13] = 0x00;

	/*  IP header:  */
	lp->data[14] = 0x45;	/*  ver  */
	lp->data[15] = 0x10;	/*  tos  */
	lp->data[16] = ip_len >> 8;
	lp->data[17] = ip_len & 0xff;
	lp->data[18] = tcp_connections[con_id].tcp_id >> 8;
	lp->data[19] = tcp_connections[con_id].tcp_id & 0xff;
	lp->data[20] = 0x40;	/*  don't fragment  */
	lp->data[21] = 0x00;
	lp->data[22] = 0x40;	/*  ttl  */
	lp->data[23] = 6;	/*  p = TCP  */
	memcpy(lp->data + 26, tcp_connections[con_id].
	    outside_ip_address, 4);
	memcpy(lp->data + 30, tcp_connections[con_id].
	    inside_ip_address, 4);
	net_ip_checksum(lp->data + 14, 10, 20);

	/*  TCP header and options at offset 34:  */
	lp->data[34] = tcp_connections[con_id].outside_tcp_port >> 8;
	lp->data[35] = tcp_connections[con_id].outside_tcp_port & 0xff;
	lp->data[36] = tcp_connections[con_id].inside_tcp_port >> 8;
	lp->data[37] = tcp_connections[con_id].inside_tcp_port & 0xff;
	lp->data[38] = (u8) (tcp_connections[con_id].
	    outside_seqnr >> 24) & 0xff;
	lp->data[39] = (u8) (tcp_connections[con_id].
	    outside_seqnr >> 16) & 0xff;
	lp->data[40] = (u8) (tcp_connections[con_id].
	    outside_seqnr >>  8) & 0xff;
	lp->data[41] = (u8) tcp_connections[con_id].
	    outside_seqnr & 0xff;
	lp->data[42] = (u8) (tcp_connections[con_id].
	    outside_acknr >> 24) & 0xff;
	lp->data[43] = (u8) (tcp_connections[con_id].
	    outside_acknr >> 16) & 0xff;
	lp->data[44] = (u8) (tcp_connections[con_id].
	    outside_acknr >>  8) & 0xff;
	lp->data[45] = (u8) tcp_connections[con_id].outside_acknr & 0xff;

	/*  Control  */
	lp->data[46] = (option_len + 20) / 4 * 0x10;
	lp->data[47] = 0x10;	/*  ACK  */
	if (connecting)
		lp->data[47] |= 0x02;	/*  SYN  */
	if (tcp_connections[con_id].state == TCP_OUTSIDE_CONNECTED)
		lp->data[47] |= 0x08;	/*  PSH  */
	if (rst)
		lp->data[47] |= 0x04;	/*  RST  */
	if (tcp_connections[con_id].state >= TCP_OUTSIDE_DISCONNECTED)
		lp->data[47] |= 0x01;	/*  FIN  */

	/*  Window  */
	lp->data[48] = 0x10;
	lp->data[49] = 0x00;

	/*  no urgent ptr  */

	/*  options  */
	/*  TODO:  HAHA, this is ugly  */
	lp->data[54] = 0x02;
	lp->data[55] = 0x04;
	lp->data[56] = 0x05;
	lp->data[57] = 0xb4;
	lp->data[58] = 0x01;
	lp->data[59] = 0x03;
	lp->data[60] = 0x03;
	lp->data[61] = 0x00;
	lp->data[62] = 0x01;
	lp->data[63] = 0x01;
	lp->data[64] = 0x08;
	lp->data[65] = 0x0a;
	lp->data[66] = (u8) (timestamp >> 24) & 0xff;
	lp->data[67] = (u8) (timestamp >> 16) & 0xff;
	lp->data[68] = (u8) (timestamp >> 8) & 0xff;
	lp->data[69] = (u8) timestamp & 0xff;
	lp->data[70] = (u8) (tcp_connections[con_id].
	    inside_timestamp >> 24) & 0xff;
	lp->data[71] = (u8) (tcp_connections[con_id].
	    inside_timestamp >> 16) & 0xff;
	lp->data[72] = (u8) (tcp_connections[con_id].
	    inside_timestamp >> 8) & 0xff;
	lp->data[73] = (u8) tcp_connections[con_id].
	    inside_timestamp & 0xff;

	/*  data:  */
	if (data != NULL) {
		memcpy(lp->data + 74, data, datalen);
		tcp_connections[con_id].outside_seqnr += datalen;
	}

	/*  Checksum:  */
	net_ip_tcp_checksum(lp->data + 34, 16, tcp_length,
	    lp->data + 26, lp->data + 30, 0);

#if 0
	{
		int i;
		fatal("[ net_ip_tcp_connectionreply(%i): ", connecting);
		for (i=0; i<ip_len+14; i++)
			fatal("%02x", lp->data[i]);
		fatal(" ]\n");
	}
#endif

	if (connecting)
		tcp_connections[con_id].outside_seqnr ++;
}

/*
 *  tcp_closeconnection():
 *
 *  Helper function which closes down a TCP connection completely.
 */
void CNetwork::tcp_closeconnection(int con_id)
{
#if defined(_WIN32)
	closesocket(tcp_connections[con_id].socket);
#else
	close(tcp_connections[con_id].socket);
#endif
	tcp_connections[con_id].state = TCP_OUTSIDE_DISCONNECTED;
	tcp_connections[con_id].in_use = 0;
	tcp_connections[con_id].incoming_buf_len = 0;
}

/*
 *  net_ip_icmp():
 *
 *  Handle an ICMP packet.
 *
 *  The IP header (at offset 14) could look something like
 *
 *	ver=45 tos=00 len=0054 id=001a ofs=0000 ttl=ff p=01 sum=a87e
 *	src=0a000005 dst=03050607
 *
 *  and the ICMP specific data (beginning at offset 34):
 *
 *	type=08 code=00 chksum=b8bf
 *	000c0008d5cee94089190c0008090a0b
 *	0c0d0e0f101112131415161718191a1b
 *	1c1d1e1f202122232425262728292a2b
 *	2c2d2e2f3031323334353637
 */
void CNetwork::net_ip_icmp(unsigned char *packet, int len)
{
	int type;
	struct ethernet_packet_link *lp;

	type = packet[34];

	switch (type) {
	case 8:	/*  ECHO request  */
		debug("[ ICMP echo ]\n");
		lp = net_allocate_ethernet_packet_link(len);

		/*  Copy the old packet first:  */
		memcpy(lp->data + 12, packet + 12, len - 12);

		/*  Switch to and from ethernet addresses:  */
		memcpy(lp->data + 0, packet + 6, 6);
		memcpy(lp->data + 6, packet + 0, 6);

		/*  Switch to and from IP addresses:  */
		memcpy(lp->data + 26, packet + 30, 4);
		memcpy(lp->data + 30, packet + 26, 4);

		/*  Change from echo REQUEST to echo REPLY:  */
		lp->data[34] = 0x00;

		/*  Decrease the TTL to a low value:  */
		lp->data[22] = 2;

		/*  Recalculate ICMP checksum:  */
		net_ip_checksum(lp->data + 34, 2, len - 34);

		/*  Recalculate IP header checksum:  */
		net_ip_checksum(lp->data + 14, 10, 20);

		break;
	default:
		fatal("[ net: ICMP type %i not yet implemented ]\n", type);
	}
}

/*
 *  net_ip_checksum():
 *
 *  Fill in an IP header checksum. (This works for ICMP too.)
 *  chksumoffset should be 10 for IP headers, and len = 20.
 *  For ICMP packets, chksumoffset = 2 and len = length of the ICMP packet.
 */
void CNetwork::net_ip_checksum(unsigned char *ip_header, int chksumoffset, int len)
{
	int i;
	uint32_t sum = 0;

	for (i=0; i<len; i+=2)
		if (i != chksumoffset) {
			uint16_t w = (ip_header[i] << 8) + ip_header[i+1];
			sum += w;
			while (sum > 65535) {
				int to_add = sum >> 16;
				sum = (sum & 0xffff) + to_add;
			}
		}

	sum ^= 0xffff;
	ip_header[chksumoffset + 0] = (u8) (sum >> 8);
	ip_header[chksumoffset + 1] = (u8) (sum & 0xff);
}


/*
 *  net_ip_tcp_checksum():
 *
 *  Fill in a TCP header checksum. This differs slightly from the IP
 *  checksum. The checksum is calculated on a pseudo header, the actual
 *  TCP header, and the data.  This is what the pseudo header looks like:
 *
 *	uint32_t srcaddr;
 *	uint32_t dstaddr;
 *	uint16_t protocol; (= 6 for tcp)
 *	uint16_t tcp_len;
 *
 *  tcp_len is length of header PLUS data.  The psedo header is created
 *  internally here, and does not need to be supplied by the caller.
 */
void CNetwork::net_ip_tcp_checksum(unsigned char *tcp_header, int chksumoffset,
	int tcp_len, unsigned char *srcaddr, unsigned char *dstaddr,
	int udpflag)
{
	int i, pad = 0;
	unsigned char pseudoh[12];
	uint32_t sum = 0;

	memcpy(pseudoh + 0, srcaddr, 4);
	memcpy(pseudoh + 4, dstaddr, 4);
	pseudoh[8] = 0x00;
	pseudoh[9] = udpflag? 17 : 6;
	pseudoh[10] = tcp_len >> 8;
	pseudoh[11] = tcp_len & 255;

	for (i=0; i<12; i+=2) {
		uint16_t w = (pseudoh[i] << 8) + pseudoh[i+1];
		sum += w;
		while (sum > 65535) {
			int to_add = sum >> 16;
			sum = (sum & 0xffff) + to_add;
		}
	}

	if (tcp_len & 1) {
		tcp_len ++;
		pad = 1;
	}

	for (i=0; i<tcp_len; i+=2)
		if (i != chksumoffset) {
			uint16_t w;
			if (!pad || i < tcp_len-2)
				w = (tcp_header[i] << 8) + tcp_header[i+1];
			else
				w = (tcp_header[i] << 8) + 0x00;
			sum += w;
			while (sum > 65535) {
				int to_add = sum >> 16;
				sum = (sum & 0xffff) + to_add;
			}
		}

	sum ^= 0xffff;
	tcp_header[chksumoffset + 0] = (u8) (sum >> 8);
	tcp_header[chksumoffset + 1] = (u8) (sum & 0xff);
}
