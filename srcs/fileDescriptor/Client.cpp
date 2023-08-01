/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfrederi <lfrederi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/18 16:02:19 by lfrederi          #+#    #+#             */
/*   Updated: 2023/08/01 12:51:34 by lfrederi         ###   ########.fr       */
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
	  _startTime(0),
	  _server(NULL),
	  _serverConf(NULL),
	  _location(NULL),
	  _responseReady(false),
	  _callCgi(false),
	  _close(false)
{
}

Client::Client(Client const &copy)
	: AFileDescriptor(copy),
	  _startTime(copy._startTime),
	  _inputData(copy._inputData),
	  _outputData(copy._outputData),
	  _server(copy._server),
	  _serverConf(copy._serverConf),
	  _location(copy._location),
	  _correctPathRequest(copy._correctPathRequest),
	  _request(copy._request),
	  _cgi(copy._cgi),
	  _responseReady(copy._responseReady),
	  _callCgi(copy._callCgi),
	  _close(copy._close)
{
}

Client &Client::operator=(Client const &rhs)
{
	if (this != &rhs)
	{
		_fd = rhs._fd;
		_webServ = rhs._webServ;
		_startTime = rhs._startTime;
		_inputData = rhs._inputData;
		_outputData = rhs._outputData;
		_serverConf = rhs._serverConf;
		_location = rhs._location;
		_correctPathRequest = rhs._correctPathRequest;
		_request = rhs._request;
		_cgi = rhs._cgi;
		_responseReady = rhs._responseReady;
		_callCgi = rhs._callCgi;
		_close = rhs._close;
	}

	return (*this);
}

Client::~Client() {
	if (_cgi.getReadFd() != -1)
		close(_cgi.getReadFd());
	if (_cgi.getWriteFd() != -1) {
		close(_cgi.getWriteFd());
		std::cout << "Close write cgi des: " << _cgi.getWriteFd() << std::endl;
	}
}
/******************************************************************************/

/**************
 * CONSTRUCTORS
 ***************/

Client::Client(int fd, WebServ &webServ, Server const *server)
	: AFileDescriptor(fd, webServ),
	  _startTime(0),
	  _server(server),
	  _serverConf(NULL),
	  _location(NULL),
	  _responseReady(false),
	  _callCgi(false),
	  _close(false)
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

ServerConf const *Client::getServerConf() const
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
	if (_close)
		return _webServ->clearFd(_fd);

	if (_startTime == 0) {
		_startTime = TimeUtils::getTimeOfDayMs();
		_webServ->addClient(this);
	}

	char buffer[BUFFER_SIZE];
	ssize_t n;

	if ((n = recv(_fd, buffer, BUFFER_SIZE, 0)) > 0)
		_inputData.insert(_inputData.end(), buffer, buffer + n);

	if (n < 0) // Try to read next time fd is NON_BLOCK and we can't check errno
		return;
	else if (n == 0) // Client close connection
		return _webServ->clearFd(_fd);
	else {
		try {
			handleRequest();
		} catch (RequestUncomplete &exception) {
			return;
		} catch (std::exception const &exception) {
			return handleException(exception);
		}
		DEBUG_COUT(_request.getHttpMethod() + " " << _request.getPathRequest());
		_webServ->updateEpoll(_fd, EPOLLOUT, EPOLL_CTL_MOD);
	}
}

/**
 * @brief Try to write response in fd if is ready otherwise create response
 * @param webServ Reference to webServ
 */
void Client::doOnWrite()
{
	if (_responseReady) {
		send(_fd, &(*_outputData.begin()), _outputData.size(), 0);
		return prepareToNextRequest();
	}

	try {
		if (_callCgi)
			return handleScript(_correctPathRequest);
		throw RequestError(METHOD_NOT_ALLOWED, "Should implement GET POST DELETE");
		/*if (method == "GET")
			return Response::getResponse(autoindex);
		if (method == "POST")
			return Response::postResponse(uploadDir);
		if (method == "DELETE")
			return Response::deleteResponse();*/
	} catch (std::exception &exception) {
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
	DEBUG_COUT("Client on error: " + StringUtils::intToString(event));
	_webServ->clearFd(_fd);
}

void Client::fillRawData(std::vector<unsigned char> const &data)
{
	_outputData.assign(data.begin(), data.end());
}

void Client::readyToRespond()
{
	_responseReady = true;
	_webServ->updateEpoll(_fd, EPOLLOUT, EPOLL_CTL_MOD);
}

void Client::handleException(std::exception const &exception)
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
	if ((TimeUtils::getTimeOfDayMs() - _startTime) > TIMEOUT) {

		if (_cgi.getReadFd() != -1) {
			_webServ->updateEpoll(_cgi.getReadFd(), 0, EPOLL_CTL_DEL);
			_webServ->removeFd(_cgi.getReadFd());
			close(_cgi.getReadFd());
			_cgi.setReadFd(-1);
		}

		if (_cgi.getWriteFd() != -1) {
			_webServ->updateEpoll(_cgi.getWriteFd(), 0, EPOLL_CTL_DEL);
			_webServ->removeFd(_cgi.getWriteFd());
			close(_cgi.getWriteFd());
			_cgi.setWriteFd(-1);
		}
	}
	return (result);
}

