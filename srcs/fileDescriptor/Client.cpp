/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfrederi <lfrederi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/18 16:02:19 by lfrederi          #+#    #+#             */
/*   Updated: 2023/07/19 21:51:02 by lfrederi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Debugger.hpp"
#include "StringUtils.hpp"
#include "Response.hpp"
#include "Exception.hpp"
#include "WebServ.hpp"

#include <cstddef>
#include <cstring> // strncmp
#include <sys/epoll.h>
#include <iostream>
#include <unistd.h> // read
#include <cstdio>	// perror
#include <fstream>
#include <sys/socket.h> // recv
#include <algorithm>	// search

/*****************
 * CANNONICAL FORM
 *****************/

Client::Client(void)
	: AFileDescriptor(), _responseReady(false)
{
}

Client::Client(Client const &copy)
	: AFileDescriptor(copy),
	  _rawData(copy._rawData),
	  _serverInfo(copy._serverInfo),
	  _serverInfoCurr(copy._serverInfoCurr),
	  _request(copy._request),
	  _responseReady(copy._responseReady),
	  _cgi(copy._cgi)
{
}

Client &Client::operator=(Client const &rhs)
{
	if (this != &rhs)
	{
		_fd = rhs._fd;
		_rawData = rhs._rawData;
		_serverInfo = rhs._serverInfo;
		_serverInfoCurr = rhs._serverInfoCurr;
		_request = rhs._request;
		_responseReady = rhs._responseReady;
		_cgi = rhs._cgi;
	}

	return (*this);
}

Client::~Client()
{
}
/******************************************************************************/

/**************
 * CONSTRUCTORS
 ***************/
Client::Client(int fd, std::vector<ServerConf> const &serverInfo)
	: 	AFileDescriptor(fd),
		_serverInfo(serverInfo)
{
}
/******************************************************************************/

/***********
 * ACCESSORS
 ************/

Request const &Client::getRequest() const
{
	return (this->_request);
}

ServerConf const &Client::getServerInfo() const
{
	return (this->_serverInfoCurr);
}
/******************************************************************************/

/****************
 * PUBLIC METHODS
 ****************/

/**
 * @brief Read data in fd and try to construct the request. While request is
 * complete retrive correct server information and update fd to EPOLLOUT
 * @param webServ Reference to webServ
 */
void Client::doOnRead(WebServ &webServ)
{
	char buffer[BUFFER_SIZE];
	ssize_t n;

	if ((n = recv(_fd, buffer, BUFFER_SIZE, 0)) > 0)
		_rawData.assign(buffer, buffer + n);

	if (n < 0) // Try to read next time fd is NON_BLOCK and we can't check errno
		return;
	else if (n == 0) // Client close connection
		webServ.removeFd(_fd);
	else
	{
		try
		{
			if (_request.getHttpMethod().empty())
				searchRequestLine();
			if (_request.getHeaders().empty())
				searchHeaders();
			if (_request.hasMessageBody())
				_request.handleMessageBody(_rawData);
		}
		catch (RequestUncomplete &exception)
		{
			return;
		}
		catch (std::exception &exception)
		{
			handleException(exception);
			webServ.updateEpoll(_fd, EPOLLOUT, EPOLL_CTL_MOD);
			return;
		}

		_serverInfoCurr = getCorrectServer();
		webServ.updateEpoll(_fd, EPOLLOUT, EPOLL_CTL_MOD);
	}
}

/**
 * @brief Try to write response in fd if is ready otherwise create response
 * @param webServ Reference to webServ
 */
void Client::doOnWrite(WebServ & webServ)
{
	if (_responseReady == true)
	{
		send(_fd, &(_rawData[0]), _rawData.size(), 0);

		// Reset client field for next request
		webServ.updateEpoll(_fd, EPOLLIN, EPOLL_CTL_MOD);
		_responseReady = false;
		_request = Request();
		if (_cgi.getPidChild() != -1)
		{
			webServ.eraseFd(_cgi.getReadFd());
			webServ.eraseFd(_cgi.getWriteFd());
			_cgi = Cgi();
		}
		return;
	}

	if (_request.getExtension().compare("php") == 0)
	{
		handleScript(webServ);
	}
	else
	{
		Location const * location = getLocationBlock(); 
		if (location)
			std::cout << "Location block is -> " << location->getUri() << std::endl;
		else
			std::cout << "Take root server config" << std::endl;
		getResponse();
		// postResponse();
		// deleteResponse();
		webServ.updateEpoll(_fd, EPOLLOUT, EPOLL_CTL_MOD);
	}
}

/**
 * @brief
 * @param webServ
 * @param event
 */
void Client::doOnError(WebServ &webServ, uint32_t event)
{
	std::cout << "Client on error, event = " << event << std::endl;
	webServ.removeFd(_fd);
}

