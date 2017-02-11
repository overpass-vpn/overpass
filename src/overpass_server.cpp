#include "internal/overpass_server_private.h"
#include "overpass_server.h"

using namespace Overpass;

OverpassServer::OverpassServer(
      const SharedIoService &ioService,
      const std::string &overpassInterfacePattern,
      const std::string &overpassIpAddress, const std::string &overpassNetmask,
      const std::string &bindIpAddress, std::uint16_t bindPort) :
   m_data(new internal::OverpassServerPrivate(
             ioService, overpassInterfacePattern, overpassIpAddress,
             overpassNetmask, bindIpAddress, bindPort))
{
	m_data->start(); // Start server
}

void OverpassServer::addKnownClient(
      const boost::asio::ip::address &overpassAddress,
      const boost::asio::ip::address &externalAddress)
{
	m_data->addKnownClient(overpassAddress, externalAddress);
}