void	Client::closeClient()
{
	_close = true;
}
/******************************************************************************/

/*****************
 * PRIVATE METHODS
 *****************/

/**
 * @brief 
 */
void Client::handleRequest() {
	if (_request.getHttpMethod().empty())
		_request.handleRequestLine(_inputData);

	if (_request.getHeaders().empty()) {
		_request.handleHeaders(_inputData);

		getCorrectServerConf();
		getCorrectLocationBlock();
		getCorrectPathRequest();
	}

	if (_request.hasMessageBody()) {
		if (!_request.isEncoded() && _request.getBodySize() > _location->getClientBodySize())
			throw RequestError(PAYLOAD_TOO_LARGE, "Body size too large");
		if (_callCgi)
			_request.handleMessageBody(_inputData);
		else
			throw RequestError(INTERNAL_SERVER_ERROR, "Upload not implemented yet");
	}
}


/**
 * @brief 
 * @return 
 */
void	Client::getCorrectServerConf() {
	std::vector<ServerConf>::const_iterator it = _server->getServerConfs().begin();

	for (; it != _server->getServerConfs().end(); it++) {
		std::vector<std::string> serversName = it->getName();
		std::vector<std::string>::iterator its = serversName.begin();

		for (; its != serversName.end(); its++) {
			std::string cmp = *its + ":" + StringUtils::intToString(it->getPort());

			if (_request.getHeaders().find("Host")->second == cmp) {
				_serverConf = &(*it);
				return ;
			}
		}
	}
	_serverConf = (&(*(_server->getServerConfs().begin())));
}


/**
 * @brief 
 * @param fullPath 
 */
void Client::handleScript(std::string const &fullPath) {
	_cgi = Cgi(*_webServ, *this, fullPath);

	if (_cgi.run() < 0)
		return Response::errorResponse(INTERNAL_SERVER_ERROR, *this);

	_webServ->addFd(_cgi.getWriteFd(), &_cgi);
	_webServ->updateEpoll(_cgi.getWriteFd(), EPOLLOUT, EPOLL_CTL_ADD);

	_webServ->addFd(_cgi.getReadFd(), &_cgi);
	_webServ->updateEpoll(_cgi.getReadFd(), 0, EPOLL_CTL_ADD);

	_webServ->updateEpoll(_fd, 0, EPOLL_CTL_MOD);
}

/**
 * @brief 
 */
void Client::getCorrectLocationBlock() {
	std::vector<Location>::const_reverse_iterator it = _serverConf->getLocation().rbegin();

	for (; it != _serverConf->getLocation().rend(); it++) {
		int result = std::strncmp((_request.getPathRequest() + "/").c_str(),
								  it->getUri().c_str(), it->getUri().size());

		if (result == 0) {
			_location = &(*it);
			return;
		}
	}
	throw RequestError(INTERNAL_SERVER_ERROR, "Could not find config for this request");
}

/**
 * @brief
 * @return
 */
void	Client::getCorrectPathRequest() {
	std::string request = _request.getPathRequest().substr(_location->getUri().size() - 1);
	if (request == "")
		request = "/";

	std::string fullPath = _location->getLocRoot() + request;
	std::string const &method = _request.getHttpMethod();

	std::vector<std::string> methods = _location->getAllowMethod();

	if (!methods.empty() && std::find(methods.begin(), methods.end(), method) == methods.end())
		throw RequestError(METHOD_NOT_ALLOWED, "Method " + method + "is not allowed");

	if (method == "GET" && request == "/")
		fullPath = searchIndexFile(fullPath, _location->getIndex(), _location->getAutoindex());

	_correctPathRequest = fullPath;
	size_t point = _correctPathRequest.rfind(".");
	if (point != std::string::npos && _correctPathRequest.substr(point + 1) == "php")
		_callCgi = true;
}


/**
 * @brief 
 * @param path 
 * @param indexs 
 * @param autoindex 
 * @return 
 */
std::string Client::searchIndexFile(std::string path, std::vector<std::string> const &indexs, bool autoindex) {
	std::vector<std::string>::const_iterator it = indexs.begin();

	for (; it != indexs.end(); it++) {
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


/**
 * @brief 
 */
void	Client::prepareToNextRequest() {
	_webServ->updateEpoll(_fd, EPOLLIN, EPOLL_CTL_MOD);
	_webServ->removeClient(_fd);

	_responseReady = false;
	_request = Request();
	_correctPathRequest = "";
	_cgi = Cgi();
	_callCgi = false;
	_startTime = 0;
	_serverConf = NULL;
	_location = NULL;
	_outputData.clear();
}
