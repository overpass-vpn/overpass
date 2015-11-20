#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/kern_control.h>
#include <sys/sys_domain.h>
#include <unistd.h>
#include <stdlib.h>
#include <net/if.h>
#include <net/if_utun.h>
#include <arpa/inet.h>
#include <stdio.h>

#include <string>

bool createVirtualInterface(std::string &interfaceName,
                            int &interfaceFileDescriptor)
{
	struct ctl_info ctlInfo = {0};
	if (strlcpy(ctlInfo.ctl_name, UTUN_CONTROL_NAME, sizeof(ctlInfo.ctl_name)) >= sizeof(ctlInfo.ctl_name))
	{
		perror("UTUN_CONTROL_NAME too long");
		return false;
	}
	int fd = socket(PF_SYSTEM, SOCK_DGRAM, SYSPROTO_CONTROL);

	if (fd == -1)
	{
		perror("socket(SYSPROTO_CONTROL)");
		return false;
	}

	if (ioctl(fd, CTLIOCGINFO, &ctlInfo) == -1)
	{
		perror("ioctl(CTLIOCGINFO)");
		close(fd);
		return false;
	}

	struct sockaddr_ctl sc = {0};
	sc.sc_id = ctlInfo.ctl_id;
	sc.sc_len = sizeof(sc);
	sc.sc_family = AF_SYSTEM;
	sc.ss_sysaddr = AF_SYS_CONTROL;
	sc.sc_unit = 2;

	if (connect(fd, (struct sockaddr *)&sc, sizeof(sc)) == -1)
	{
		perror("connect(AF_SYS_CONTROL)");
		close(fd);
		return false;
	}

	char sockName[IFNAMSIZ];
	socklen_t sockLen = sizeof(sockName);
	
	// Get the interface name
	if (getsockopt(fd, SYSPROTO_CONTROL, UTUN_OPT_IFNAME, &sockName, &sockLen) == -1)
	{
		perror("Problem getting file path");
		close(fd);
		return false;
	}

	interfaceName = sockName;
	interfaceFileDescriptor = fd;

	return true;
}

bool assignDeviceAddress(const std::string &interfaceName,
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
		perror("Unable to set IP address");
		return false;
	}

	// Set netmask
	inet_pton(PF_INET, netmask.c_str(), &socketAddress->sin_addr);
	if (ioctl(sockfd, SIOCSIFNETMASK, &request) < 0)
	{
		perror("Unable to set netmask");
		return false;
	}

	// Grab current flags on the interface (e.g. up, down, running, etc.)
	if (ioctl(sockfd, SIOCGIFFLAGS, &request) < 0)
	{
		perror("Unable to obtain interface flags");
		return false;
	}

	// Make sure the interface is up and running (it may have already been, but
	// this won't hurt anything)
	request.ifr_flags |= (IFF_UP | IFF_RUNNING);
	if (ioctl(sockfd, SIOCSIFFLAGS, &request) < 0)
	{
		perror("Unable to set interface up");
		return false;
	}

	return true;
}
