#include <iostream>
#include <memory>
#include <vector>
#include <thread>

#include <boost/asio.hpp>
#include <boost/program_options.hpp>

#include "include/version.h"
#include "include/virtual_interface.h"

void callback(std::shared_ptr<std::vector<boost::uint8_t> > buffer)
{
	std::cout << "Received " << buffer->size() << " bytes: " << buffer->data() << std::endl;
}

void parseParameters(
		int argc, char *argv[],
		boost::program_options::options_description &availableParameters,
		boost::program_options::variables_map &parameters)
{
	availableParameters.add_options()
			("help,h", "Print help message")
			("version,v", "Print version number");

	boost::program_options::store(
				boost::program_options::parse_command_line(argc, argv,
														   availableParameters),
				parameters);

	boost::program_options::notify(parameters);
}

int main(int argc, char *argv[])
{
	boost::program_options::options_description availableParameters("Parameters");
	boost::program_options::variables_map parameters;

	try
	{
		parseParameters(argc, argv, availableParameters, parameters);
	}
	catch(const boost::program_options::error &exception)
	{
		std::cerr << "Unable to parse parameters: " << exception.what() << std::endl;
		return 1;
	}

	if (parameters.count("help"))
	{
		std::cout << availableParameters << std::endl;
		return 0;
	}

	if (parameters.count("version"))
	{
		std::cout << "Overpass v"
				  << LIBOVERPASS_VERSION_MAJOR << "."
				  << LIBOVERPASS_VERSION_MINOR << "."
				  << LIBOVERPASS_VERSION_PATCH << std::endl;
		return 0;
	}

	std::string interfaceName = "ovp%d";
	int interfaceFileDescriptor;
	if (!createVirtualInterface(interfaceName, interfaceFileDescriptor))
	{
		std::cerr << "Unable to create virtual interface." << std::endl;
		return 1;
	}

	if (!assignDeviceAddress(interfaceName, "11.11.11.1", "255.255.255.0"))
	{
		std::cerr << "Unable to assign address." << std::endl;
		return 1;
	}

	int val;
	std::cin >> val;

	return 0;
}
