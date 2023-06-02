/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServ.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfrederi <lfrederi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/01 19:39:13 by lfrederi          #+#    #+#             */
/*   Updated: 2023/06/02 21:36:30 by lfrederi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServ.hpp"
#include "Exception.hpp"
#include "SocketFd.hpp"
#include <sys/epoll.h> // epoll_create
#include <cstring> // strerror
#include <errno.h> // errno
#include <unistd.h> // bzero, close
#include <sys/socket.h> // accept
#include <fcntl.h> // fcntl

WebServ::WebServ()
{}

WebServ::WebServ(WebServ const & copy)
    :   _epollFd(copy._epollFd),
        _mapServers(copy._mapServers),
        _mapFileDescriptors(copy._mapFileDescriptors)
{}

WebServ &   WebServ::operator=(WebServ const & rhs)
{
    if (this != &rhs)
    {
        this->_epollFd = rhs._epollFd;
        this->_mapServers = rhs._mapServers;
        this->_mapFileDescriptors = rhs._mapFileDescriptors;
    }
    return (*this);
}

WebServ::~WebServ()
{
	std::map<int, AFileDescriptor *>::iterator it;
	for (it = this->_mapFileDescriptors.begin(); it != this->_mapFileDescriptors.end(); it++)
	{
		delete it->second;
		close(it->first);
	}

	std::map<int, Server>::iterator it1;
	for (it1 = this->_mapServers.begin(); it1 != this->_mapServers.end(); it1++)
		close(it1->first);
	close(this->_epollFd);
}

void    WebServ::addServer(std::pair<int, Server> server)
{
    this->_mapServers.insert(server);
}

void    WebServ::epollInit()
{
    std::map<int, Server>::const_iterator   it;
    struct epoll_event                      event;

    if ((this->_epollFd = epoll_create(1)) < 0)
        throw EpollInitError(strerror(errno));

    bzero(&event, sizeof(event));
    event.events = EPOLLIN;
    for (it = this->_mapServers.begin(); it != this->_mapServers.end(); it++)
    {
        event.data.fd = it->first;
        if (epoll_ctl(this->_epollFd, EPOLL_CTL_ADD, it->first, &event) < 0)
            throw EpollInitError(strerror(errno));
    }
}

void    WebServ::start()
{
    int nfds;
    struct epoll_event ev, events[MAX_EVENTS];
    int j = 0;

	while (j != 1)
	{
        // How handle if nfds < 0
		nfds = epoll_wait(this->_epollFd, events, MAX_EVENTS, -1);

		for (int i = 0; i < nfds; i++)
		{
			int			fd = events[i].data.fd;
			uint32_t	event = events[i].events;

			if (this->_mapServers.find(fd) != this->_mapServers.end())
				clientConnect(fd);
			else 
			{
				switch (event)
				{
					case EPOLLIN:
						if (this->_mapFileDescriptors[fd]->doOnRead() == Request::requestComplete)
						{
							bzero(&ev, sizeof(ev));
							ev.events = EPOLLOUT;
							ev.data.fd = fd;
                            // TODO: How handle if epoll_ctl failed
							epoll_ctl(this->_epollFd, EPOLL_CTL_MOD, fd, &ev);
						}
						break;
					case EPOLLOUT:
						this->_mapFileDescriptors[fd]->doOnWrite();
						// Don't close if if header alive is present
                        delete this->_mapFileDescriptors[fd];
						this->_mapFileDescriptors.erase(fd);
						close(fd);
						//j = 1;
						break;
					default:
						std::cout << "default => event = " << event;
						/* DEBUG_COUT("Error"); */
				}
			}

		}
	}
}

void    WebServ::clientConnect(int serverFd)
{
    int					socketConnect;
	struct epoll_event	event;

	if ((socketConnect = accept(serverFd, NULL, NULL)) < 0)
	{
		std::cerr << "Accept error" << std::endl;
		return ;
	}

	if (fcntl(socketConnect, F_SETFL, O_NONBLOCK) < 0)
	{
		std::cerr << "Fcntl error" << std::endl;
		return ;
	}

	bzero(&event, sizeof(event));
    event.events = EPOLLIN;
	event.data.fd = socketConnect;
	if (epoll_ctl(this->_epollFd, EPOLL_CTL_ADD, socketConnect, &event) < 0)
	{
		close(socketConnect);
		std::cerr << "Epoll_ctl error" << std::endl;
		return ;
	}
	this->_mapFileDescriptors[socketConnect] = new SocketFd(socketConnect);
}