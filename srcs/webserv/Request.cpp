/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eantoine <eantoine@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/18 18:21:33 by lfrederi          #+#    #+#             */
/*   Updated: 2023/08/29 13:41:56 by eantoine         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"
#include "StringUtils.hpp"
#include "HttpUtils.hpp"
#include "Exception.hpp"
#include <limits>

#include <string>
#include <algorithm> // all_of

/*****************
* CANNONICAL FORM
*****************/

Request::Request() : _hasMessageBody(false), _encode(false)
{}

Request::Request(Request const & copy)
	:	_httpMethod(copy._httpMethod),
		_pathRequest(copy._pathRequest),
		_httpVersion(copy._httpVersion),
		_headers(copy._headers),
		_messageBody(copy._messageBody),
		_hasMessageBody(copy._hasMessageBody),
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
		_hasMessageBody = rhs._hasMessageBody;
		_messageBody = rhs._messageBody;
		_encode = rhs._encode;
		_bodySize = rhs._bodySize;
		_upload = rhs._upload;
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

bool	Request::isEncoded() const
{
	return (_encode);
}

size_t		Request::getBodySize() const
{
	return (_bodySize);
}

Upload &	Request::getUpload() {
	return (_upload);
}
/******************************************************************************/

/****************
* PUBLIC METHODS
****************/


/**
 * @brief 
 * @param rawData 
 */
void	Request::handleRequestLine(std::vector<unsigned char> & rawData)
{
	std::vector<unsigned char>::iterator it;
	unsigned char src[] = {'\r', '\n'};
	std::cout <<rawData.size()<<"\n";
	if (rawData.size() > MAX_URI_LENGTH)
		throw RequestError(URI_TOO_LONG, "Request line is too long");

	it = std::search(rawData.begin(), rawData.end(), src, src + 2);
	if (it == rawData.end())
		throw RequestUncomplete();

	std::string requestLine(rawData.begin(), it);
	std::vector<std::string> vec = StringUtils::splitStringSingle(requestLine, " ");
	if (vec.size() != 3)
		throw RequestError(BAD_REQUEST, "Request line is invalid");
	_httpMethod = vec[0];
	_pathRequest = vec[1];
	_httpVersion = vec[2];

	if (_httpMethod != "GET" && _httpMethod != "POST" && _httpMethod != "DELETE")
		throw RequestError(METHOD_NOT_ALLOWED, "This server doesn't handle this method: " + _httpMethod);
	if (_httpVersion.compare("HTTP/1.1") != 0)
		throw RequestError(BAD_REQUEST, "The http version must be HTTP/1.1");
	if (_pathRequest[0] != '/')
		throw RequestError(BAD_REQUEST, "A path request must start with /");

	if (_httpMethod.compare("POST") == 0)
		_hasMessageBody = true;

	size_t query = _pathRequest.find("?");
	if (query != std::string::npos)
	{
		_queryParam = _pathRequest.substr(query + 1);
		_pathRequest = _pathRequest.substr(0, query);
	}
	rawData.erase(rawData.begin(), it + 2);
}

bool isNumber(const std::string& str) {
    char* end;
    std::strtol(str.c_str(), &end, 10);

    // Si end pointe vers la fin de la chaîne, alors la conversion a réussi
    return (*end == '\0');
}

/**
 * @brief 
 * @param rawData 
 */
