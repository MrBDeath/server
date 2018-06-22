

#include <getopt.h>
#include <iostream>
#include "config.h"
#include "log.h"
#include "server.h"

int main(int argc, char* argv[])
{
	int next_option;
	char *config_path = nullptr;
	const char* short_options = "c:";
	const struct option long_options[] =
	{
			{"config", 	1, nullptr, 'c' },
			{nullptr,	0, nullptr,  0  }
	};

	do
	{
		next_option = getopt_long (argc, argv, short_options,
								   long_options, nullptr);
		switch(next_option) {
			case 'c':
				config_path = optarg; break;

			case -1:
				break;
			default:
				break;
		}

	} while(next_option != -1);

	Config main_cfg;
	if(!main_cfg.GetSettsFromFile(config_path))
		std::cerr << "Can't read config\n";

	Log* log = Log::Instance();
	log->init(main_cfg.GetLogPath());

	Server main_srv;
    if(!main_srv.Start(main_cfg))
    {
        ERROR("Closing the server...");
        log->destroy();
        return EXIT_FAILURE;
    }

    log->destroy();
	return EXIT_SUCCESS;
}
