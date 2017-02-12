#include <iostream>
#include <thread>

#include <boost/program_options.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/asio/ip/address.hpp>

#include "version.h"
#include "overpass_server.h"

void parseParameters(
      int argc, char *argv[],
      boost::program_options::options_description &availableParameters,
      boost::program_options::variables_map &parameters)
{
	using boost::program_options::value;

	availableParameters.add_options()
	      ("help,h", "Print help message")
	      ("version,v", "Print version number")
	      ("address", value<std::string>(), "Selected Overpass address")
	      ("client,c", value<std::vector<std::string>>(),
	       "<overpass client IP>:<external IP>");

	using boost::program_options::store;
	using boost::program_options::parse_command_line;

	store(parse_command_line(argc, argv, availableParameters), parameters);

	using boost::program_options::notify;
	notify(parameters);
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
		std::cout << "Overpass v" << Overpass::version() << std::endl;
		return 0;
	}

	if (!parameters.count("address"))
	{
		std::cerr << "--address option is required" << std::endl;
		return 1;
	}

	std::string overpassAddress = parameters["address"].as<std::string>();

	std::shared_ptr<boost::asio::io_service> ioService(
	         new boost::asio::io_service);
	std::unique_ptr<Overpass::OverpassServer> server;

	try
	{
		server.reset(new Overpass::OverpassServer(
		                ioService, "ovp%d", overpassAddress, "255.255.255.0",
		                "0.0.0.0", 14358));
	}
	catch (const Overpass::Exception &exception)
	{
		std::cerr << exception.what() << std::endl;
		return 1;
	}

	if (parameters.count("client"))
	{
		std::vector<std::string> clients = parameters["client"].as<std::vector<std::string>>();
		for (const auto &client : clients)
		{
			std::vector<std::string> substrings;
			boost::split(substrings, client, boost::is_any_of(":"));
			if (substrings.size() != 2)
			{
				std::cerr << "Invalid client specification: " << client
				          << std::endl;
				return 1;
			}

			auto overpassAddress = boost::asio::ip::address::from_string(
			                          substrings.at(0));
			auto externalAddress = boost::asio::ip::address::from_string(
			                          substrings.at(1));

			std::cout << "Adding known client mapping "
			          << overpassAddress.to_string() << " -> "
			          << externalAddress.to_string() << std::endl;;
			server->addKnownClient(overpassAddress, externalAddress);
		}
	}
	else
	{
		std::cout << "No known clients... Overpass functionality will be "
		          << "limited." << std::endl;
	}

	// Construct a signal set registered for process termination.
	boost::asio::signal_set signal_set(*ioService, SIGINT, SIGTERM);

	// Start an asynchronous wait for one of the signals to occur.
	using boost::system::error_code;
	signal_set.async_wait(
	         [&ioService](const error_code& error, int /*signalNumber*/)
	{
		if (error)
		{
			std::cerr << "Got error: " << error << std::endl;
		}

		std::cout << "Caught signal: requesting stop" << std::endl;

		ioService->stop();
	});

	// Make sure we have at least two threads
	auto numberOfCores = std::max(static_cast<unsigned int>(2),
	                              std::thread::hardware_concurrency());

	std::cout << "Firing up " << numberOfCores << " threads..." << std::endl;

	std::vector<std::thread> threadPool;
	for (unsigned int i = 0; i < numberOfCores; ++i)
	{
		threadPool.push_back(std::thread([ioService](){ioService->run();}));
	}

	// This will block, and the current thread will begin serving the IO service
	// until it is stopped, at which time execution will resume here.
	ioService->run();

	std::cout << "Stopping..." << std::endl;

	std::for_each(threadPool.begin(), threadPool.end(),
	              [](std::thread &thread)
	{
		thread.join();
	});

	return 0;
}
