#ifndef UDP_PACKET_H
#define UDP_PACKET_H

#include <cstdint>
#include <vector>

/*!
 * \brief The UdpPacket class represents... well... a UDP packet.
 *
 * A UDP packet's bytes are constructed as follows:
 * Bytes 0-1: Source port
 * Bytes 2-3: Destination port
 * Bytes 4-5: Packet length
 * Bytes 6-7: Checksum
 * Bytes 8 on: Payload
 *
 * This class allows for the extraction of those values from a set of bytes, or
 * for turning those values into bytes ready to be sent over the wire.
 */
class UdpPacket
{
	public:
		/*!
		 * \brief Construct a UdpPacket given its components.
		 *
		 * \param[in] sourcePort
		 * The port from which this packet is being sent.
		 *
		 * \param[in] destinationPort
		 * The port to which this packet is being sent.
		 *
		 * \param[in] payload
		 * The payload contained within this packet.
		 */
		UdpPacket(uint16_t sourcePort, uint16_t destinationPort,
		          const std::vector<uint8_t> &payload);

		/*!
		 * \brief Construct a UdpPacket from a buffer of bytes.
		 *
		 * \param[in] buffer
		 * Bytes from which the packet components will be extracted. These bytes
		 * should be in network-byte-order.
		 */
		UdpPacket(const std::vector<uint8_t> &buffer);

		/*!
		 * \brief Convert this packet into bytes.
		 *
		 * \param[out] buffer
		 * The buffer which will be filled with bytes repesenting this packet.
		 * Note that the bytes will be in network-byte-order.
		 */
		void toBuffer(std::vector<uint8_t> &buffer) const;

		/*!
		 * \brief Convert this packet into a human-readable string.
		 *
		 * \param[out] packetString
		 * The string into which the packet will be written.
		 */
		void toString(std::string &packetString) const;

		/*!
		 * \brief Source port of the packet.
		 *
		 * \return The source port of the packet.
		 */
		uint16_t sourcePort() const;

		/*!
		 * \brief Destination port of the packet.
		 *
		 * \return The destination port of the packet.
		 */
		uint16_t destinationPort() const;

		/*!
		 * \brief Length of the packet, including the header.
		 *
		 * \return The length of the packet.
		 */
		uint16_t packetLength() const;

		/*!
		 * \brief Checksum of the packet.
		 *
		 * \return The checksum of the packet.
		 */
		uint16_t checksum() const;

		/*!
		 * \brief Payload of the packet.
		 *
		 * \return The payload of the packet.
		 */
		const std::vector<uint8_t>& payload() const;

	private:
		uint16_t m_sourcePort; // Bits 0-15
		uint16_t m_destinationPort; // Bits 16-31
		uint16_t m_packetLength; // Bits 32-47
		uint16_t m_checksum; // Bits 48-63
		std::vector<uint8_t> m_payload; // Bits 64 on
};

#endif // UDP_PACKET_H
