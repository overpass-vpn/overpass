#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <tins/ip.h>
#include <tins/udp.h>
#include <tins/rawpdu.h>

#include "router.h"

// Test that a packet from the external interface gets sent to the virtual
// interface.
TEST(Router, FromExternal)
{
	uint16_t destinationPort = 1000;
	uint16_t sourcePort = 1001;

	auto externalSender = [&](const boost::asio::ip::udp::endpoint&,
	                      const Overpass::SharedBuffer&)
	{
		FAIL() << "Router unexpectedly sent data to the external interface";
	};

	bool virtualSenderCalled = false;
	auto virtualSender = [&](const Overpass::SharedBuffer &buffer)
	{
		virtualSenderCalled = true;

		// Reconstruct the IP packet from the buffer
		Tins::IP packet(buffer->data(), buffer->size());
		const Tins::UDP udpPacket = packet.rfind_pdu<Tins::UDP>();

		// Verify that this packet is the nested one.
		EXPECT_EQ(destinationPort, udpPacket.dport());
		EXPECT_EQ(sourcePort, udpPacket.sport());

		const Tins::RawPDU& raw = udpPacket.rfind_pdu<Tins::RawPDU>();
		const Tins::RawPDU::payload_type &payload = raw.payload();
		std::string payloadString(reinterpret_cast<const char*>(
		                             payload.data()), payload.size());
		EXPECT_EQ("test-packet", payloadString);
	};

	Overpass::Router router(externalSender, virtualSender, 1234);

	Tins::IP packet = Tins::IP("11.11.11.2") /
	                  Tins::UDP(destinationPort, sourcePort) /
	                  Tins::RawPDU("test-packet");

	router.handlePacketFromExternal(packet);
	EXPECT_EQ(true, virtualSenderCalled)
	      << "Expected virtual sender to be called";
}

// Test that a packet from the virtual interface gets sent to the correct client
// over the external interface.
TEST(Router, FromVirtual)
{
	uint16_t destinationPort = 1000;
	uint16_t sourcePort = 1001;

	auto overpassAddress = boost::asio::ip::address::from_string("11.11.11.2");
	auto externalAddress = boost::asio::ip::address::from_string("1.2.3.4");

	Tins::IP packet = Tins::IP(overpassAddress.to_string()) /
	                  Tins::UDP(destinationPort, sourcePort) /
	                  Tins::RawPDU("test-packet");

	bool externalSenderCalled = false;
	auto externalSender = [&](const boost::asio::ip::udp::endpoint &destination,
	                      const Overpass::SharedBuffer &buffer)
	{
		externalSenderCalled = true;

		// Verify the destination is the client's external IP address.
		EXPECT_EQ(externalAddress, destination.address());

		// Reconstruct the packet from the buffer, and verify it's the one we
		// sent.
		Tins::IP ipPacket(buffer->data(), buffer->size());
		const Tins::UDP &udpPacket = ipPacket.rfind_pdu<Tins::UDP>();

		EXPECT_EQ(destinationPort, udpPacket.dport());
		EXPECT_EQ(sourcePort, udpPacket.sport());

		const Tins::RawPDU& raw = udpPacket.rfind_pdu<Tins::RawPDU>();
		const Tins::RawPDU::payload_type &payload = raw.payload();
		std::string payloadString(reinterpret_cast<const char*>(
		                             payload.data()), payload.size());
		EXPECT_EQ("test-packet", payloadString);
	};

	auto virtualSender = [&](const Overpass::SharedBuffer&)
	{
		FAIL() << "Router unexpectedly sent data to the virtual interface";
	};

	Overpass::Router router(externalSender, virtualSender, 1234);
	router.addKnownClient(overpassAddress, externalAddress);

	router.handlePacketFromVirtual(packet);
	EXPECT_EQ(true, externalSenderCalled)
	      << "Expected external sender to be called";
}

// Test that an exception is raised if a packet from the virtual interface is
// trying to get to an unknown client.
TEST(Router, UnknownClient)
{
	auto overpassAddress = boost::asio::ip::address::from_string("11.11.11.2");
	uint16_t destinationPort = 1000;
	uint16_t sourcePort = 1001;

	Tins::IP packet = Tins::IP(overpassAddress.to_string()) /
	                  Tins::UDP(destinationPort, sourcePort) /
	                  Tins::RawPDU("test-packet");

	auto externalSender = [&](const boost::asio::ip::udp::endpoint&,
	                      const Overpass::SharedBuffer&)
	{
		FAIL() << "Router unexpectedly sent data to the external interface";
	};

	auto virtualSender = [&](const Overpass::SharedBuffer&)
	{
		FAIL() << "Router unexpectedly sent data to the virtual interface";
	};

	Overpass::Router router(externalSender, virtualSender, 1234);
	try
	{
		router.handlePacketFromVirtual(packet);
		FAIL() << "Expected router to throw exception";
	}
	catch (const Overpass::UnknownClientException &exception)
	{
		EXPECT_STREQ(exception.what(),
		             "Overpass error: unable to route packet: no client with "
		             "address '11.11.11.2'");
	}
	catch(...)
	{
		FAIL() << "Expected router to throw Overpass::UnknownClientException";
	}
}
