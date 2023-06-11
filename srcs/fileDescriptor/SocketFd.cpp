/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SocketFd.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfrederi <lfrederi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/18 16:02:19 by lfrederi          #+#    #+#             */
/*   Updated: 2023/06/11 21:52:43 by lfrederi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "SocketFd.hpp"
#include "Debugger.hpp"
#include "Response.hpp"
#include "Exception.hpp"
#include "WebServ.hpp"
#include "Cgi.hpp"

#include <cstddef>
#include <sys/epoll.h>
#include <iostream>
#include <unistd.h> // read
#include <cstdio> // perror
#include <fstream>
#include <sys/socket.h> // recv
#include <algorithm> // search

/*****************
* CANNONICAL FORM
*****************/

SocketFd::SocketFd(void) 
: AFileDescriptor(), _serverInfo(NULL), _responseReady(false)
{}

SocketFd::SocketFd(SocketFd const & copy)
	:	AFileDescriptor(copy), _serverInfo(copy._serverInfo),
		_responseReady(copy._responseReady)
{}

SocketFd & SocketFd::operator=(SocketFd const & rhs)
{
	if (this != &rhs)
	{
		this->_fd = rhs._fd;
		this->_rawData = rhs._rawData;
		this->_open = rhs._open;
		this->_responseReady = rhs._responseReady;
	}

	return (*this);

}

SocketFd::~SocketFd()
{}
/******************************************************************************/

/**************
* CONSTRUCTORS
***************/

SocketFd::SocketFd(int fd, Server const & serverInfo)
	:	AFileDescriptor(fd), _serverInfo(&serverInfo)
{}
/******************************************************************************/

/***********
* ACCESSORS
************/

Request const &	SocketFd::getRequest() const
{
	return (this->_request);
}

Server const &	SocketFd::getServerInfo() const
{
	return (*this->_serverInfo);
}
/******************************************************************************/

/****************
* PUBLIC METHODS
****************/

/// @brief 
/// @return 
void		SocketFd::readRequest(int epoll)
{
	char	buffer[BUFFER_SIZE];
	ssize_t	n;
	std::vector<unsigned char>::iterator it;

	if ((n = recv(this->_fd, buffer, BUFFER_SIZE, 0)) > 0)
		this->_rawData.assign(buffer, buffer + n);
	
	// Try to read next time fd is NON_BLOCK and we must not check errno
	if (n < 0)
		return ;
	// Socket connection close, a EOF was present
	if (n == 0)
		throw ClientCloseConnection();
	// Try to retrieve request line
	if (_request.getHttpMethod().empty() && searchRequestLine() == false)
		return ;
	// Try to retrieve headers
	if (_request.getHeaders().empty() && searchHeaders() == false)
		return ;
	// Try to retrieve message body if necessary
	if (_request.hasMessageBody() && _request.handleMessageBody(_rawData) == false)
		return ;
	_rawData.erase(_rawData.begin(), _rawData.end());
	WebServ::updateEpoll(epoll, _fd, EPOLLOUT, EPOLL_CTL_MOD);
	
}

/// @brief 
void	SocketFd::sendResponse(int epoll, std::map<int, AFileDescriptor *> & mapFd)
{
	Cgi * cgi = NULL;

	if (_responseReady == true)
	{
		send(_fd, &(_rawData[0]), _rawData.size(), 0);
		return ;
	}
	
	if (_request.getExtension().compare("php") == 0)
	{
		cgi = new Cgi(*this);
		if (cgi->run() < 0)
		{
			delete cgi;
			Response::createResponse(_rawData);
			_responseReady = true;
			WebServ::updateEpoll(epoll, _fd, EPOLLOUT, EPOLL_CTL_MOD);
			return ;
		}
		mapFd[cgi->getReadFd()] = cgi;
		WebServ::updateEpoll(epoll, _fd, 0, EPOLL_CTL_MOD);
		WebServ::updateEpoll(epoll, cgi->getReadFd(), EPOLLIN, EPOLL_CTL_ADD);
	}
	else
	{
		Response::createResponse(_rawData);
		_responseReady = true;
		WebServ::updateEpoll(epoll, _fd, EPOLLOUT, EPOLL_CTL_MOD);
	}
}


/******************************************************************************/

/*****************
* PRIVATE METHODS
*****************/

bool	SocketFd::searchRequestLine()
{
	std::vector<unsigned char>::iterator it;
	unsigned char src[] = {'\r', '\n'};

	it = std::search(_rawData.begin(), _rawData.end(), src, src + 2);
	if (it == _rawData.end())
		return false;
	if (!_request.handleRequestLine(std::string(_rawData.begin(), it)))
		throw RequestError("Invalid request line");
	_rawData.erase(_rawData.begin(), it);
	return true;
}

bool	SocketFd::searchHeaders()
{
	std::vector<unsigned char>::iterator it;
	unsigned char src[] = {'\r', '\n', '\r', '\n'};

	it = std::search(_rawData.begin(), _rawData.end(), src, src + 4);
	if (it == _rawData.end())
		return false;
	if (!_request.handleHeaders(std::string(_rawData.begin(), it)))
		throw RequestError("Invalid headers");
	_rawData.erase(_rawData.begin(), it + 4);
	return true;
}