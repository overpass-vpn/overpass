#ifndef ROUTER_H
#define ROUTER_H

#include <map>

#include <boost/asio/ip/udp.hpp>

#include "types.h"

namespace Tins
{
	class IP;
}

namespace Overpass
{
	class RoutingException : public Exception
	{
		public:
			RoutingException(const std::string &what);
	};

	class UnknownClientException : public RoutingException
	{
		public:
			UnknownClientException(const boost::asio::ip::address &address);
	};

	/*!
	 * \brief The Router class shuffles packets between the external and virtual
	 *        interfaces.
	 */
	class Router
	{
		public:
			typedef std::function<void (
			      const boost::asio::ip::udp::endpoint &,
			      const Overpass::SharedBuffer &)> ExternalSender;
			typedef std::function<void (
			      const Overpass::SharedBuffer &)> VirtualSender;

			/*!
			 * \brief Router constructor.
			 *
			 * \param[in] externalSender
			 * Function to call in order to send a packet over the external
			 * interface.
			 *
			 * \param[in] virtualSender
			 * Function to call in order to send a packet over the virtual
			 * interface.
			 *
			 * \param[in] overpassPort
			 * Port on which Overpass clients should be listening.
			 */
			Router(ExternalSender externalSender, VirtualSender virtualSender,
			       std::uint16_t overpassPort);

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

			/*!
			 * \brief Route a packet from the virtual interface to a known client
			 *        over the external interface.
			 *
			 * \param[in] packet
			 * The packet to be routed.
			 */
			void handlePacketFromVirtual(Tins::IP &packet);

			/*!
			 * \brief Route a packet from the external interface to the virtual
			 *        interface.
			 *
			 * \param[in] packet
			 * The packet to be routed.
			 */
			void handlePacketFromExternal(Tins::IP &packet);

		private:
			ExternalSender m_externalSender;
			VirtualSender m_virtualSender;

			typedef std::map<boost::asio::ip::address, boost::asio::ip::address>
			ClientMap;
			ClientMap m_knownClients;

			std::uint16_t m_overpassPort;
	};
}

#endif // ROUTER_H
