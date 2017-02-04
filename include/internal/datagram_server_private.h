#ifndef DATAGRAM_SERVER_PRIVATE_H
#define DATAGRAM_SERVER_PRIVATE_H

#include <iostream>

namespace Overpass
{
	namespace internal
	{
		/*!
		 * \brief The DatagramServerPrivate class is a server for datagram
		 *        sockets.
		 */
		template <typename T>
		class DatagramServerPrivate :
		      public std::enable_shared_from_this<DatagramServerPrivate<T>>
		{
			public:
				typedef std::function<void (
				      const typename T::endpoint&, const SharedBuffer&)> ReadCallback;

				/*!
				 * \brief DatagramServerPrivate constructor.
				 *
				 * \param[in,out] ioService
				 * IO service used for running the server.
				 *
				 * \param[in] socket
				 * Datagram socket-like object.
				 *
				 * \param[in] callback
				 * Function to be called when new data has been received from the
				 * socket.
				 *
				 * \param[in] bufferSize
				 * How large of a buffer size to support (this is the packet size).
				 */
				DatagramServerPrivate(const SharedIoService &ioService,
				                      std::unique_ptr<T> socket,
				                      ReadCallback callback,
				                      std::size_t bufferSize) :
				   m_ioService(ioService),
				   m_callback(callback),
				   m_socket(std::move(socket)),
				   m_bufferSize(bufferSize)
				{
				}

				/*!
				 * \brief Start reading from the socket.
				 *
				 * This function will return immediately. It will do nothing until
				 * the IO service is up and running (i.e. it queues up work to be
				 * done).
				 */
				void beginReading()
				{
					SharedBuffer buffer(new Overpass::Buffer(m_bufferSize));

					// Allocate a new endpoint to hold the sender's information.
					std::shared_ptr<typename T::endpoint> endpoint(new typename T::endpoint);

					m_socket->async_receive_from(
					         boost::asio::buffer(*buffer), *endpoint,
					         std::bind(&DatagramServerPrivate::handleRead,
					                   this->shared_from_this(), endpoint, buffer,
					                   std::placeholders::_1, std::placeholders::_2));
				}

				/*!
				 * \brief Send data to destination over socket.
				 *
				 * \param[in] destination
				 * Where to send the data.
				 *
				 * \param[in] buffer
				 * The data to send.
				 */
				void sendTo(const typename T::endpoint &destination,
				            const SharedBuffer &buffer)
				{
					m_socket->send_to(boost::asio::buffer(*buffer), destination);
				}

			private:
				/*!
				 * \brief Handle a completed read from the socket.
				 *
				 * \param[in] sender
				 * Who sent the data we just received.
				 *
				 * \param[in] buffer
				 * Buffer that was read (note that it's not necessarily full, check
				 * bytesRead).
				 *
				 * \param[in] error
				 * Error that occurred during reading (if any).
				 *
				 * \param[in] bytesRead
				 * The number of bytes read during the operation (if any).
				 */
				void handleRead(
				      std::shared_ptr<typename T::endpoint> sender,
				      SharedBuffer buffer,
				      const boost::system::error_code &error,
				      std::size_t bytesRead)
				{
					if (error)
					{
						std::cerr << "Error reading: " << error << std::endl;
						return;
					}

					if (bytesRead == 0)
					{
						std::cerr << "Received zero bytes?" << std::endl;
						return;
					}

					// We got something: dispatch callback with buffer.
					m_ioService->post(std::bind(m_callback, *sender, buffer));

					// Read some more.
					beginReading();
				}

			private:
				SharedIoService m_ioService;
				ReadCallback m_callback;
				std::unique_ptr<T> m_socket;
				std::size_t m_bufferSize;
		};
	}
}

#endif // DATAGRAM_SERVER_PRIVATE_H
