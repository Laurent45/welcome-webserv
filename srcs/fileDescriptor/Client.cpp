/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfrederi <lfrederi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/18 16:02:19 by lfrederi          #+#    #+#             */
/*   Updated: 2023/07/23 18:42:57 by lfrederi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Debugger.hpp"
#include "StringUtils.hpp"
#include "Response.hpp"
#include "Exception.hpp"
#include "WebServ.hpp"
#include "FileUtils.hpp"

#include <cstddef>
#include <cstring> // strncmp
#include <sys/epoll.h>
#include <iostream>
#include <unistd.h> // read
#include <cstdio>	// perror
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
	if (_cgi.getPidChild() != -1)
		kill(_cgi.getPidChild(), SIGTERM);
}
/******************************************************************************/

/**************
 * CONSTRUCTORS
 ***************/

Client::Client(int fd, WebServ & webServ, std::vector<ServerConf> const &serverInfo)
	: 	AFileDescriptor(fd, webServ),
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
void	Client::doOnRead()
{
	char buffer[BUFFER_SIZE];
	ssize_t n;

	if ((n = recv(_fd, buffer, BUFFER_SIZE, 0)) > 0)
		_rawData.assign(buffer, buffer + n);

	if (n < 0) // Try to read next time fd is NON_BLOCK and we can't check errno
		return;
	else if (n == 0) // Client close connection
		_webServ->removeFd(_fd);
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
		catch (std::exception &exception)
		{
			handleException(exception);
			_webServ->updateEpoll(_fd, EPOLLOUT, EPOLL_CTL_MOD);
			return;
		}
		_serverInfoCurr = getCorrectServer();
		_webServ->updateEpoll(_fd, EPOLLOUT, EPOLL_CTL_MOD);
	}
}


/**
 * @brief Try to write response in fd if is ready otherwise create response
 * @param webServ Reference to webServ
 */
void	Client::doOnWrite()
{
	if (_responseReady)
	{
		send(_fd, &(_rawData[0]), _rawData.size(), 0);

		// Reset client field for next request
		_webServ->updateEpoll(_fd, EPOLLIN, EPOLL_CTL_MOD);
		_responseReady = false;
		_request = Request();
		if (_cgi.getPidChild() != -1)
		{
			_webServ->eraseFd(_cgi.getReadFd());
			_webServ->eraseFd(_cgi.getWriteFd());
			_cgi = Cgi();
		}
		return;
	}

	try
	{
		handleRequest(getLocationBlock());
		if (_responseReady)
			_webServ->updateEpoll(_fd, EPOLLOUT, EPOLL_CTL_MOD);
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
	_webServ->removeFd(_fd);
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
	//attention de bien prendre le root du bloc location (par defaut meme que serveur , mis a jour s'il existe dans le location bloc)
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
		delete []buffer;

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
		std::vector<std::string> serversName = it->getName();
		std::vector<std::string>::iterator its = serversName.begin();
		for (;its !=serversName.end();its++)
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
				int result = std::strncmp(
				(_request.getPathRequest() + "/").c_str(), 
				it2->getUri().c_str(), 
				it2->getUri().size());
				if (result == 0)
					return (*it); 
		}
	}
	return (*(_serverInfo.begin()));
}


void	Client::handleScript(std::string const & fullPath)
{
	_cgi = Cgi(*_webServ, *this, fullPath);

	if (_cgi.run() < 0)
	{
		errorResponse(INTERNAL_SERVER_ERROR);
		_webServ->updateEpoll(_fd, EPOLLOUT, EPOLL_CTL_MOD);
		return;
	}

	_webServ->addFd(_cgi.getWriteFd(), &_cgi);
	_webServ->updateEpoll(_cgi.getWriteFd(), EPOLLOUT, EPOLL_CTL_ADD);

	// Set NULL at AFileDescriptor in order to avoir double free if error occur
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
		errorResponse(error.getStatusCode());
	}
	catch (std::exception &exception)
	{
		errorResponse(INTERNAL_SERVER_ERROR);
	}
}


Location const *Client::getLocationBlock()
{
	std::vector<Location>::const_reverse_iterator it = _serverInfoCurr.getLocation().rbegin();

	for (; it != _serverInfoCurr.getLocation().rend(); it++)
	{
		int result = std::strncmp(
			(_request.getPathRequest() + "/").c_str(),
			it->getUri().c_str(),
			it->getUri().size());
		if (result == 0)
			return &(*it);
	}
	return (NULL);
}

void	Client::handleRequest(Location const * location)
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
		throw RequestError(METHOD_NOT_ALLOWED);

	if (method == "GET")
	{
		if (location)
		{
			if (std::strncmp(
					(request + "/").c_str(),
					location->getUri().c_str(),
					location->getUri().size()) == 0)
				fullPath = searchIndexFile(fullPath, location->getIndex(), location->getAutoindex());
		}
		else
		{
			if (request == "/")
				fullPath = searchIndexFile(fullPath, _serverInfoCurr.getIndex(), _serverInfoCurr.getAutoindex());
		}
	}

	size_t point = fullPath.rfind(".");
	if (point != std::string::npos && fullPath.substr(point + 1) == "php")
		return handleScript(fullPath);
	throw RequestError(METHOD_NOT_ALLOWED);
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
			throw RequestError(FORBIDDEN);
		else
			return path + *it;
	}

	// otherwise check if autoindex is on
	if (!autoindex)
		throw RequestError(FORBIDDEN);
	return path;
}

/*void Client::handlePostRequest(Location *location)
{
}

void Client::handleDeleteRequest(Location *location)
{
}*/