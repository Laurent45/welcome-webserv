/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfrederi <lfrederi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/18 16:02:19 by lfrederi          #+#    #+#             */
/*   Updated: 2023/07/28 11:43:53 by lfrederi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Debugger.hpp"
#include "StringUtils.hpp"
#include "Response.hpp"
#include "Exception.hpp"
#include "WebServ.hpp"
#include "FileUtils.hpp"
#include "TimeUtils.hpp"

#include <cstddef>
#include <cstring> // strncmp
#include <sys/epoll.h>
#include <iostream>
#include <unistd.h> // read
#include <fstream>
#include <sys/socket.h> // recv
#include <algorithm>	// search

/*****************
 * CANNONICAL FORM
 *****************/

Client::Client(void)
	: AFileDescriptor(), 
	  _server(NULL),
	  _serverConf(NULL),
	  _location(NULL),
	  _responseReady(false)
{
}

Client::Client(Client const &copy)
	: AFileDescriptor(copy),
	  _startTime(copy._startTime),
	  _rawData(copy._rawData),
	  _server(copy._server),
	  _serverConf(copy._serverConf),
	  _request(copy._request),
	  _correctPathRequest(copy._correctPathRequest),
	  _responseReady(copy._responseReady),
	  _cgi(copy._cgi)
{
}

Client &Client::operator=(Client const &rhs)
{
	if (this != &rhs)
	{
		_fd = rhs._fd;
		_webServ = rhs._webServ;
		_startTime = rhs._startTime;
		_rawData = rhs._rawData;
		_serverConf = rhs._serverConf;
		_request = rhs._request;
		_correctPathRequest = rhs._correctPathRequest;
		_responseReady = rhs._responseReady;
		_cgi = rhs._cgi;
	}

	return (*this);
}

Client::~Client()
{
	if (_cgi.getReadFd() != -1)
		close(_cgi.getReadFd());
	if (_cgi.getWriteFd() != -1)
		close(_cgi.getWriteFd());		
}
/******************************************************************************/

/**************
 * CONSTRUCTORS
 ***************/

Client::Client(int fd, WebServ &webServ, Server const * server) 
	: AFileDescriptor(fd, webServ),
	  _startTime(0),
	  _server(server),
	  _serverConf(NULL),
	  _location(NULL),
	  _responseReady(false)
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

ServerConf const * Client::getServerConf() const
{
	return (this->_serverConf);
}
/******************************************************************************/

/****************
 * PUBLIC METHODS
 ****************/

/**
 * @brief Read data in fd and try to construct the request. While request is
 * complete retrive correct server information and update fd to EPOLLOUT
 */
void Client::doOnRead()
{
	if (_startTime == 0)
	{
		_startTime = TimeUtils::getTimeOfDayMs();
		_webServ->addClient(this);
	}

	char buffer[BUFFER_SIZE];
	ssize_t n;

	if ((n = recv(_fd, buffer, BUFFER_SIZE, 0)) > 0)
		_rawData.assign(buffer, buffer + n);

	if (n < 0) // Try to read next time fd is NON_BLOCK and we can't check errno
		return;
	else if (n == 0) // Client close connection
		_webServ->clearFd(_fd);
	else
	{
		try
		{
			if (_request.getHttpMethod().empty())
				_request.handleRequestLine(_rawData);
			if (_request.getHeaders().empty())
				_request.handleHeaders(_rawData);
			if (_request.hasMessageBody())
				_request.handleMessageBody(_rawData);
		}
		catch (RequestUncomplete &exception)
		{
			return;
		}
		catch (std::exception const &exception)
		{
			handleException(exception);
			return;
		}
		DEBUG_COUT(_request.getHttpMethod() + " " << _request.getPathRequest());
		_serverConf = getCorrectServerConf();
		_webServ->updateEpoll(_fd, EPOLLOUT, EPOLL_CTL_MOD);
	}
}

/**
 * @brief Try to write response in fd if is ready otherwise create response
 * @param webServ Reference to webServ
 */
void Client::doOnWrite()
{
	if (_responseReady)
	{
		send(_fd, &(_rawData[0]), _rawData.size(), 0);

		// Reset client field for next request
		_webServ->updateEpoll(_fd, EPOLLIN, EPOLL_CTL_MOD);
		_webServ->removeClient(_fd);
		_responseReady = false;
		_request = Request();
		_correctPathRequest = "";
		_cgi = Cgi();
		_startTime = 0;
		_serverConf = NULL;
		_location = NULL;
		return;
	}

	try
	{
		getCorrectLocationBlock();
		getCorrectPathRequest();
		size_t point = _correctPathRequest.rfind(".");
		if (point != std::string::npos && _correctPathRequest.substr(point + 1) == "php")
			return handleScript(_correctPathRequest);
		throw RequestError(METHOD_NOT_ALLOWED, "Should implement GET POST DELETE");
		/*if (method == "GET")
			return Response::getResponse(autoindex);
		if (method == "POST")
			return Response::postResponse(uploadDir);
		if (method == "DELETE")
			return Response::deleteResponse();*/
	}
	catch (std::exception &exception)
	{
		handleException(exception);
	}
}

/**
 * @brief
 * @param webServ
 * @param event
 */
void Client::doOnError(uint32_t event)
{
	std::cout << "Client on error, event = " << event << std::endl;
	_webServ->clearFd(_fd);
}

/**
 * @brief
 * @param response
 */
