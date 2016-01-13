#include <stdexcept>
#include <sstream>

#include "udp_packet.h"

namespace
{
	const int UDP_HEADER_SIZE = 8;
}

UdpPacket::UdpPacket(uint16_t sourcePort, uint16_t destinationPort,
                     const std::vector<uint8_t> &payload) :
    m_sourcePort(sourcePort),
    m_destinationPort(destinationPort),
    m_checksum(0),
    m_payload(payload)
{
	m_packetLength = UDP_HEADER_SIZE + m_payload.size();
}

UdpPacket::UdpPacket(const std::vector<uint8_t> &buffer)
{
	auto bufferSize = buffer.size();
	if (bufferSize < UDP_HEADER_SIZE)
	{
		std::stringstream stream;
		stream << "UDP header is " << UDP_HEADER_SIZE << " bytes, but buffer "
		       << "is only " << bufferSize << " bytes";
		throw std::length_error(stream.str());
	}

	/* Parse the buffer into a UDP packet, which is formatted like this:
	 * Bits  0-15: Source port
	 * Bits 15-31: Destination port
	 * Bits 32-47: Packet length
	 * Bits 48-63: Checksum
	 * Bits 64 on: Payload
	 */

	// Parse source port, bytes 0-1.
	m_sourcePort = (buffer[0] << 8) | buffer[1];

	// Parse destination port, bytes 2-3.
	m_destinationPort = (buffer[2] << 8) | buffer[3];

	// Parse packet length, bytes 4-5.
	m_packetLength = (buffer[4] << 8) | buffer[5];
	if (bufferSize != m_packetLength)
	{
		std::stringstream stream;
		stream << "Buffer is " << bufferSize << " bytes, expected "
		       << m_packetLength;
		throw std::length_error(stream.str());
	}

	// Parse checksum, bytes 6-7.
	m_checksum = (buffer[6] << 8) | buffer[7];

	// Parse payload, bytes 8 and on.
	m_payload.assign(buffer.cbegin()+8, buffer.cend());
}

void UdpPacket::toBuffer(std::vector<uint8_t> &buffer) const
{
	buffer = {
	    static_cast<uint8_t>(sourcePort() >> 8),
	    static_cast<uint8_t>(sourcePort() & 0x0f),

	    static_cast<uint8_t>(destinationPort() >> 8),
	    static_cast<uint8_t>(destinationPort() & 0x0f),

	    static_cast<uint8_t>(packetLength() >> 8),
	    static_cast<uint8_t>(packetLength() & 0x0f),

	    static_cast<uint8_t>(checksum() >> 8),
	    static_cast<uint8_t>(checksum() & 0x0f),
	};

	buffer.insert(buffer.end(), m_payload.begin(), m_payload.end());
}

void UdpPacket::toString(std::string &packetString) const
{
	std::stringstream stream;
	stream << "[UDP PACKET]" << std::endl;
	stream << "Source port:\t" << sourcePort() << std::endl;
	stream << "Destination port:\t" << destinationPort() << std::endl;
	stream << "Packet length:\t" << packetLength() << std::endl;
	stream << "Checksum:\t" << checksum() << std::endl;

	packetString = stream.str();
}

uint16_t UdpPacket::sourcePort() const
{
	return m_sourcePort;
}

uint16_t UdpPacket::destinationPort() const
{
	return m_destinationPort;
}

uint16_t UdpPacket::packetLength() const
{
	return m_packetLength;
}

uint16_t UdpPacket::checksum() const
{
	return m_checksum;
}

const std::vector<uint8_t>& UdpPacket::payload() const
{
	return m_payload;
}
