/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfrederi <lfrederi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/29 22:17:26 by lfrederi          #+#    #+#             */
/*   Updated: 2023/07/23 13:01:34 by lfrederi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <cstring>
#include "WebServ.hpp"
#include "Debugger.hpp"
#include "Parser.hpp"

#include <csignal>

void     handleSigInt(int code);

int     main(int argc, const char **argv) 
{
	WebServ webServ;
    signal(SIGINT, handleSigInt);

	argc = 1;
    if (argv[argc] && !std::strcmp(argv[argc], "-v"))
	{
        DEBUG_START(true);
		argc++;
	}

    if (argv[argc] == NULL)
        argv[argc] = "conf/default.conf";
    else if (argv[argc + 1] != NULL)
    {
        std::cerr << "Usage: " << argv[0] << " [-v] [file.conf]" << std::endl;
        return (1);
    }

    try
	{
        Parser parser(argv[argc]);
		parser.parseConfFile(webServ);
        webServ.epollInit();
    }
    catch (const std::exception &e) {
        std::cerr << "Error config: " << e.what() << std::endl;
        return (1);
    }

    webServ.start();

	return (0);
}


void     handleSigInt(int code)
{
    //(void) code;
    std::cout << "Catch signal: " << code << std::endl;
    g_run = false;
}      