/**
 * @brief
 * @param response
 */
void Client::responseCgi(std::string const &response)
{
	_responseReady = true;
	_rawData.assign(response.begin(), response.end());
}

/******************************************************************************/

/*****************
 * PRIVATE METHODS
 *****************/

void Client::searchRequestLine()
{
	std::vector<unsigned char>::iterator it;
	unsigned char src[] = {'\r', '\n'};

	it = std::search(_rawData.begin(), _rawData.end(), src, src + 2);
	if (it == _rawData.end())
		throw RequestUncomplete();
	_request.handleRequestLine(std::string(_rawData.begin(), it));
	_rawData.erase(_rawData.begin(), it);
}

void Client::searchHeaders()
{
	std::vector<unsigned char>::iterator it;
	unsigned char src[] = {'\r', '\n', '\r', '\n'};

	it = std::search(_rawData.begin(), _rawData.end(), src, src + 4);
	if (it == _rawData.end())
		throw RequestUncomplete();
	_request.handleHeaders(std::string(_rawData.begin(), it));
	_rawData.erase(_rawData.begin(), it + 4);
}

void Client::errorResponse(status_code_t status)
{
	// TODO: Find error file
	std::string extension = "html";

	std::string error = Response::errorResponse(status);
	std::vector<unsigned char> body = std::vector<unsigned char>(error.begin(), error.end());

	resp_t resp = {status, body, extension, _rawData, false};
	Response::createResponse(resp);
	_responseReady = true;
}

void Client::getResponse()
{
	// TODO: Verifier avec le serverConf le path du fichier et son existence OU errorResponse(NOT_FOUND) and change / to index.html
	std::cout << _request;

	std::vector<unsigned char> body;
	std::string filename = _serverInfoCurr.getRoot() + "/" + _request.getPathRequest();
	std::ifstream is(filename.c_str(), std::ifstream::binary);

	if (is.good())
	{
		is.seekg(0, is.end);
		int length = is.tellg();
		is.seekg(0, is.beg);

		char *buffer = new char[length];
		is.read(buffer, length);
		body = std::vector<unsigned char>(buffer, buffer + length);
		is.close();
		delete buffer;

		resp_t resp = {OK, body, _request.getExtension(), _rawData, true};
		Response::createResponse(resp);
		_responseReady = true;
	}
	else
	{
		errorResponse(NOT_FOUND);
	}
}

ServerConf Client::getCorrectServer()
{
	std::vector<ServerConf>::iterator it = _serverInfo.begin();
	for (; it != _serverInfo.end(); it++)
	{
		if (_request.getHeaders().find("Host")->second == it->getName())
			return (*it);
	}
	it = _serverInfo.begin();
	for (; it != _serverInfo.end(); it++)
	{
		std::vector<Location>::const_iterator it1 = it->getLocation().begin();
		for (; it1 != it->getLocation().end(); it1++)
			if (_request.getPathRequest() == it1->getRoot())
				return (*it);
	}
	return (*(_serverInfo.begin()));
}


void	Client::handleScript(WebServ & webServ)
{
	_cgi = Cgi(*this);

	if (_cgi.run() < 0)
	{
		errorResponse(INTERNAL_SERVER_ERROR);
		webServ.updateEpoll(_fd, EPOLLOUT, EPOLL_CTL_MOD);
		return;
	}

	webServ.addFd(_cgi.getWriteFd(), &_cgi);
	webServ.updateEpoll(_cgi.getWriteFd(), EPOLLOUT, EPOLL_CTL_ADD);

	// Set NULL at AFileDescriptor in order to avoir double free if error occur
	webServ.addFd(_cgi.getReadFd(), &_cgi);
	webServ.updateEpoll(_cgi.getReadFd(), 0, EPOLL_CTL_ADD);

	webServ.updateEpoll(_fd, 0, EPOLL_CTL_MOD);
}


void Client::handleException(std::exception &exception)
{
	DEBUG_COUT(exception.what());
	try
	{
		dynamic_cast<RequestError &>(exception);
		errorResponse(BAD_REQUEST);
	}
	catch (std::exception &exception)
	{
		errorResponse(INTERNAL_SERVER_ERROR);
	}
}

Location const * Client::getLocationBlock()
{
	std::vector<Location>::const_reverse_iterator it = _serverInfoCurr.getLocation().rbegin();

	for (; it != _serverInfoCurr.getLocation().rend(); it++)
	{
		int result = std::strncmp(
			(_request.getPathRequest() + "/").c_str(), 
			it->getUri().c_str(), 
			it->getUri().size()
		);
		if (result == 0)
			return &(*it); 
	}
	return (NULL);
}