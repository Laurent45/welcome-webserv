/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfrederi <lfrederi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/18 18:21:33 by lfrederi          #+#    #+#             */
/*   Updated: 2023/07/03 21:37:00 by lfrederi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"
#include "StringUtils.hpp"
#include "HttpUtils.hpp"
#include "Exception.hpp"

#include <string>
#include <algorithm> // all_of

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
		_messageBody(copy._messageBody),
		_encode(copy._encode),
		_bodySize(copy._bodySize)
{}

Request & Request::operator=(Request const & rhs)
{
	if (this != &rhs)
	{
		_httpMethod = rhs._httpMethod;
		_pathRequest = rhs._pathRequest;
		_httpVersion = rhs._httpVersion;
		_headers = rhs._headers;
		_messageBody = rhs._messageBody;
		_encode = rhs._encode;
		_bodySize = rhs._bodySize;
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
	return (_httpMethod);
}

std::string const & Request::getPathRequest() const
{
	return (_pathRequest);
}

std::string const &	Request::getFileName() const
{
	return (_fileName);
}

std::string const &	Request::getExtension() const
{
	return (_extension);
}

std::string const &	Request::getQueryParam() const
{
	return (_queryParam);
}

std::string const & Request::getHttpVersion() const
{
	return (_httpVersion);
}

std::map<std::string, std::string> const & Request::getHeaders() const
{
	return (_headers);
}

std::vector<unsigned char> const & Request::getMessageBody() const
{
	return (_messageBody);
}

bool	Request::hasMessageBody() const
{
	return (_hasMessageBody);
}

void	Request::setHttpMethod(std::string const & httpMethod)
{
	_httpMethod = httpMethod;
}

void	Request::setPathRequest(std::string const & pathRequest)
{
	_pathRequest = pathRequest;
}

void	Request::setHttpVersion(std::string const & httpVersion)
{
	_httpVersion = httpVersion;
}

void	Request::setHeaders(std::map<std::string, std::string> const & headers)
{
	_headers = headers; 
}
/******************************************************************************/

/****************
* PUBLIC METHODS
****************/

/// @brief 
/// @param requestLine 
/// @return 
void	Request::handleRequestLine(std::string requestLine)
{
	std::vector<std::string> vec = StringUtils::splitString(requestLine, " ");
	if (vec.size() != 3)
		throw RequestError("Request line uncomplete");
	_httpMethod = vec[0];
	_pathRequest = vec[1];
	_httpVersion = vec[2];

	size_t lastSlash = _pathRequest.rfind("/");
	size_t query = _pathRequest.find("?");
	size_t extension;

	if (HttpUtils::isMethodAllowed(_httpMethod) == false)
		throw RequestError("Http method not handle in this server");
	if (_httpMethod.compare("POST") == 0)
		_hasMessageBody = true;
	if (_httpVersion.compare("HTTP/1.1") != 0)
		throw RequestError("Bad http version");
	if (_pathRequest[0] != '/')
		throw RequestError("Path request must start with /");
	if (query == std::string::npos)
		_fileName = _pathRequest.substr(lastSlash + 1);
	else
	{
		_fileName = _pathRequest.substr(lastSlash, query);
		_queryParam = _pathRequest.substr(query);
	}
	if ((extension = _fileName.rfind(".")) != std::string::npos)
		_extension = _fileName.substr(extension + 1);
}

/// @brief 
/// @param headers 
/// @return 
void	Request::handleHeaders(std::string headers)
{
	std::vector<std::string> vec = StringUtils::splitString(headers, "\r\n");
	std::vector<std::string>::iterator it = vec.begin();

	for (; it != vec.end(); it++)
	{
		size_t sep = (*it).find(":");
		if (sep == std::string::npos)
			continue;
		if (std::find_if((*it).begin(), (*it).begin() + sep, isblank) != (*it).begin()+sep)
			continue;
		std::string key = (*it).substr(0, sep);
		std::string value = StringUtils::trimWhitespaces((*it).substr(sep + 1));
		if (key == "Content-Length")
			_encode = false;
		if (key == "Transfer-Encoding")
			_encode = true;
		_headers[key] = value;
	}
	if (_headers.find("Host") == _headers.end())
		throw RequestError("Missing Host header");
	if (_httpMethod == "POST")
	{
		std::map<std::string, std::string>::iterator length = _headers.find("Content-Length");
		std::map<std::string, std::string>::iterator encod = _headers.find("Transfer-Encoding");
		if (length == _headers.end() && encod == _headers.end())
			throw RequestError("Missing headers about message body");
		if (_encode == false)
			_bodySize = atoi(length->second.c_str()); // TODO: Check if size is an integer
	}
		
}

/// @brief 
/// @param messageBody 
/// @return 
void	Request::handleMessageBody(std::vector<unsigned char> & rawData)
{
	if (_encode == false)
	{
		_messageBody.insert(_messageBody.end(), rawData.begin(), rawData.end());
		_bodySize -= rawData.size();
		if (_bodySize > 0)
			throw RequestUncomplete();
	}
}

std::ostream    &operator<<(std::ostream & o, Request const & r)
{
	o << r.getHttpMethod() << " " << r.getPathRequest() << " " << r.getHttpVersion() << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = r.getHeaders().begin(); it != r.getHeaders().end(); it++)
		o << it->first << ": " << it->second << std::endl; 
	o << std::endl << r.getMessageBody() << std::endl;
	return o;
}
