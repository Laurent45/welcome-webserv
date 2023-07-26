/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServ.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfrederi <lfrederi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/01 19:39:13 by lfrederi          #+#    #+#             */
/*   Updated: 2023/07/25 22:17:54 by lfrederi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServ.hpp"
#include "Exception.hpp"
#include "Client.hpp"
#include "Cgi.hpp"
#include "Response.hpp"

#include <cstring> // strerror, bzero
#include <errno.h> // errno
#include <unistd.h> // close
#include <sys/socket.h> // accept
#include <fcntl.h> // fcntl
#include <stdio.h> // REMOVE

bool g_run = true;

/*****************
* CANNONICAL FORM
*****************/

WebServ::WebServ()
{}

WebServ::WebServ(WebServ const & copy)
    :   _epollFd(copy._epollFd),
        _mapFd(copy._mapFd),
		_clients(copy._clients)
{}

WebServ &   WebServ::operator=(WebServ const & rhs)
{
    if (this != &rhs)
    {
        _epollFd = rhs._epollFd;
        _mapFd = rhs._mapFd;
		_clients = rhs._clients;
    }
    return (*this);
}

WebServ::~WebServ()
{
	std::map<int, AFileDescriptor *>::iterator it = this->_mapFd.begin();
	for (; it != this->_mapFd.end(); it++)
	{
		if (dynamic_cast<Client *>(it->second) || dynamic_cast<Server *>(it->second))
		{
			delete it->second;
			close(it->first);
		}
	}
	close(_epollFd);

}
/******************************************************************************/

/****************
* PUBLIC METHODS
****************/

/**
 * @brief Bind a file descriptor with a Afiledescriptor object in mapFd 
 * @param fd File descriptor number
 * @param fileDescriptor File descriptor object
 */
void    WebServ::addFd(int fd, AFileDescriptor * fileDescriptor)
{
    _mapFd[fd] = fileDescriptor;
}


/**
 * @brief 
 * @param fd 
 */
void	WebServ::removeFd(int fd)
{
	_mapFd.erase(fd);
}


void	WebServ::removeClient(int fd)
{
	std::vector<Client *>::iterator it = _clients.begin();
	for (; it != _clients.end(); it++)
	{
		if ((*it)->getFd() == fd)
		{
			_clients.erase(it);
			break;
		}
	}
}


/**
 * @brief Keep a track of client start time
 * @param clientInfo A pair which bind a fd with a start time
 */
void	WebServ::addClient(Client * client)
{
	_clients.push_back(client);
}


/**
 * @brief Init an epoll and add listening server/socket
 * @throw EpollInitError
 */
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
        event.data.fd = it->first;
        if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, it->first, &event) < 0)
            throw EpollInitError(strerror(errno));
    }
}


/**
 * @brief Start webServ and handle events with epoll
 */
void    WebServ::start()
{
    int nfds;
    struct epoll_event events[MAX_EVENTS];

	while (g_run)
	{
        // How handle if nfds < 0
		nfds = epoll_wait(this->_epollFd, events, MAX_EVENTS, 1);

		for (int i = 0; i < nfds; i++)
		{
			int					fd = events[i].data.fd;
			uint32_t			event = events[i].events;
			AFileDescriptor *	aFd = _mapFd[fd];
			
			if (event & EPOLLIN)
				aFd->doOnRead();
			if (event & EPOLLOUT)
				aFd->doOnWrite();
			if (!(event & EPOLLIN) && !(event & EPOLLOUT))
				aFd->doOnError(event);
		}
		
		if (!_clients.empty())
		{
			Client * client = _clients[0];
			if (client->timeoutReached())
			{
				Response::errorResponse(REQUEST_TIMEOUT, *client);
				_clients.erase(_clients.begin());
			}
		}
	}
}


/**
 * @brief Update a fd in epoll according event and mod
 * @param fd Fd number
 * @param event see man epoll (Ex: EPOLLIN, EPOLLOUT ...)
 * @param mod see man epoll (Ex: EPOLL_CTL_ADD, EPOLL_CTL_MOD ...)
 * @throw EpollInitError
 */
void	WebServ::updateEpoll(int fd, u_int32_t event, int mod)
{
	struct epoll_event ev;

	bzero(&ev, sizeof(ev));
	ev.events = event;
	ev.data.fd = fd;
	if (epoll_ctl(_epollFd, mod, fd, &ev) < 0)
		throw EpollInitError(strerror(errno));
}


/**
 * @brief Remove fd in mapFd. Close fd, delete object if present and erase item
 * in mapFd and remove fd is present in clientTimes
 * @param fd Fd number
 */
void	WebServ::clearFd(int fd)
{
	std::vector<Client *>::iterator it = _clients.begin();
	for (; it != _clients.end(); it++)
	{
		if ((*it)->getFd() == fd)
		{
			_clients.erase(it);
			break;
		}
	}

    updateEpoll(fd, 0, EPOLL_CTL_DEL);
	close(fd);

	if (_mapFd.find(fd) == _mapFd.end())
		return ;
	if (_mapFd[fd])
		delete _mapFd[fd];
	_mapFd.erase(fd);
	
}
/******************************************************************************/

/****************
* PRIVATE METHODS
****************/

/******************************************************************************/


/***********************
* PUBLIC STATIC METHODS
***********************/
