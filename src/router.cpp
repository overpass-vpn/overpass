#include <iostream>

#include <boost/asio/detail/socket_ops.hpp>

#include <tins/ip.h>
#include <tins/udp.h>

#include "router.h"

using namespace Overpass;

Router::Router(ExternalSender externalSender, VirtualSender virtualSender) :
   m_externalSender(externalSender),
   m_virtualSender(virtualSender)
{
}

void Router::addKnownClient(const boost::asio::ip::address &overpassAddress,
                            const boost::asio::ip::address &externalAddress)
{
	m_knownClients[overpassAddress] = externalAddress;
}

void Router::handlePacketFromVirtual(Tins::IP &packet)
{
	// Extract the destination of this packet
	boost::asio::ip::address_v4 destination(
	         boost::asio::detail::socket_ops::network_to_host_long(
	            packet.dst_addr()));

	// Coming from the virtual interface, the destination will be an IP address
	// on the Overpass network. We need to look it up in our routing table to
	// determine where this packet actually needs to go.
	auto clientAddress = m_knownClients.at(destination);

	auto buffer = std::make_shared<Overpass::Buffer>(
	                 std::move(packet.serialize()));
	boost::asio::ip::udp::endpoint endpoint(clientAddress, 1234);
	m_externalSender(endpoint, buffer);
}

void Router::handlePacketFromExternal(Tins::IP &packet)
{
	// This packet is destined for something listening on our virtual interface.
	// Send it there.
	auto buffer = std::make_shared<Overpass::Buffer>(
	                 std::move(packet.serialize()));
	m_virtualSender(buffer);
}
