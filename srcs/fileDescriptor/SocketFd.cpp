/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SocketFd.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfrederi <lfrederi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/18 16:02:19 by lfrederi          #+#    #+#             */
/*   Updated: 2023/06/19 20:04:47 by lfrederi         ###   ########.fr       */
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
: AFileDescriptor(), _responseReady(false)
{}

SocketFd::SocketFd(SocketFd const & copy)
	:	AFileDescriptor(copy),
		_rawData(copy._rawData),
		_serverInfo(copy._serverInfo),
		_serverInfoCurr(copy._serverInfoCurr),
		_request(copy._request),
		_responseReady(copy._responseReady)
{}

SocketFd & SocketFd::operator=(SocketFd const & rhs)
{
	if (this != &rhs)
	{
		_fd = rhs._fd;
		_rawData = rhs._rawData;
		_serverInfoCurr = rhs._serverInfoCurr;
		_request = rhs._request;
		_responseReady = rhs._responseReady;
	}

	return (*this);

}

SocketFd::~SocketFd()
{}
/******************************************************************************/

/**************
* CONSTRUCTORS
***************/
SocketFd::SocketFd(int epollFd, int fd, std::vector<ServerConf> const & serverInfo)
	:	AFileDescriptor(epollFd, fd), _serverInfo(serverInfo)
{}
/******************************************************************************/

/***********
* ACCESSORS
************/

Request const &	SocketFd::getRequest() const
{
	return (this->_request);
}

ServerConf const &	SocketFd::getServerInfo() const
{
	return (this->_serverInfoCurr);
}
/******************************************************************************/

/****************
* PUBLIC METHODS
****************/

/// @brief 
/// @return 
void		SocketFd::doOnRead(std::map<int, AFileDescriptor *> & mapFd)
{
	char	buffer[BUFFER_SIZE];
	ssize_t	n;
	std::vector<unsigned char>::iterator it;

	if ((n = recv(_fd, buffer, BUFFER_SIZE, 0)) > 0)
		_rawData.assign(buffer, buffer + n);
	
	// Try to read next time fd is NON_BLOCK and we must not check errno
	if (n < 0)
		return ;
	// Socket connection close, a EOF was present
	if (n == 0)
	{
		close(_fd);
		mapFd.erase(_fd);
		return ;
	}
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
	_serverInfoCurr = _serverInfo[0]; // TODO: Find the good server info
	WebServ::updateEpoll(_epollFd, _fd, EPOLLOUT, EPOLL_CTL_MOD);
}

/// @brief 
void	SocketFd::doOnWrite(std::map<int, AFileDescriptor *> & mapFd)
{
	Cgi * cgi = NULL;

	if (_responseReady == true)
	{
		send(_fd, &(_rawData[0]), _rawData.size(), 0);
		WebServ::updateEpoll(_epollFd, _fd, EPOLLIN, EPOLL_CTL_MOD);
		_responseReady = false;
		_request = Request();
		return ;
	}
	
	if (_request.getExtension().compare("php") == 0)
	{
		cgi = new Cgi(*this);
		cgi->setEpollFd(_epollFd);
		if (cgi->run() < 0)
		{
			delete cgi;
			Response::createResponse(_rawData, *this);
			_responseReady = true;
			WebServ::updateEpoll(_epollFd, _fd, EPOLLOUT, EPOLL_CTL_MOD);
			return ;
		}
		mapFd[cgi->getReadFd()] = cgi;
		WebServ::updateEpoll(_epollFd, _fd, 0, EPOLL_CTL_MOD);
		WebServ::updateEpoll(_epollFd, cgi->getReadFd(), EPOLLIN, EPOLL_CTL_ADD);
	}
	else
	{
		Response::createResponse(_rawData, *this);
		_responseReady = true;
		WebServ::updateEpoll(_epollFd, _fd, EPOLLOUT, EPOLL_CTL_MOD);
	}
}

/// @brief 
/// @param mapFd 
/// @param event 
void	SocketFd::doOnError(std::map<int, AFileDescriptor *> & mapFd, uint32_t event)
{
	std::cout << "SocketFd on error, event = " << event << std::endl;
	close(_fd);
	mapFd.erase(_fd);
}

void	SocketFd::responseCgi(std::string const & response)
{
	_responseReady = true;
	_rawData.assign(response.begin(), response.end());
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