void Client::responseCgi(std::vector<unsigned char> const & cgiRawData)
{
	_responseReady = true;
	_rawData.assign(cgiRawData.begin(), cgiRawData.end());
}


void Client::fillRawData(std::vector<unsigned char> const &data)
{
	_rawData.assign(data.begin(), data.end());
}

void Client::readyToRespond()
{
	_responseReady = true;
	_webServ->updateEpoll(_fd, EPOLLOUT, EPOLL_CTL_MOD);
}


void	Client::handleException(std::exception const & exception)
{
	DEBUG_COUT(exception.what());
	try
	{
		RequestError error = dynamic_cast<RequestError const &>(exception);
		Response::errorResponse(error.getStatusCode(), *this);
	}
	catch (std::exception &exception)
	{
		Response::errorResponse(INTERNAL_SERVER_ERROR, *this);
	}
}


bool Client::timeoutReached()
{
	bool result = (TimeUtils::getTimeOfDayMs() - _startTime) > TIMEOUT;
	if (result)
	{
		if (_cgi.getReadFd() != -1)
		{
			_webServ->updateEpoll(_cgi.getReadFd(), 0, EPOLL_CTL_DEL);
			_webServ->removeFd(_cgi.getReadFd());
			close(_cgi.getReadFd());
			_cgi.setReadFd(-1);
		}
		if (_cgi.getWriteFd() != -1)
		{
			_webServ->updateEpoll(_cgi.getWriteFd(), 0, EPOLL_CTL_DEL);
			_webServ->removeFd(_cgi.getWriteFd());
			close(_cgi.getWriteFd());
			_cgi.setWriteFd(-1);
		}
	}
	return (result);
}
/******************************************************************************/

/*****************
 * PRIVATE METHODS
 *****************/

ServerConf const *		 Client::getCorrectServerConf()
{
	std::vector<ServerConf>::const_iterator it = _server->getServerConfs().begin();
	for (; it != _server->getServerConfs().end(); it++)
	{
		std::vector<std::string> serversName = it->getName();
		std::vector<std::string>::iterator its = serversName.begin();
		for (; its != serversName.end(); its++)
		{
			if (_request.getHeaders().find("Host")->second == *its + ":" + StringUtils::intToString(it->getPort()))
				return (&(*it));
		}
	}

	it = _server->getServerConfs().begin();
	for (; it != _server->getServerConfs().end(); it++)
	{
		std::vector<Location>::const_reverse_iterator it2 = it->getLocation().rbegin();
		for (; it2 != it->getLocation().rend(); it2++)
		{
			int result = std::strncmp((_request.getPathRequest() + "/").c_str(), 
				it2->getUri().c_str(), it2->getUri().size());
				if (result == 0)
					return (&(*it)); 
		}
	}
	return (&(*(_server->getServerConfs().begin())));
}


void		Client::handleScript(std::string const & fullPath)
{
	_cgi = Cgi(*_webServ, *this, fullPath);

	if (_cgi.run() < 0)
		return Response::errorResponse(INTERNAL_SERVER_ERROR, *this);

	_webServ->addFd(_cgi.getWriteFd(), &_cgi);
	_webServ->updateEpoll(_cgi.getWriteFd(), EPOLLOUT, EPOLL_CTL_ADD);

	_webServ->addFd(_cgi.getReadFd(), &_cgi);
	_webServ->updateEpoll(_cgi.getReadFd(), 0, EPOLL_CTL_ADD);

	_webServ->updateEpoll(_fd, 0, EPOLL_CTL_MOD);
}


void		Client::getCorrectLocationBlock()
{
	std::vector<Location>::const_reverse_iterator it = _serverConf->getLocation().rbegin();

	for (; it != _serverConf->getLocation().rend(); it++)
	{
		int result = std::strncmp((_request.getPathRequest() + "/").c_str(), 
									it->getUri().c_str(), it->getUri().size());
		if (result == 0)
		{
			_location = &(*it);
			return ;
		}
	}
	throw RequestError(INTERNAL_SERVER_ERROR, "Could not find config for this request");
}


/**
 * @brief 
 * @return 
 */
void		Client::getCorrectPathRequest()
{
	std::string request = _request.getPathRequest().substr(_location->getUri().size() - 1);
	if (request == "")
		request = "/";
	std::string fullPath = _location->getLocRoot() + request;
	std::string const & method = _request.getHttpMethod();

	std::vector<std::string> methods = _location->getAllowMethod();

	if (!methods.empty() && std::find(methods.begin(), methods.end(), method) == methods.end())
		throw RequestError(METHOD_NOT_ALLOWED, "Method " + method + "is not allowed");

	if (method == "GET" && request == "/")
		fullPath = searchIndexFile(fullPath, _location->getIndex(), _location->getAutoindex());

	_correctPathRequest = fullPath;
}


std::string Client::searchIndexFile(std::string path, std::vector<std::string> const &indexs, bool autoindex)
{
	std::vector<std::string>::const_iterator it = indexs.begin();
	for (; it != indexs.end(); it++)
	{
		if (!FileUtils::fileExists((path + *it).c_str()))
			continue;
		if (!FileUtils::fileRead((path + *it).c_str()))
			throw RequestError(FORBIDDEN, "File cannot read");
		else
			return path + *it;
	}

	// otherwise check if autoindex is on
	if (!autoindex)
		throw RequestError(FORBIDDEN, "Autoindex is off");
	return path;
}
