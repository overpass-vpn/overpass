#include <unistd.h>

#include <tins/ip.h>

#include "virtual_interface.h"
#include "datagram_server.h"
#include "stream_server.h"
#include "router.h"
#include "internal/overpass_server_private.h"

using namespace Overpass::internal;

OverpassServerPrivate::OverpassServerPrivate(
      const SharedIoService &ioService,
      const std::string &overpassInterfacePattern,
      const std::string &overpassIpAddress, const std::string &overpassNetmask,
      const std::string &bindIpAddress, std::uint16_t bindPort) :
   m_ioService(ioService),
   m_interfaceName(overpassInterfacePattern),
   m_overpassIpAddress(overpassIpAddress),
   m_overpassNetmask(overpassNetmask),
   m_bindIpAddress(bindIpAddress),
   m_bindPort(bindPort)
{
	Overpass::createVirtualInterface(m_interfaceName,
	                                 m_virtualInterfaceDescriptor);
	Overpass::assignDeviceAddress(m_interfaceName, overpassIpAddress,
	                              overpassNetmask);
}

OverpassServerPrivate::~OverpassServerPrivate()
{
	if (m_virtualInterfaceDescriptor)
	{
		close(m_virtualInterfaceDescriptor);
	}
}

void OverpassServerPrivate::start()
{
	std::unique_ptr<boost::asio::ip::udp::socket> socket(
	         new boost::asio::ip::udp::socket(
	            *m_ioService, boost::asio::ip::udp::endpoint(
	               boost::asio::ip::address::from_string(
	                  m_bindIpAddress), m_bindPort)));
	m_externalServer.reset(new UdpServer(
	                          m_ioService, std::move(socket),
	                          std::bind(
	                             &OverpassServerPrivate::handleReadFromExternal,
	                             shared_from_this(),
	                             std::placeholders::_1, std::placeholders::_2)));

	std::unique_ptr<boost::asio::posix::stream_descriptor> descriptor(
	         new boost::asio::posix::stream_descriptor(*m_ioService));
	descriptor->assign(m_virtualInterfaceDescriptor);

	m_virtualServer = makeStreamServer(
	                         m_ioService, std::bind(
	                            &OverpassServerPrivate::handleReadFromVirtual,
	                            shared_from_this(),
	                            std::placeholders::_1),
	                         std::move(descriptor));

	m_router.reset(new Overpass::Router(
	                  std::bind(&UdpServer::sendTo, m_externalServer,
	                            std::placeholders::_1, std::placeholders::_2),
	                  std::bind(&PosixStreamServer::write, m_virtualServer,
	                            std::placeholders::_1),
	                  m_bindPort));
}

void OverpassServerPrivate::addKnownClient(
      const boost::asio::ip::address &overpassAddress,
      const boost::asio::ip::address &externalAddress)
{
	if (!m_router)
	{
		throw Exception("server isn't started, cannot add client.");
	}

	m_router->addKnownClient(overpassAddress, externalAddress);
}

void OverpassServerPrivate::handleReadFromVirtual(const SharedBuffer &buffer)
{
	// Traffic coming in from the virtual interface. This means some software
	// running on the host is reaching out to an Overpass client.
	Tins::IP packet(buffer->data(), buffer->size());
	try
	{
		m_router->handlePacketFromVirtual(packet);
	}
	catch (const UnknownClientException &exception)
	{
		std::cerr << exception.what() << std::endl;
	}
}

void OverpassServerPrivate::handleReadFromExternal(
      const boost::asio::ip::udp::endpoint &/*endpoint*/,
      const SharedBuffer &buffer)
{
	// Traffic coming in from the external interface contains a nested IP packet
	// destined for some software running on our host, bound to the virtual
	// interface.
	Tins::IP packet(buffer->data(), buffer->size());
	m_router->handlePacketFromExternal(packet);
}
