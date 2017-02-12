#ifndef OVERPASS_SERVER_H
#define OVERPASS_SERVER_H

#include "types.h"

namespace boost
{
	namespace asio
	{
		namespace ip
		{
			class address;
		}
	}
}

namespace Overpass
{
	namespace internal
	{
		class OverpassServerPrivate;
	}

	/*!
	 * \brief The OverpassServer is for orchestrating the Overpass client
	 *        components.
	 *
	 * These components include two servers (one UDP server, one listening on the
	 * virtual interface) and a router to route traffic between them.
	 */
	class OverpassServer
	{
		public:
			/*!
			 * \brief OverpassServer constructor.
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
			OverpassServer(const SharedIoService &ioService,
			               const std::string &overpassInterfacePattern,
			               const std::string &overpassIpAddress,
			               const std::string &overpassNetmask,
			               const std::string &bindIpAddress,
			               std::uint16_t bindPort);

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
			 */
			void addKnownClient(
			      const boost::asio::ip::address &overpassAddress,
			      const boost::asio::ip::address &externalAddress);

		private:
			// Using a shared_ptr instead of unique_ptr because of
			// enable_shared_from_this.
			std::shared_ptr<internal::OverpassServerPrivate> m_data;
	};
}

#endif // OVERPASS_SERVER_H
