#ifdef __linux__ // Everything in this file only applies to Linux systems.

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <arpa/inet.h>
#include <stdio.h>

#include <cstring>
#include <string>

#include "types.h"
#include "virtual_interface.h"

using namespace Overpass;

namespace
{
	const char *CLONE_DEVICE = "/dev/net/tun";
}

VirtualInterfaceException::VirtualInterfaceException(const std::string &what) :
   Exception(what + ": " + strerror(errno))
{
}

void Overpass::createVirtualInterface(std::string &interfaceName,
                                      int &interfaceFileDescriptor)
{
	// First open the clone device RW
	interfaceFileDescriptor = open(CLONE_DEVICE, O_RDWR);
	if (interfaceFileDescriptor < 0)
	{
		throw Overpass::VirtualInterfaceException("failed to open clone device");
	}

	// Zero-out the request (so our strings are null-terminated)
	ifreq request;
	memset(&request, 0, sizeof(request));

	// Create a TUN device = layer-3 IP packets (not layer 2). Also, tell the
	// kernel not to include the extra packet info (flags and protocol).
	request.ifr_flags = IFF_TUN | IFF_NO_PI;

	// Tell it what name we'd like (or template to satisfy)
	interfaceName.copy(request.ifr_name, IFNAMSIZ);

	// Finally, clone the device and setup the new virtual interface
	if (ioctl(interfaceFileDescriptor, TUNSETIFF, &request) < 0)
	{
		throw Overpass::VirtualInterfaceException(
		         "failed to create virtual interface");
	}

	// Set the name that we actually received
	interfaceName = std::string(request.ifr_name);
}

void Overpass::assignDeviceAddress(const std::string &interfaceName,
                                   const std::string &ipAddress,
                                   const std::string &netmask)
{
	// Zero-out the request (so our strings are null-terminated)
	ifreq request;
	memset(&request, 0, sizeof(request));

	// Set the name of the interface we're about to modify
	interfaceName.copy(request.ifr_name, IFNAMSIZ);

	// Create an IPv4 datagram socket
	request.ifr_addr.sa_family = PF_INET;
	int sockfd = socket(PF_INET, SOCK_DGRAM, 0);

	sockaddr_in *socketAddress = reinterpret_cast<sockaddr_in*>(&request.ifr_addr);

	// Set IP address
	inet_pton(PF_INET, ipAddress.c_str(), &socketAddress->sin_addr);
	if (ioctl(sockfd, SIOCSIFADDR, &request) < 0)
	{
		throw Overpass::VirtualInterfaceException("unable to set IP address");
	}

	// Set netmask
	inet_pton(PF_INET, netmask.c_str(), &socketAddress->sin_addr);
	if (ioctl(sockfd, SIOCSIFNETMASK, &request) < 0)
	{
		throw Overpass::VirtualInterfaceException("unable to set netmask");
	}

	// Grab current flags on the interface (e.g. up, down, running, etc.)
	if (ioctl(sockfd, SIOCGIFFLAGS, &request) < 0)
	{
		throw Overpass::VirtualInterfaceException(
		         "unable to obtain interface flags");
	}

	// Make sure the interface is up and running (it may have already been, but
	// this won't hurt anything)
	request.ifr_flags |= (IFF_UP | IFF_RUNNING);
	if (ioctl(sockfd, SIOCSIFFLAGS, &request) < 0)
	{
		throw Overpass::VirtualInterfaceException("unable to bring up interface");
	}
}

#endif // __linux__
