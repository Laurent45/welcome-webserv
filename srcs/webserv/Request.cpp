/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfrederi <lfrederi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/18 18:21:33 by lfrederi          #+#    #+#             */
/*   Updated: 2023/06/02 21:32:49 by lfrederi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"
#include <iostream>
#include <cstring>
#include <fstream>
#include <ctime>

int const Request::requestUncomplete = REQUEST_UNCOMPLETE;
int const Request::requestComplete = REQUEST_COMPLETE;

Request::Request()
{}

Request::Request(const std::string &buffer)
{
	this->status_code_so_far = 200;
	init_authorised_methods();
	parseRawRequest(buffer);
}

Request::Request(Request const & copy)
	:	_httpMethod(copy._httpMethod),
		_pathRequest(copy._pathRequest),
		_httpVersion(copy._httpVersion),
		_headers(copy._headers),
		_messageBody(copy._messageBody)
{
}

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

// Geters
std::string const & Request::getHttpMethod() const
{
	return (this->_httpMethod);
}

std::string const & Request::getPathRequest() const
{
	return (this->_pathRequest);
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
	if (this->_hasMessageBody == true)
		return (true);
	return (false);
}

// Seters
void	Request::setStatusCode(int code)
{
	this->status_code_so_far = code;
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

//methods
void	Request::init_authorised_methods(void)
{
	this->authorised_methods.push_back("GET");
	this->authorised_methods.push_back("POST");
	this->authorised_methods.push_back("DELETE");
}

// void Request::parseRequestLine(const std::string& request)
// {
//     size_t methodEnd = request.find(' ');
//     size_t pathEnd = request.find(' ', methodEnd + 1);


//     this->_httpMethod = request.substr(0, methodEnd);
//     this->_pathRequest = request.substr(methodEnd + 1, pathEnd - methodEnd - 1);
//     this->_httpVersion = request.substr(pathEnd + 1);
// }

void Request::parseRawRequest(const std::string& request)
{
    std::size_t endPos = request.find("\r\n");
    std::string firstLineStr = request.substr(0, endPos);
    std::size_t spacePos = firstLineStr.find(' ');
    std::size_t startPos = 0;
    while (spacePos != std::string::npos)
    {
        std::string token = firstLineStr.substr(startPos, spacePos - startPos);
        this->requestline_extracted.push_back(token);

        startPos = spacePos + 1;
        spacePos = firstLineStr.find(' ', startPos);
    }
    if (startPos < firstLineStr.length())
    {
        std::string token = firstLineStr.substr(startPos);
        this->requestline_extracted.push_back(token);
    }
    startPos = endPos + 2;  // Ignorer la séquence "\r\n"
    endPos = request.find("\r\n", startPos);
    while (endPos != std::string::npos)
    {
        std::string line = request.substr(startPos, endPos - startPos);
        std::size_t colonPos = line.find(": ");
        if (colonPos != std::string::npos)
        {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 2);  // Ignorer ": "

            this->_headers[key] = value;
        }
        startPos = endPos + 2;  // Ignorer la séquence "\r\n"
        endPos = request.find("\r\n", startPos);
    }
	if (!strcmp(this->requestline_extracted[0].c_str(), "POST") &&  startPos < request.length())
	{
		this->_hasMessageBody = true;
		std::string body = request.substr(startPos, request.length());
		this->_messageBody = body;
	}
	else
		this->_hasMessageBody = false;
}

void	Request::fill_from_request_line(void)
{
	this->_httpMethod = this->requestline_extracted[0];
	this->_pathRequest = this->requestline_extracted[1];
	this->_httpVersion = this->requestline_extracted[2];
}

bool Request::isRequestLineAccepted(void)
{
	bool is_method_valid = false;

	if (this->requestline_extracted.size() != 3)
		return (false);
	for (std::size_t i = 0; i < this->authorised_methods.size(); ++i)
	{
		if (this->authorised_methods[i] == this->requestline_extracted[0])
		{
			is_method_valid = true;
			break ;
		}
	}
	if (is_method_valid == false)
		return (false);
	if (this->requestline_extracted[2] != "HTTP/1.1")
		return (false);
	if (this->requestline_extracted[1][0] != '/')
		return (false);
	this->status_code_so_far = 200;
	return (true);
}

void Request::print_Pending_Request(void)
{
	std::cout << "METHOD [" << this->getHttpMethod() << "] PROTOCOL [" << this->getHttpVersion() << "] PATH [" << this->getPathRequest() << "]" << std::endl;
	std::cout << "HEADERS FROM REQUEST :" << std::endl;
	std::map<std::string, std::string>::const_iterator it;
    for (it = this->_headers.begin(); it !=  this->_headers.end(); ++it) {
        std::cout << "KEY: " << it->first << ", VALUE:[" << it->second << "]" << std::endl;
    }
	if (this->_hasMessageBody == true)
	{
		std::cout << "MESSAGEBODY SPOTTED => [" << this->_messageBody << "]" << std::endl;
	}
}

void Request::logRequest(const Request &request)
{
	(void)request;
    std::ofstream file("requests.log", std::ios::app);
    if (file.is_open()) {
        std::time_t now = std::time(NULL);
        char timestamp[100];
        std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

        file << timestamp << " " << this->requestline_extracted[0] << " wants to access " << this->requestline_extracted[1] << std::endl;
        file.close();
    } else {
        std::cerr << "Impossible d'ouvrir le fichier 'requests.log'." << std::endl;
    }
}