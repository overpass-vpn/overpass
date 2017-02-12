#ifndef OVERPASS_SERVER_PRIVATE_H
#define OVERPASS_SERVER_PRIVATE_H

#include <boost/asio/ip/udp.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>

#include "types.h"

namespace Overpass
{
	template <typename T>
	class DatagramServer;

	template <typename T>
	class StreamServer;

	class Router;

	namespace internal
	{
		/*!
		 * \brief The OverpassServerPrivate class is the implementation of the
		 *        Overpass server.
		 *
		 * It's responsible for orchestrating the various servers and routers that
		 * make up the Overpass client.
		 */
		class OverpassServerPrivate :
		      public std::enable_shared_from_this<OverpassServerPrivate>
		{
			public:
				/*!
				 * \brief OverpassServerPrivate constructor.
				 *
				 * \param[in,out] ioService
				 * IO service used for running the server.
				 *
				 * \param[in] overpassInterfacePattern
				 * Pattern determining virtual interface name (e.g. "ovp%d").
				 *
				 * \param[in] overpassIpAddress
				 * The IP address to use on the Overpass network. This needs to be
				 * unique among clients.
				 *
				 * \param[in] overpassNetmask
				 * Netmask to use for the virtual interface.
				 *
				 * \param[in] bindIpAddress
				 * IP address on which to bind listening for Overpass traffic.
				 *
				 * \param[in] bindPort
				 * UDP port on which to bind listening for Overpass traffic.
				 */
				OverpassServerPrivate(const SharedIoService &ioService,
				                      const std::string &overpassInterfacePattern,
				                      const std::string &overpassIpAddress,
				                      const std::string &overpassNetmask,
				                      const std::string &bindIpAddress,
				                      std::uint16_t bindPort);

				/*!
				 * \brief OverpassServerPrivate destructor.
				 *
				 * Simply close the virtual interface file descriptor, if any.
				 */
				~OverpassServerPrivate();

				/*!
				 * \brief Actually bring all servers/routers up.
				 *
				 * This class is useless without running this method.
				 */
				void start();

				/*!
				 * \brief Add a known client, mapping Overpass address to external
				 *        address.
				 *
				 * \param[in] overpassAddress
				 * Client's IP address on the Overpass network.
				 *
				 * \param[in] externalAddress
				 * Client's external IP address, on which the Overpass client is
				 * (hopefully) listening.
				 *
				 * start() must be called before this function can be used.
				 *
				 * \exception Overpass::Exception
				 * If called before start().
				 */
				void addKnownClient(
				      const boost::asio::ip::address &overpassAddress,
				      const boost::asio::ip::address &externalAddress);

			private:
				/*!
				 * \brief Handle incoming data from the virtual interface.
				 *
				 * \param[in] buffer
				 * Incoming data.
				 */
				void handleReadFromVirtual(const SharedBuffer &buffer);

				/*!
				 * \brief Handle incoming data from the external interface.
				 *
				 * \param[in] endpoint
				 * Source endpoint for the data.
				 *
				 * \param[in] buffer
				 * Data sent by the endpoint.
				 */
				void handleReadFromExternal(
				      const boost::asio::ip::udp::endpoint &endpoint,
				      const SharedBuffer &buffer);

			private:
				SharedIoService m_ioService;
				std::string m_interfaceName;
				int m_virtualInterfaceDescriptor;
				std::string m_overpassIpAddress;
				std::string m_overpassNetmask;
				std::string m_bindIpAddress;
				std::uint16_t m_bindPort;

				std::unique_ptr<Router> m_router;

				typedef DatagramServer<boost::asio::ip::udp> UdpServer;
				std::shared_ptr<UdpServer> m_externalServer;

				typedef StreamServer<boost::asio::posix::stream_descriptor>
				PosixStreamServer;
				std::shared_ptr<PosixStreamServer> m_virtualServer;
		};
	}
}

#endif // OVERPASS_SERVER_PRIVATE_H
