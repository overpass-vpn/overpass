#ifndef DATAGRAM_SERVER_H
#define DATAGRAM_SERVER_H

#include <boost/noncopyable.hpp>

#include "types.h"
#include "internal/datagram_server_private.h"

namespace Overpass
{
	/*!
	 * \brief The DatagramServer class is a server for datagram sockets.
	 */
	template <typename T>
	class DatagramServer: private boost::noncopyable
	{
		public:
			typedef typename internal::DatagramServerPrivate<T>::ReadCallback ReadCallback;

			/*!
			 * \brief DatagramServer constructor.
			 *
			 * \param[in,out] ioService
			 * IO service used for running the server.
			 *
			 * \param[in] socket
			 * Datagram socket-like object.
			 *
			 * \param[in] callback
			 * Function to be called when new data has been read from the socket.
			 *
			 * \param[in] bufferSize
			 * How large of a buffer size to support (this is the packet size).
			 */
			DatagramServer(const SharedIoService &ioService,
			               std::unique_ptr<T> socket, ReadCallback callback,
			               std::size_t bufferSize = 1500) :
			   m_data(new internal::DatagramServerPrivate<T>(
			             ioService, std::move(socket), callback, bufferSize))
			{
				m_data->beginReading();
			}

			/*!
			 * \brief Send data over the socket to a specific endpoint.
			 *
			 * \param[in] destination
			 * Where to send the data.
			 *
			 * \param[in] buffer
			 * The data to send.
			 */
			void sendTo(const typename T::endpoint &destination,
			            const SharedBuffer &buffer) const
			{
				m_data->sendTo(destination, buffer);
			}

		private:
			// Using a shared_ptr instead of unique_ptr because of
			// enable_shared_from_this.
			std::shared_ptr<internal::DatagramServerPrivate<T>> m_data;
	};
}

#endif // DATAGRAM_SERVER_H
