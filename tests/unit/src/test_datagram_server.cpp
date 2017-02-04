#include <thread>
#include <condition_variable>
#include <chrono>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <boost/asio/buffer.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>

#include "datagram_server.h"

class FakeDatagramSocket
{
	public:
		typedef std::string endpoint;

		FakeDatagramSocket(const Overpass::SharedIoService &ioService) :
		   m_ioService(ioService)
		{
		}

	protected:
		Overpass::SharedIoService m_ioService;
};

class FakeDatagramSocketReceiveSuccess : public FakeDatagramSocket
{
	public:
		FakeDatagramSocketReceiveSuccess(
		      const Overpass::SharedIoService &ioService) :
		   FakeDatagramSocket(ioService)
		{
		}

		void async_receive_from(
		      boost::asio::mutable_buffers_1 buffers,
		      std::string &endpoint,
		      std::function<void (
		         boost::system::error_code, std::size_t)> callback)
		{
			m_ioService->post([buffers, callback, &endpoint](){
				// Make sure that there's room for the single byte we're about to
				// write into the buffer.
				ASSERT_LT(1, boost::asio::buffer_size(buffers));
				Overpass::Buffer buffer{0xff};
				memcpy(boost::asio::buffer_cast<void *>(buffers),
				       buffer.data(),
				       1);
				endpoint = "test-sender";
				callback(boost::system::error_code(), 1);
			});
		}
};

class FakeDatagramSocketReceiveError : public FakeDatagramSocket
{
	public:
		FakeDatagramSocketReceiveError(
		      const Overpass::SharedIoService &ioService) :
		   FakeDatagramSocket(ioService)
		{
		}

		void async_receive_from(
		      boost::asio::mutable_buffers_1 buffers,
		      std::string &endpoint,
		      std::function<void (
		         boost::system::error_code, std::size_t)> callback)
		{
			m_ioService->post([buffers, callback, &endpoint](){
				// Make sure that there's room for the single byte we're about to
				// write into the buffer.
				ASSERT_LT(1, boost::asio::buffer_size(buffers));
				Overpass::Buffer buffer{0xff};
				memcpy(boost::asio::buffer_cast<void *>(buffers),
				       buffer.data(),
				       1);
				endpoint = "test-sender";
				using boost::system::errc::make_error_code;
				callback(make_error_code(
				            boost::system::errc::operation_canceled), 0);
			});
		}
};

class FakeDatagramSocketReceiveNothing : public FakeDatagramSocket
{
	public:
		FakeDatagramSocketReceiveNothing(
		      const Overpass::SharedIoService &ioService) :
		   FakeDatagramSocket(ioService)
		{
		}

		void async_receive_from(
		      boost::asio::mutable_buffers_1 buffers,
		      std::string &endpoint,
		      std::function<void (
		         boost::system::error_code, std::size_t)> callback)
		{
			m_ioService->post([buffers, callback, &endpoint](){
				// Make sure that there's room for the single byte we're about to
				// write into the buffer.
				ASSERT_LT(1, boost::asio::buffer_size(buffers));
				Overpass::Buffer buffer{0xff};
				memcpy(boost::asio::buffer_cast<void *>(buffers),
				       buffer.data(),
				       1);
				endpoint = "test-sender";
				callback(boost::system::error_code(), 0);

			});
		}
};

TEST(DatagramServer, ReadCallback)
{
	Overpass::SharedIoService ioService(new boost::asio::io_service);
	using boost::system::error_code;

	std::mutex mutex;
	std::condition_variable condition;

	auto callback = [&](const std::string &endpoint, const Overpass::SharedBuffer &buffer)
	{
		EXPECT_EQ("test-sender", endpoint);
		EXPECT_LT(1, buffer->size());
		EXPECT_EQ(0xff, buffer->at(0));
		ioService->stop();
		std::unique_lock<std::mutex> lock(mutex);
		condition.notify_one();
	};

	std::unique_ptr<FakeDatagramSocketReceiveSuccess> socket(
	         new FakeDatagramSocketReceiveSuccess(ioService));

	auto server = std::make_shared<Overpass::DatagramServer<FakeDatagramSocketReceiveSuccess>>(
	                 ioService, std::move(socket), callback);

	std::thread thread([&ioService](){ioService->run();});

	std::unique_lock<std::mutex> lock(mutex);
	auto status = condition.wait_for(lock, std::chrono::seconds(1));
	EXPECT_EQ(std::cv_status::no_timeout, status) << "Unexpectedly timed out";
	ioService->stop();
	thread.join();
}

TEST(DatagramServer, ReadError)
{
	Overpass::SharedIoService ioService(new boost::asio::io_service);
	using boost::system::error_code;

	std::mutex mutex;
	std::condition_variable condition;

	auto callback = [&](const std::string &/*endpoint*/,
	                const Overpass::SharedBuffer &/*buffer*/)
	{
		FAIL() << "Callback was unexpectedly called";
		ioService->stop();
		std::unique_lock<std::mutex> lock(mutex);
		condition.notify_one();
	};

	std::unique_ptr<FakeDatagramSocketReceiveError> socket(
	         new FakeDatagramSocketReceiveError(ioService));

	auto server = std::make_shared<Overpass::DatagramServer<FakeDatagramSocketReceiveError>>(
	                 ioService, std::move(socket), callback);

	std::thread thread([&ioService](){ioService->run();});

	std::unique_lock<std::mutex> lock(mutex);
	auto status = condition.wait_for(lock, std::chrono::seconds(1));
	EXPECT_EQ(std::cv_status::timeout, status) << "Unexpectedly DIDN'T timed out";
	ioService->stop();
	thread.join();
}

TEST(DatagramServer, ReadNothing)
{
	Overpass::SharedIoService ioService(new boost::asio::io_service);
	using boost::system::error_code;

	std::mutex mutex;
	std::condition_variable condition;

	auto callback = [&](const std::string &/*endpoint*/,
	                const Overpass::SharedBuffer &/*buffer*/)
	{
		FAIL() << "Callback was unexpectedly called";
		ioService->stop();
		std::unique_lock<std::mutex> lock(mutex);
		condition.notify_one();
	};

	std::unique_ptr<FakeDatagramSocketReceiveNothing> socket(
	         new FakeDatagramSocketReceiveNothing(ioService));

	auto server = std::make_shared<Overpass::DatagramServer<FakeDatagramSocketReceiveNothing>>(
	                 ioService, std::move(socket), callback);

	std::thread thread([&ioService](){ioService->run();});

	std::unique_lock<std::mutex> lock(mutex);
	auto status = condition.wait_for(lock, std::chrono::seconds(1));
	EXPECT_EQ(std::cv_status::timeout, status) << "Unexpectedly DIDN'T timed out";
	ioService->stop();
	thread.join();
}
