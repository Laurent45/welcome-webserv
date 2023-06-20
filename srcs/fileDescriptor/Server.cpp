/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfrederi <lfrederi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/19 18:05:52 by lfrederi          #+#    #+#             */
/*   Updated: 2023/06/19 19:34:08 by lfrederi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Exception.hpp"
#include "SocketFd.hpp"
#include "WebServ.hpp"

#include <unistd.h>		// close
#include <sys/socket.h> // accept
#include <fcntl.h>		// fcntl
#include <cstring>		// bzero
#include <sys/epoll.h>	// epoll_ctl
#include <cerrno>		// errno


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
		_epollFd = src._epollFd;
		_serverConfs = src._serverConfs;
	}
	return *this;
}

Server::~Server()
{}

Server::Server(std::vector<ServerConf> const & serverConfs)
	: 	AFileDescriptor(),
		_serverConfs(serverConfs)
{}

void	Server::doOnRead(std::map<int, AFileDescriptor *> & mapFd)
{
    int		cs;

	if ((cs = accept(_fd, NULL, NULL)) < 0)
	{
		std::cerr << "Accept error" << std::endl;
		return ;
	}

	if (fcntl(cs, F_SETFL, O_NONBLOCK) < 0)
		throw FileDescriptorError(strerror(errno));

	try
	{
		WebServ::updateEpoll(_epollFd, cs, EPOLLIN, EPOLL_CTL_ADD);
	} 
	catch (EpollInitError & e)
	{
		close(cs);
		throw e;
	}

	mapFd[cs] = new SocketFd(_epollFd, cs, _serverConfs);
}

void	Server::doOnWrite(std::map<int, AFileDescriptor *> & mapFd)
{
	(void) mapFd;
}

void	Server::doOnError(std::map<int, AFileDescriptor *> & mapFd, uint32_t event)
{
	(void) mapFd;
	std::cout << "SocketFd on error, event = " << event << std::endl;
}