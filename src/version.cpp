#include <sstream>

#include "version.h"

std::string Overpass::version()
{
	std::stringstream stream;
	stream << LIBOVERPASS_VERSION_MAJOR << "."
	       << LIBOVERPASS_VERSION_MINOR << "."
	       << LIBOVERPASS_VERSION_PATCH;
	return stream.str();
}
