#include <sstream>

#include <gtest/gtest.h>

#include "version.h"

TEST(Version, VersionString)
{
	std::stringstream stream;
	stream << LIBOVERPASS_VERSION_MAJOR << "."
	       << LIBOVERPASS_VERSION_MINOR << "."
	       << LIBOVERPASS_VERSION_PATCH;
	EXPECT_EQ(stream.str(), Overpass::version());
}
