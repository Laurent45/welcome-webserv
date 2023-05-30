
#include <iostream>
#include <cstring>
#include "ServerConf.hpp"
#include "Debugger.hpp"

extern volatile bool g_run;
int									g_epollFd; // REMOVE


static int  printUsage(char const *const prog_name) {
    std::cerr << "Usage: " << prog_name << " [-v] [file.conf]" << std::endl;
    return (1);
}

static int process_server(std::vector<ServerConf> serverlist)
{
		std::map<int, ServerConf *> serverFd;
	
	try
	{
		serverFd = createServerSockets(serverlist);
		// Servers are in a map with socket number as index, ans server as content, ready to go!
		//startEpollServer(serverFd);
		//startServer(serverFd);

	}
	catch(const std::exception& e)
	{
		 std::cerr << "Error Server Execution: " << e.what() << std::endl;
        return (1);
	}
	return (0);
}



int     main(int argc, const char **argv) {
    std::vector<ServerConf> serverlist;
	argc = 1;
    if (argv[argc] && !std::strcmp(argv[argc], "-v"))
	{
        DEBUG_START(true);
		argc++;
	}
    if (argv[argc] == NULL)
        argv[argc] = "conf/default.conf";
    else if (argv[argc + 1] != NULL)
        return (printUsage(argv[0]));
    try {
        serverlist = parseConfFile(argv[argc]);
		//autoindex autind("/");
		//std::cout<<autind.getIndexPage()<<std::endl;
	//	std::cout << serverlist << std::endl;
    }
    catch (const std::exception &e) {
        std::cerr << "Error config: " << e.what() << std::endl;
        return (1);
    }
	return (process_server(serverlist));
}
