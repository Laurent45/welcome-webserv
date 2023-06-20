/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServ.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfrederi <lfrederi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/01 19:39:13 by lfrederi          #+#    #+#             */
/*   Updated: 2023/06/19 19:29:56 by lfrederi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServ.hpp"
#include "Exception.hpp"
#include "SocketFd.hpp"
#include "Cgi.hpp"
#include <cstring> // strerror, bzero
#include <errno.h> // errno
#include <unistd.h> // close
#include <sys/socket.h> // accept
#include <fcntl.h> // fcntl
#include <stdio.h> // REMOVE

/*****************
* CANNONICAL FORM
*****************/

WebServ::WebServ()
{}

WebServ::WebServ(WebServ const & copy)
    :   _epollFd(copy._epollFd),
        _mapFd(copy._mapFd)
{}

WebServ &   WebServ::operator=(WebServ const & rhs)
{
    if (this != &rhs)
    {
        this->_epollFd = rhs._epollFd;
        this->_mapFd = rhs._mapFd;
    }
    return (*this);
}

WebServ::~WebServ()
{
	std::map<int, AFileDescriptor *>::iterator it = this->_mapFd.begin();
	for (; it != this->_mapFd.end(); it++)
	{
		delete it->second;
		close(it->first);
	}
	close(_epollFd);

}
/******************************************************************************/

/****************
* PUBLIC METHODS
****************/

/// @brief Add a pair to map servers
/// @param server <file descriptor, Server object>
void    WebServ::addServer(int socketFd, Server const & server)
{
	Server * serv = new Server(server);
	serv->setFd(socketFd);
    _mapFd[socketFd] = serv;
}


/// @brief Init an epoll and add listening socket
/// @throw EpollInitError
void    WebServ::epollInit()
{
    std::map<int, AFileDescriptor *>::const_iterator   	it;
    struct epoll_event                      			event;


    if ((_epollFd = epoll_create(1)) < 0)
        throw EpollInitError(strerror(errno));

    bzero(&event, sizeof(event));
    event.events = EPOLLIN;
    for (it = _mapFd.begin(); it != _mapFd.end(); it++)
    {
		it->second->setEpollFd(_epollFd);
        event.data.fd = it->first;
        if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, it->first, &event) < 0)
            throw EpollInitError(strerror(errno));
    }
}

/// @brief Start webServ and handle events with epoll
void    WebServ::start()
{
    int nfds;
    struct epoll_event events[MAX_EVENTS];
    int j = 0;

	while (j != 1)
	{
        // How handle if nfds < 0
		nfds = epoll_wait(this->_epollFd, events, MAX_EVENTS, -1);

		for (int i = 0; i < nfds; i++)
		{
			int					fd = events[i].data.fd;
			uint32_t			event = events[i].events;
			AFileDescriptor *	aFd = _mapFd[fd];
			
			if (event & EPOLLIN)
				aFd->doOnRead(_mapFd);
			if (event & EPOLLOUT)
				aFd->doOnWrite(_mapFd);
			if (!(event & EPOLLIN) && !(event & EPOLLOUT))
				aFd->doOnError(_mapFd, event);
		}
	}
}
/******************************************************************************/

/****************
* PRIVATE METHODS
****************/

/******************************************************************************/


/***********************
* PUBLIC STATIC METHODS
***********************/
void	WebServ::updateEpoll(int epoll, int fd, u_int32_t event, int mod)
{
	struct epoll_event ev;

	bzero(&ev, sizeof(ev));
	ev.events = event;
	ev.data.fd = fd;
	if (epoll_ctl(epoll, mod, fd, &ev) < 0)
		throw EpollInitError(strerror(errno));
}


//void	WebServ::popFd(int fd)
//{
//	close()
//}

