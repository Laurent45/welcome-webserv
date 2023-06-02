/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SocketFd.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfrederi <lfrederi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/18 16:02:19 by lfrederi          #+#    #+#             */
/*   Updated: 2023/06/02 21:35:09 by lfrederi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "SocketFd.hpp"
#include "Debugger.hpp"

#include <cstddef>
#include <cstring>
#include <iostream>
#include <unistd.h> // read
#include <cstdio> // perror
#include <fstream>
#include <sys/socket.h> // recv

SocketFd::SocketFd(void) : AFileDescriptor()
{}

SocketFd::SocketFd(SocketFd const & copy)
	:	AFileDescriptor(copy)
{}

SocketFd & SocketFd::operator=(SocketFd const & rhs)
{
	if (this != &rhs)
	{
		this->_fd = rhs._fd;
		this->_rawData = rhs._rawData;
		this->_open = rhs._open;
	}

	return (*this);

}

SocketFd::~SocketFd()
{
	std::cout << "SocketFd destructor()" << std::endl;
}

SocketFd::SocketFd(int fd)
	:	AFileDescriptor(fd)
{}


Request const &	SocketFd::getRequest() const
{
	return (this->_request);
}

int		SocketFd::doOnRead()
{
	char	buffer[BUFFER_SIZE];
	ssize_t	n;
	// size_t	posHeadersEnd;

	std::cout << "FD on read " << this->_fd << std::endl;

	while ((n = recv(this->_fd, buffer, BUFFER_SIZE - 1, 0)) > 0)
	{
		buffer[n] = '\0';
		this->_rawData.append(buffer);
	}
	
	// Socket connection close, a EOF was present
	if (n == 0)
		return (1);
	if (this->_rawData.find("\r\n\r\n"))
	{
		Request Pending_Request(this->_rawData);
		Pending_Request.logRequest(Pending_Request);
		if (Pending_Request.isRequestLineAccepted() == false)
		{
			Pending_Request.setStatusCode(400);
			Pending_Request.setPathRequest("index_bad_request.html");
			return (Request::requestComplete);
		}
		else
		{
			Pending_Request.fill_from_request_line();
			Pending_Request.setStatusCode(200);
			// Pending_Request.print_Pending_Request();
			this->_request = Pending_Request;
			return (Request::requestComplete);

		}
	}
	else
		return (Request::requestUncomplete);

	// posHeadersEnd = this->_rawData.find("\r\n\r\n");

	// // Try to fill hearders if empty
	// if (this->_request.getHeaders().empty())
	// {
	// 	if (posHeadersEnd == std::string::npos)
	// 		return (Request::requestUncomplete);
	// 	this->_request.fillHeaders(this->_rawData);
	// }

	// if (!this->_request.hasMessageBody())
	// {
	// 	this->_rawData.clear();
	// 	return (Request::requestComplete);
	// }

	// this->_request.fillMessageBody(this->_rawData);
	// if (this->_request.isMessageBodyTerminated())
	// 	return (Request::requestComplete);

	// return (Request::requestUncomplete);
}

char *fill_my_buffer(std::string file_name)
 {
    std::string processed_file_name = file_name;
    std::size_t question_mark_pos = processed_file_name.find("?");
    if (question_mark_pos != std::string::npos) {
        processed_file_name = processed_file_name.substr(0, question_mark_pos);
    }
	if (processed_file_name == "/")
		processed_file_name = "index.html";
	else if (processed_file_name[0] == '/' && processed_file_name.length() > 2)
	{
		processed_file_name = processed_file_name.substr(1, processed_file_name.length());

	}
	if (access(processed_file_name.c_str(), R_OK) == 0)
	{
		std::ifstream file(processed_file_name.c_str(), std::ios::in | std::ios::binary);
		if (!file)
		{
        	std::cerr << "Impossible d'ouvrir le fichier : " << processed_file_name << std::endl;
        	return NULL;
    	}
    	file.seekg(0, std::ios::end);
    	std::streampos file_size = file.tellg();
    	file.seekg(0, std::ios::beg);
		file_size += 1;
    	char* buffer = new char[file_size];
		file.read(buffer, file_size);
		buffer[file.gcount()] = '\0';
   		file.close();
	    return buffer;
	}
	else if (file_name.find("html"))
	{
	    std::ifstream file("index_bad_request.html", std::ios::in | std::ios::binary);
 		file.seekg(0, std::ios::end);
    	std::streampos file_size = file.tellg();
    	file.seekg(0, std::ios::beg);
		file_size += 1;
	    char* buffer = new char[file_size];
	    file.read(buffer, file_size);
		buffer[file.gcount()] = '\0';
	    file.close();
		return buffer;
	}
	else
		return (NULL);
}

int		SocketFd::doOnWrite()
{
	std::cout << "write" << std::endl << std::endl;
	char *buffer = NULL;
	if (strlen(this->_request.getPathRequest().c_str()) <= 2)
		buffer = fill_my_buffer("index.html");
	else
		buffer = fill_my_buffer(this->_request.getPathRequest().c_str());

	// std::ifstream file("index.html");
	// if (!file.good())
	// {
	// 	std::cerr << "Error while opening file" << std::endl;
	// 	return (1);
	// }
	// std::string result;
	// const char * response = "HTTP/1.1 200 OK\r\nContent-type: application/json\r\nContent-length: ";

	// result.append(response);
	// std::ifstream file(file_name);
	// file.seekg(0, file.end);
	// int size = file.tellg();
	// file.seekg(0, file.beg);

	// char * buffer = new char [size + 1];
	// file.read(buffer, size);
	// buffer[file.gcount()] = '\0';

	// // char integer[20];
	// // std::sprintf(integer, "%d", size);
	// // result.append(integer);
	// // result.append("\r\n\r\n");
	// // result.append(buffer);

	// // delete [] buffer;
	// file.close();
	write(this->_fd, buffer, strlen(buffer));

	return (0);
}
