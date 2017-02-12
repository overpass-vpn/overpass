#include <iostream>

#include <boost/asio/detail/socket_ops.hpp>

#include <tins/ip.h>
#include <tins/udp.h>

#include "router.h"

using namespace Overpass;

RoutingException::RoutingException(const std::string &what) :
   Exception("unable to route packet: " + what)
{
}

UnknownClientException::UnknownClientException(
      const boost::asio::ip::address &address) :
   RoutingException("no client with address '" + address.to_string() + "'")
{
}

Router::Router(ExternalSender externalSender, VirtualSender virtualSender,
               std::uint16_t overpassPort) :
   m_externalSender(externalSender),
   m_virtualSender(virtualSender),
   m_overpassPort(overpassPort)
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
	boost::asio::ip::address clientAddress;
	try
	{
		clientAddress = m_knownClients.at(destination);
	}
	catch (const std::out_of_range&)
	{
		throw UnknownClientException(destination);
	}

	auto buffer = std::make_shared<Overpass::Buffer>(
	                 std::move(packet.serialize()));
	boost::asio::ip::udp::endpoint endpoint(clientAddress, m_overpassPort);
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