void	Request::handleHeaders(std::vector<unsigned char> & rawData) {	

	std::vector<unsigned char>::iterator ite;
	unsigned char src[] = {'\r', '\n', '\r', '\n'};
	if (rawData.size() > CLIENT_HEADER_SIZE)
		throw RequestError(BAD_REQUEST, "Header size is too large");

	ite = std::search(rawData.begin(), rawData.end(), src, src + 4);
	if (ite == rawData.end())
		throw RequestUncomplete();

	std::string headers(rawData.begin(), ite);
	std::vector<std::string> vec = StringUtils::splitString(headers, "\r\n");
	std::vector<std::string>::iterator it = vec.begin();
	for (; it != vec.end(); it++) {
		size_t sep = (*it).find(":");
		if (sep == std::string::npos)
			continue;
		if (std::find_if((*it).begin(), (*it).begin() + sep, isblank) != (*it).begin()+sep)
			throw RequestError(BAD_REQUEST, "Invalid space before :");
		std::string key = (*it).substr(0, sep);
		if (!key.compare(""))
			throw RequestError(BAD_REQUEST, "Empty header directive");
		std::string value = StringUtils::trimWhitespaces((*it).substr(sep + 1));
		if (key == "Transfer-Encoding")
			_encode = true;
		if (key == "Content-Length"){
			if (!isNumber(value) || atol(value.c_str()) > std::numeric_limits<int>::max() || atol(value.c_str())< 0)
			{
				throw RequestError(BAD_REQUEST, "Content-Length value is wrong");	
			}
			if (_headers.find("Content-Length") != _headers.end())
				throw RequestError(BAD_REQUEST, "Only one Content-Length directive accepted");
		}
		_headers[key] = value;
	}

	if (_headers.find("Host") == _headers.end())
		throw RequestError(BAD_REQUEST, "Host header is mandatory");

	if (_httpMethod == "POST") {
		std::map<std::string, std::string>::iterator length = _headers.find("Content-Length");
		std::map<std::string, std::string>::iterator encod = _headers.find("Transfer-Encoding");
		if (length == _headers.end() && encod == _headers.end())
			throw RequestError(BAD_REQUEST, "Header about post body is missing");
		if (_encode == false) {
			long convert = atol(length->second.c_str());
			if (convert <= 0)
				throw RequestError(BAD_REQUEST, "Body size has an invalid value");
			_bodySize = convert;
		}
	}
	rawData.erase(rawData.begin(), ite + 4);
}


/**
 * @brief 
 * @param rawData 
 */
void	Request::handleMessageBody(std::vector<unsigned char> & rawData)
{
	if (_encode)
		throw RequestError(METHOD_NOT_ALLOWED, "Encoded message body is not implemented");
	
	std::vector<unsigned char>::iterator end = rawData.end();
	if (rawData.size() > _bodySize)
		end = rawData.begin() + (rawData.size() - _bodySize) + 1;

	_messageBody.insert(_messageBody.end(), rawData.begin(), end);
	_bodySize -= (&(*end) - &(*rawData.begin()));
	if (_bodySize > 0) {
		rawData.clear();
		throw RequestUncomplete();
	}
	rawData.erase(rawData.begin(), end);
}


void	Request::uploadFiles(std::vector<unsigned char> & rawData) {
	
	if (!_upload.isUploading()) {
		_upload.prepareUpload(rawData, _bodySize);
		if (rawData.empty())
			throw RequestUncomplete();
	}

	_upload.upload(rawData, _bodySize);
}


void	Request::searchBondary() {
	
	std::string bondary = "boundary=";
	std::map<std::string, std::string>::iterator it = _headers.find("Content-Type");

	if (it == _headers.end())
		throw RequestError(BAD_REQUEST, "Content-Type header missing for search boundary");

	std::string const & value = it->second;

	size_t pos =value.find(bondary);
	if (pos == std::string::npos)
		throw RequestError(BAD_REQUEST, "Boundary value is not set");

	_upload.setBondary(value.substr(pos + bondary.size()));
}
/******************************************************************************/

/****************
* FUNCTIONS
****************/

std::ostream    &operator<<(std::ostream & o, Request const & r)
{
	o << r.getHttpMethod() << " " << r.getPathRequest() << " " << r.getHttpVersion() << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = r.getHeaders().begin(); it != r.getHeaders().end(); it++)
		o << it->first << ": " << it->second << std::endl; 
	o << std::endl << r.getMessageBody() << std::endl;
	return o;
}
