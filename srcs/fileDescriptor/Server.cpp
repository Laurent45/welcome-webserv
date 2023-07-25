/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfrederi <lfrederi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/19 18:05:52 by lfrederi          #+#    #+#             */
/*   Updated: 2023/07/25 21:29:38 by lfrederi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Exception.hpp"
#include "TimeUtils.hpp"
#include "Client.hpp"
#include "WebServ.hpp"

#include <unistd.h>		// close
#include <sys/socket.h> // accept
#include <fcntl.h>		// fcntl
#include <cstring>		// bzero
#include <sys/epoll.h>	// epoll_ctl
#include <cerrno>		// errno

/*****************
 * CANNONICAL FORM
 *****************/

Server::Server(void) : AFileDescriptor()
{}

Server::Server(Server const &src)
	: 	AFileDescriptor(src),
		_serverConfs(src._serverConfs)
{}

Server &	Server::operator=(Server const &src)
{
	if (this != &src)
	{
		_fd = src._fd;
		_webServ = src._webServ;
		_serverConfs = src._serverConfs;
	}
	return *this;
}

Server::~Server()
{}
/******************************************************************************/

/**************
 * CONSTRUCTORS
 ***************/

Server::Server(int fd, WebServ & webServ, std::vector<ServerConf> const & serverConfs)
	: 	AFileDescriptor(fd, webServ),
		_serverConfs(serverConfs)
{}
/******************************************************************************/

/****************
 * PUBLIC METHODS
 ****************/

/**
 * @brief 
 * @param webServ 
 */
void	Server::doOnRead()
{
    int			clientSocket;
	Client *	client = NULL;

	if ((clientSocket = accept(_fd, NULL, NULL)) < 0)
	{
		std::cerr << "Accept error" << std::endl;
		return ;
	}

	try
	{
		if (fcntl(clientSocket, F_SETFL, O_NONBLOCK) < 0)
			throw FileDescriptorError(strerror(errno));
		_webServ->updateEpoll(clientSocket, EPOLLIN, EPOLL_CTL_ADD);
	} 
	catch (std::exception & e)
	{
		close(clientSocket);
		return ;
	}

	client = new Client(clientSocket, *_webServ, _serverConfs);
	_webServ->addFd(clientSocket, client);
}


/**
 * @brief 
 */
void	Server::doOnWrite()
{
}


/**
 * @brief 
 * @param event 
 */
void	Server::doOnError(uint32_t event)
{
	std::cout << "Client on error, event = " << event << std::endl;
}
/******************************************************************************/