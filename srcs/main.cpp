/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfrederi <lfrederi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/18 15:44:08 by lfrederi          #+#    #+#             */
/*   Updated: 2023/05/18 17:56:43 by lfrederi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "SocketFd.hpp"

#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT_LISTEN 18000
#define N_QUEUED	10
#define	BUFFER_SIZE	1080

int		couldFailedAndExit(int retFunction, const char *functionName)
{
	if (!(retFunction < 0))
		return (retFunction);
	perror(functionName);
	exit(EXIT_FAILURE);
}

int main()
{
	struct sockaddr_in addr;
	int serverFd;
	int	socketFd;

	serverFd = couldFailedAndExit(socket(AF_INET, SOCK_STREAM, 0), "socket");
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT_LISTEN);
	addr.sin_addr.s_addr = INADDR_ANY;

	couldFailedAndExit(bind(serverFd, (struct sockaddr *) &addr, sizeof(addr)), "bind");
	couldFailedAndExit(listen(serverFd, N_QUEUED), "listen");

	std::cout << "Server is listenning on port " << PORT_LISTEN << std::endl;

	socketFd = couldFailedAndExit(accept(serverFd, NULL, NULL), "accept");
	
	char buffer[BUFFER_SIZE];
	int n = read(socketFd, buffer, BUFFER_SIZE);
	buffer[n] = '\0';
	std::cout << buffer;

	close(socketFd);
	close(serverFd);

	return 0;
}
