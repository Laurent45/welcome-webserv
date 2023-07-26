/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfrederi <lfrederi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/18 16:02:19 by lfrederi          #+#    #+#             */
/*   Updated: 2023/07/25 22:21:02 by lfrederi         ###   ########.fr       */
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
#include <csignal>		// kill

/*****************
 * CANNONICAL FORM
 *****************/

Client::Client(void)
	: AFileDescriptor(), _responseReady(false)
{
}

Client::Client(Client const &copy)
	: AFileDescriptor(copy),
	  _startTime(copy._startTime),
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
		_webServ = rhs._webServ;
		_startTime = rhs._startTime;
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
	if (_cgi.getReadFd() != -1)
		close(_cgi.getReadFd());
	if (_cgi.getWriteFd() != -1)
		close(_cgi.getWriteFd());		
	if (_cgi.getPidChild() != -1)
		kill(_cgi.getPidChild(), SIGTERM);;
}
/******************************************************************************/

/**************
 * CONSTRUCTORS
 ***************/

Client::Client(int fd, WebServ &webServ, std::vector<ServerConf> const &serverInfo)
	: AFileDescriptor(fd, webServ),
	  _startTime(0),
	  _serverInfo(serverInfo),
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
		_serverInfoCurr = getCorrectServer();
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
		_cgi = Cgi();
		_startTime = 0;
		return;
	}

	try
	{
		handleRequest(getLocationBlock());
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
/******************************************************************************/

/*****************
 * PRIVATE METHODS
 *****************/

/* void Client::getResponse()
{
	// TODO: Verifier avec le serverConf le path du fichier et son existence OU errorResponse(NOT_FOUND) and change / to index.html
	// attention de bien prendre le root du bloc location (par defaut meme que serveur , mis a jour s'il existe dans le location bloc)
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
		delete[] buffer;

		resp_t resp = {OK, body, _request.getExtension(), _rawData, true};
		Response::createResponse(resp);
		_responseReady = true;
	}
	else
	{
	}
} */

ServerConf Client::getCorrectServer()
{
	std::vector<ServerConf>::iterator it = _serverInfo.begin();
	for (; it != _serverInfo.end(); it++)
	{
		std::vector<std::string> serversName = it->getName();
		std::vector<std::string>::iterator its = serversName.begin();
		for (; its != serversName.end(); its++)
		{
			if (_request.getHeaders().find("Host")->second == *its + ":" + StringUtils::intToString(it->getPort()))
				return (*it);
		}
	}

	it = _serverInfo.begin();
	for (; it != _serverInfo.end(); it++)
	{
		std::vector<Location>::const_reverse_iterator it2 = it->getLocation().rbegin();
		for (; it2 != it->getLocation().rend(); it2++)
		{
			int result = std::strncmp((_request.getPathRequest() + "/").c_str(), 
				it2->getUri().c_str(), it2->getUri().size());
				if (result == 0)
					return (*it); 
		}
	}
	return (*(_serverInfo.begin()));
}

void Client::handleScript(std::string const &fullPath)
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


void Client::handleException(std::exception &exception)
{
	DEBUG_COUT(exception.what());
	try
	{
		RequestError error = dynamic_cast<RequestError &>(exception);
		Response::errorResponse(error.getStatusCode(), *this);
	}
	catch (std::exception &exception)
	{
		Response::errorResponse(INTERNAL_SERVER_ERROR, *this);
	}
}


Location const *Client::getLocationBlock()
{
	std::vector<Location>::const_reverse_iterator it = _serverInfoCurr.getLocation().rbegin();

	for (; it != _serverInfoCurr.getLocation().rend(); it++)
	{
		int result = std::strncmp((_request.getPathRequest() + "/").c_str(), 
									it->getUri().c_str(), it->getUri().size());
		if (result == 0)
			return &(*it);
	}
	return (NULL);
}


void Client::handleRequest(Location const *location)
{
	std::string path = _serverInfoCurr.getRoot();
	std::string request = _request.getPathRequest();
	std::string fullPath = path + request;
	std::string method = _request.getHttpMethod();

	if (location && location->getUri()[0] != '/')
		path = "";

	std::vector<std::string> methods;
	if (location)
		methods = location->getAllowMethod();

	if (!methods.empty() && std::find(methods.begin(), methods.end(), method) == methods.end())
		throw RequestError(METHOD_NOT_ALLOWED, "Method " + method + "is not allowed");

	size_t point = fullPath.rfind(".");
	if (point != std::string::npos && fullPath.substr(point + 1) == "php")
		return handleScript(fullPath);
	throw RequestError(METHOD_NOT_ALLOWED, "Should implement GET POST DELETE");
	/*if (method == "GET")
		return Response::getResponse(autoindex);
	if (method == "POST")
		return Response::postResponse(uploadDir);
	if (method == "DELETE")
		return Response::deleteResponse();*/
}


std::string Client::searchIndexFile(std::string path, std::vector<std::string> const &indexs, bool autoindex)
{
	std::vector<std::string>::const_iterator it = indexs.begin();
	for (; it != indexs.end(); it++)
	{
		if (!FileUtils::fileExists((path + "/" + *it).c_str()))
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

bool Client::timeoutReached()
{
	bool result = (TimeUtils::getTimeOfDayMs() - _startTime) > TIMEOUT;
	if (result)
	{
		// TODO: Set to -1
		if (_cgi.getReadFd() != -1)
		{
			_webServ->updateEpoll(_cgi.getReadFd(), 0, EPOLL_CTL_DEL);
			_webServ->removeFd(_cgi.getReadFd());
			close(_cgi.getReadFd());
		}
		if (_cgi.getWriteFd() != -1)
		{
			_webServ->updateEpoll(_cgi.getWriteFd(), 0, EPOLL_CTL_DEL);
			_webServ->removeFd(_cgi.getWriteFd());
			close(_cgi.getWriteFd());
		}
		if (_cgi.getPidChild() != -1)
			kill(_cgi.getPidChild(), SIGTERM);
	}
	return (result);
}