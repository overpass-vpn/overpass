#include <thread>
#include <condition_variable>
#include <chrono>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <boost/asio/buffer.hpp>
#include <boost/system/error_code.hpp>

#include "stream_server.h"

class FakeStreamDescriptor
{
	public:
		FakeStreamDescriptor(const Overpass::SharedIoService &ioService) :
		   m_ioService(ioService)
		{
		}

		MOCK_METHOD2(async_write_some, void(
		                const boost::asio::const_buffers_1&,
		                std::function<void (boost::system::error_code,
		                                    std::size_t)>));

	protected:
		Overpass::SharedIoService m_ioService;
};

class FakeStreamDescriptorReadSuccess : public FakeStreamDescriptor
{
	public:
		FakeStreamDescriptorReadSuccess(const Overpass::SharedIoService &ioService) :
		   FakeStreamDescriptor(ioService)
		{
		}

		void async_read_some(
		      boost::asio::mutable_buffers_1 buffers,
		      std::function<void (
		         boost::system::error_code, std::size_t)> callback)
		{
			m_ioService->post([buffers, callback](){
				// Make sure that there's room for the single byte we're about to
				// write into the buffer.
				ASSERT_LT(1, boost::asio::buffer_size(buffers));
				Overpass::Buffer buffer{0xff};
				memcpy(boost::asio::buffer_cast<void *>(buffers),
				       buffer.data(),
				       1);
				callback(boost::system::error_code(), 1);
			});
		}
};

class FakeStreamDescriptorReadError : public FakeStreamDescriptor
{
	public:
		FakeStreamDescriptorReadError(const Overpass::SharedIoService &ioService) :
		   FakeStreamDescriptor(ioService)
		{
		}

		void async_read_some(
		      boost::asio::mutable_buffers_1 /*buffers*/,
		      std::function<void (
		         boost::system::error_code, std::size_t)> callback)
		{
			m_ioService->post([callback](){
				using boost::system::errc::make_error_code;
				callback(make_error_code(
				            boost::system::errc::operation_canceled), 0);
			});
		}
};

class FakeStreamDescriptorReadNothing : public FakeStreamDescriptor
{
	public:
		FakeStreamDescriptorReadNothing(const Overpass::SharedIoService &ioService) :
		   FakeStreamDescriptor(ioService)
		{
		}

		void async_read_some(
		      boost::asio::mutable_buffers_1 /*buffers*/,
		      std::function<void (
		         boost::system::error_code, std::size_t)> callback)
		{
			m_ioService->post([callback](){
				callback(boost::system::error_code(), 0);
			});
		}
};

TEST(StreamServer, ReadCallback)
{
	Overpass::SharedIoService ioService(new boost::asio::io_service);
	using boost::system::error_code;

	std::mutex mutex;
	std::condition_variable condition;

	auto callback = [&](const Overpass::SharedBuffer &buffer)
	{
		EXPECT_LT(1, buffer->size());
		EXPECT_EQ(0xff, buffer->at(0));
		ioService->stop();
		std::unique_lock<std::mutex> lock(mutex);
		condition.notify_one();
	};

	std::unique_ptr<FakeStreamDescriptorReadSuccess> socket(
	         new FakeStreamDescriptorReadSuccess(ioService));

	auto streamServer = Overpass::makeStreamServer(ioService, callback, std::move(socket));

	std::thread thread([&ioService](){ioService->run();});

	std::unique_lock<std::mutex> lock(mutex);
	auto status = condition.wait_for(lock, std::chrono::seconds(1));
	EXPECT_EQ(std::cv_status::no_timeout, status) << "Unexpectedly timed out";
	ioService->stop();
	thread.join();
}

TEST(StreamServer, ReadError)
{
	Overpass::SharedIoService ioService(new boost::asio::io_service);
	using boost::system::error_code;

	std::mutex mutex;
	std::condition_variable condition;

	auto callback = [&](const Overpass::SharedBuffer &/*buffer*/)
	{
		FAIL() << "Callback was unexpectedly called";
		ioService->stop();
		std::unique_lock<std::mutex> lock(mutex);
		condition.notify_one();
	};

	std::unique_ptr<FakeStreamDescriptorReadError> socket(
	         new FakeStreamDescriptorReadError(ioService));

	auto streamServer = Overpass::makeStreamServer(ioService, callback, std::move(socket));

	std::thread thread([&ioService](){ioService->run();});

	std::unique_lock<std::mutex> lock(mutex);
	auto status = condition.wait_for(lock, std::chrono::seconds(1));
	EXPECT_EQ(std::cv_status::timeout, status) << "Unexpectedly DIDN'T timed out";
	ioService->stop();
	thread.join();
}

TEST(StreamServer, ReadNothing)
{
	Overpass::SharedIoService ioService(new boost::asio::io_service);
	using boost::system::error_code;

	std::mutex mutex;
	std::condition_variable condition;

	auto callback = [&](const Overpass::SharedBuffer &/*buffer*/)
	{
		FAIL() << "Callback was unexpectedly called";
		ioService->stop();
		std::unique_lock<std::mutex> lock(mutex);
		condition.notify_one();
	};

	std::unique_ptr<FakeStreamDescriptorReadNothing> socket(
	         new FakeStreamDescriptorReadNothing(ioService));

	auto streamServer = Overpass::makeStreamServer(ioService, callback, std::move(socket));

	std::thread thread([&ioService](){ioService->run();});

	std::unique_lock<std::mutex> lock(mutex);
	auto status = condition.wait_for(lock, std::chrono::seconds(1));
	EXPECT_EQ(std::cv_status::timeout, status) << "Unexpectedly DIDN'T timed out";
	ioService->stop();
	thread.join();
}
