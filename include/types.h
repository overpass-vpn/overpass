#ifndef TYPES_H
#define TYPES_H

#include <vector>
#include <memory>
#include <cstdint>
#include <functional>

namespace boost
{
	namespace asio
	{
		class io_service;
	}
}

namespace Overpass
{
	typedef std::vector<uint8_t> Buffer;
	typedef std::shared_ptr<Buffer> SharedBuffer;

	typedef std::shared_ptr<boost::asio::io_service> SharedIoService;

	class Exception : public std::runtime_error
	{
		public:
			Exception(const std::string &what) :
			   std::runtime_error("Overpass error: " + what)
			{
			}
	};
}

#endif // TYPES_H
