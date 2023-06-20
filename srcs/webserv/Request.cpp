/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfrederi <lfrederi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/18 18:21:33 by lfrederi          #+#    #+#             */
/*   Updated: 2023/06/16 15:07:59 by lfrederi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"
#include <string>
#include <algorithm> // all_of
#include "Syntax.hpp"
#include <iostream> // REMOVE

int const Request::requestUncomplete = REQUEST_UNCOMPLETE;
int const Request::requestComplete = REQUEST_COMPLETE;

/*****************
* CANNONICAL FORM
*****************/

Request::Request()
{}

Request::Request(Request const & copy)
	:	_httpMethod(copy._httpMethod),
		_pathRequest(copy._pathRequest),
		_httpVersion(copy._httpVersion),
		_headers(copy._headers),
		_messageBody(copy._messageBody)
{}

Request & Request::operator=(Request const & rhs)
{
	if (this != &rhs)
	{
		this->_httpMethod = rhs._httpMethod;
		this->_pathRequest = rhs._pathRequest;
		this->_httpVersion = rhs._httpVersion;
		this->_headers = rhs._headers;
		this->_messageBody = rhs._messageBody;
	}

	return (*this);
}
	
Request::~Request()
{}
/******************************************************************************/

/***********
* ACCESSORS
************/

std::string const & Request::getHttpMethod() const
{
	return (this->_httpMethod);
}

std::string const & Request::getPathRequest() const
{
	return (this->_pathRequest);
}

std::string const &	Request::getFileName() const
{
	return (this->_fileName);
}

std::string const &	Request::getExtension() const
{
	return (this->_extension);
}

std::string const &	Request::getQueryParam() const
{
	return (this->_queryParam);
}

std::string const & Request::getHttpVersion() const
{
	return (this->_httpVersion);
}

std::map<std::string, std::string> const & Request::getHeaders() const
{
	return (this->_headers);
}

std::string const & Request::getMessageBody() const
{
	return (this->_messageBody);
}

bool	Request::hasMessageBody() const
{
	return (false);
}

void	Request::setHttpMethod(std::string const & httpMethod)
{
	this->_httpMethod = httpMethod;
}

void	Request::setPathRequest(std::string const & pathRequest)
{
	this->_pathRequest = pathRequest;
}

void	Request::setHttpVersion(std::string const & httpVersion)
{
	this->_httpVersion = httpVersion;
}

void	Request::setHeaders(std::map<std::string, std::string> const & headers)
{
	this->_headers = headers; 
}

void	Request::setMessageBody(std::string const & messageBody)
{
	this->_messageBody = messageBody;
}
/******************************************************************************/

/****************
* PUBLIC METHODS
****************/

/// @brief 
/// @param requestLine 
/// @return 
bool	Request::handleRequestLine(std::string requestLine)
{
	std::vector<std::string> vec = Syntax::splitString(requestLine, " ");
	if (vec.size() != 3)
		return false;
	_httpMethod = vec[0];
	_pathRequest = vec[1];
	_httpVersion = vec[2];

	size_t lastSlash = _pathRequest.rfind("/");
	size_t query = _pathRequest.find("?");
	size_t extension;
	// TODO: Add DELETE POST
	if (_httpMethod.compare("GET") != 0)
		return false;
	if (_httpVersion.compare("HTTP/1.1") != 0)
		return false;
	if (_pathRequest[0] != '/')
		return false;
	// TODO: Add index to config file
	if (_pathRequest.size() == 1)
		_fileName = "index.html";
	else if (query == std::string::npos)
		_fileName = _pathRequest.substr(lastSlash + 1);
	else
	{
		_fileName = _pathRequest.substr(lastSlash, query);
		_queryParam = _pathRequest.substr(query);
	}
	if ((extension = _fileName.rfind(".")) != std::string::npos)
		_extension = _fileName.substr(extension + 1);
	return true;	
}

/// @brief 
/// @param headers 
/// @return 
bool	Request::handleHeaders(std::string headers)
{
	std::vector<std::string> vec = Syntax::splitString(headers, "\r\n");
	std::vector<std::string>::iterator it = vec.begin();

	for (; it != vec.end(); it++)
	{
		size_t sep = (*it).find(":");
		if (sep == std::string::npos)
			continue;
		if (std::find_if((*it).begin(), (*it).begin() + sep, isblank) != (*it).begin()+sep)
			continue;
		std::string key = (*it).substr(0, sep);
		std::string value = Syntax::trimWhitespaces((*it).substr(sep + 1));

		_headers[key] = value;
	}
	if (_headers.find("Host") == _headers.end())
		return false;
	return true;
}

/// @brief 
/// @param messageBody 
/// @return 
bool	Request::handleMessageBody(std::vector<unsigned char> messageBody)
{
	(void) messageBody;
	return true;
}

