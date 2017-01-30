#ifndef STREAM_SERVER_H
#define STREAM_SERVER_H

#include <memory>
#include <iostream>

#include <boost/system/error_code.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/io_service.hpp>

#include "types.h"

namespace Overpass
{
	typedef std::function<void (const SharedBuffer&)> ReadCallback;

	template <typename T>
	class StreamServer;

	template <typename T>
	std::shared_ptr<StreamServer<T>> makeStreamServer(
	      const SharedIoService &ioService,
	      ReadCallback callback,
	      std::unique_ptr<T> socket,
	      std::size_t bufferSize = 1500);

	template <typename T>
	using SharedStreamServer = std::shared_ptr<StreamServer<T>>;

	/*!
	 * \brief The StreamServer class acts like a server for stream descriptors.
	 */
	template <typename T>
	class StreamServer :
	      public std::enable_shared_from_this<StreamServer<T>>
	{
		public:
			/*!
			 * \brief StreamServer constructor
			 *
			 * \param[in,out] ioService
			 * IO service used for running the server.
			 *
			 * \param[in] callback
			 * Function to be called when new data has been read from the
			 * descriptor.
			 *
			 * \param[in] socket
			 * Stream descriptor-like object.
			 *
			 * \param[in] bufferSize
			 * How large of a buffer size to support (this is the package size).
			 */
			StreamServer(const SharedIoService &ioService,
			             ReadCallback callback,
			             std::unique_ptr<T> socket,
			             std::size_t bufferSize = 1500):
			   m_ioService(ioService),
			   m_callback(callback),
			   m_bufferSize(bufferSize),
			   m_socket(std::move(socket))
			{
			}

			/*!
			 * \brief Write data to descriptor.
			 *
			 * \param[in] buffer
			 * Buffer containing data to be written.
			 *
			 * This function will return immediately. It will do nothing until the
			 * IO service is up and running (i.e. it queues up work to be done).
			 */
			void write(const Overpass::SharedBuffer &buffer) const
			{
				boost::asio::async_write(m_socket,
				                         boost::asio::buffer(*buffer),
				                         std::bind(&StreamServer::handleWrite,
				                                   this->shared_from_this(),
				                                   std::placeholders::_1,
				                                   std::placeholders::_2));
			}

			friend std::shared_ptr<StreamServer> makeStreamServer<T>(
			      const SharedIoService &ioService,
			      ReadCallback callback,
			      std::unique_ptr<T> socket,
			      std::size_t bufferSize);

		private:

			/*!
			 * \brief Start reading from the descriptor.
			 *
			 * This function will return immediately. It will do nothing until the
			 * IO service is up and running (i.e. it queues up work to be done).
			 */
			void beginReading() const
			{
				Overpass::SharedBuffer buffer(new Overpass::Buffer(m_bufferSize));

				m_socket->async_read_some(boost::asio::buffer(*buffer),
				                          std::bind(&StreamServer::handleRead,
				                                    this->shared_from_this(),
				                                    buffer,
				                                    std::placeholders::_1,
				                                    std::placeholders::_2));
			}

			/*!
			 * \brief Handle a completed read from the descriptor.
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
			void handleRead(Overpass::SharedBuffer buffer,
			                const boost::system::error_code &error,
			                std::size_t bytesRead) const
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
				m_ioService->post(std::bind(m_callback, buffer));

				// Read some more.
				beginReading();
			}

			/*!
			 * \brief Handle a completed write to the descriptor.
			 *
			 * \param[in] error
			 * Error that occurred during writing (if any).
			 *
			 * \param[in] bytesWritten
			 * The number of bytes written during the operation (if any).
			 */
			void handleWrite(const boost::system::error_code &error,
			                 std::size_t bytesWritten) const
			{
				if (error)
				{
					std::cerr << "Error writing: " << error << std::endl;
				}

				if (bytesWritten == 0)
				{
					std::cerr << "Wrote zero bytes?" << std::endl;
				}
			}

		private:
			SharedIoService m_ioService;
			ReadCallback m_callback;
			std::size_t m_bufferSize;
			std::unique_ptr<T> m_socket;
	};

	/*!
	 * \brief Create a new StreamServer and start it.
	 *
	 * \param[in,out] ioService
	 * IO service used for running the server.
	 *
	 * \param[in] callback
	 * Function to be called when new data has been read from the
	 * descriptor.
	 *
	 * \param[in] socket
	 * Stream descriptor-like object.
	 *
	 * \param[in] bufferSize
	 * How large of a buffer size to support (this is the package size).
	 */
	template <typename T>
	std::shared_ptr<StreamServer<T>> makeStreamServer(
	      const SharedIoService &ioService,
	      ReadCallback callback,
	      std::unique_ptr<T> socket,
	      std::size_t bufferSize)
	{
		auto communicator = std::make_shared<StreamServer<T> >(
		                       ioService, callback, std::move(socket),
		                       bufferSize);

		// Ideally the constructor would do this, but it can't as shared_from_this
		// can only be used once at least one shared pointer is pointing to the
		// object in question (in this case, the communicator).
		communicator->beginReading();
		return communicator;
	}
}

#endif // STREAM_SERVER_H
