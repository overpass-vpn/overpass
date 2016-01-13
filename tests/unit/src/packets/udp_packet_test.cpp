#include <gtest/gtest.h>

#include "udp_packet.h"

// Verify that parsing a UDP packet from a buffer works as expected.
TEST(UdpPacket, Parse)
{
	uint16_t sourcePort = 1; // Bits 0-15
	uint16_t destinationPort = 2; // Bits 16-31
	uint16_t packetLength = 8; // Bits 32-47
	uint16_t checksum = 3; // Bits 48-63

	std::vector<uint8_t> buffer = {
	    static_cast<uint8_t>(sourcePort >> 8),
	    static_cast<uint8_t>(sourcePort & 0x0f),

	    static_cast<uint8_t>(destinationPort >> 8),
	    static_cast<uint8_t>(destinationPort & 0x0f),

	    static_cast<uint8_t>(packetLength >> 8),
	    static_cast<uint8_t>(packetLength & 0x0f),

	    static_cast<uint8_t>(checksum >> 8),
	    static_cast<uint8_t>(checksum & 0x0f),
	};

	UdpPacket packet(buffer);

	EXPECT_EQ(sourcePort, packet.sourcePort());
	EXPECT_EQ(destinationPort, packet.destinationPort());
	EXPECT_EQ(packetLength, packet.packetLength());
	EXPECT_EQ(checksum, packet.checksum());
}

// Verify that an exception is thrown if the buffer is too short to hold the
// UDP header.
TEST(UdpPacket, ParseTooShortForHeader)
{
	std::vector<uint8_t> buffer = {1}; // Size of 1, should be 8 for the header

	ASSERT_THROW(UdpPacket packet(buffer), std::length_error);
}

// Verify that an exception is thrown if the packet size is shorter than the
// buffer.
TEST(UdpPacket, ParseStatedLengthTooShort)
{
	uint16_t sourcePort = 1; // Bits 0-15
	uint16_t destinationPort = 2; // Bits 16-31
	uint16_t packetLength = 7; // Bits 32-47
	uint16_t checksum = 3; // Bits 48-63

	std::vector<uint8_t> buffer = {
	    static_cast<uint8_t>(sourcePort >> 8),
	    static_cast<uint8_t>(sourcePort & 0x0f),

	    static_cast<uint8_t>(destinationPort >> 8),
	    static_cast<uint8_t>(destinationPort & 0x0f),

	    static_cast<uint8_t>(packetLength >> 8),
	    static_cast<uint8_t>(packetLength & 0x0f),

	    static_cast<uint8_t>(checksum >> 8),
	    static_cast<uint8_t>(checksum & 0x0f),
	};

	EXPECT_THROW(UdpPacket packet(buffer), std::length_error);
}

// Verify that an exception is thrown if the packet size is longer than the
// buffer.
TEST(UdpPacket, ParseStatedLengthTooLong)
{
	uint16_t sourcePort = 1; // Bits 0-15
	uint16_t destinationPort = 2; // Bits 16-31
	uint16_t packetLength = 9; // Bits 32-47
	uint16_t checksum = 3; // Bits 48-63

	std::vector<uint8_t> buffer = {
	    static_cast<uint8_t>(sourcePort >> 8),
	    static_cast<uint8_t>(sourcePort & 0x0f),

	    static_cast<uint8_t>(destinationPort >> 8),
	    static_cast<uint8_t>(destinationPort & 0x0f),

	    static_cast<uint8_t>(packetLength >> 8),
	    static_cast<uint8_t>(packetLength & 0x0f),

	    static_cast<uint8_t>(checksum >> 8),
	    static_cast<uint8_t>(checksum & 0x0f),
	};

	EXPECT_THROW(UdpPacket packet(buffer), std::length_error);
}

// Test that toBuffer() works as expected by creating a packet from the buffer
// again.
TEST(UdpPacket, ToBufferAndBack)
{
	uint16_t sourcePort = 1;
	uint16_t destinationPort = 2;
	std::vector<uint8_t> payload{3, 4, 5};

	UdpPacket initialPacket(sourcePort, destinationPort, payload);

	std::vector<uint8_t> buffer;
	initialPacket.toBuffer(buffer);
	UdpPacket packet(buffer);

	EXPECT_EQ(sourcePort, packet.sourcePort());
	EXPECT_EQ(destinationPort, packet.destinationPort());
	EXPECT_EQ(11, packet.packetLength());
	EXPECT_EQ(0, packet.checksum()); // 0 for now
	EXPECT_EQ(payload, packet.payload());
}
