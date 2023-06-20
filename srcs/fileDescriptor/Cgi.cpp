/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfrederi <lfrederi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/03 23:51:46 by lfrederi          #+#    #+#             */
/*   Updated: 2023/06/19 19:58:30 by lfrederi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cgi.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "WebServ.hpp"

#include <algorithm> // replace
#include <cstring> // toupper
#include <unistd.h> // pipe, read
#include <errno.h>
#include <sys/fcntl.h>
#include "Debugger.hpp"

/*****************
* CANNONICAL FORM
*****************/

Cgi::Cgi(void) : AFileDescriptor(), _socketInfo(NULL)
{}

Cgi::Cgi(Cgi const & copy)
	:	AFileDescriptor(copy), _socketInfo(copy._socketInfo)
{}

Cgi & Cgi::operator=(Cgi const & rhs)
{
	if (this != &rhs)
	{
		this->_fd = rhs._fd;
		this->_socketInfo = rhs._socketInfo;
	}

	return (*this);
}

Cgi::~Cgi()
{}
/******************************************************************************/

/**************
* CONSTRUCTORS
***************/
Cgi::Cgi(SocketFd & socketFd) : AFileDescriptor(), _socketInfo(&socketFd)
{}
/******************************************************************************/

/***********
* ACCESSORS
************/

int     Cgi::getReadFd() const
{
    return _fdRead;
}
/******************************************************************************/

/****************
* PUBLIC METHODS
****************/
int     Cgi::run()
{
    int pipeToCgi[2];
    int pipeFromCgi[2];
    int pid;

    if ((pid = initPipe(pipeToCgi, pipeFromCgi)) < 0)
        return (-1);

    if (pid == 0)
    {
        std::string cgiPath = _socketInfo->getServerInfo().getCgi().find("php")->second;
        char * cgiPathCopy = new char[cgiPath.size() + 1];
        strcpy(cgiPathCopy, cgiPath.c_str());

        std::string script = _socketInfo->getRequest().getFileName();
        char * scriptCopy = new char[script.size() + 1];
        strcpy(scriptCopy, script.c_str());

        char ** argv = new char* [3];
        argv[0] = cgiPathCopy;
        argv[1] = scriptCopy;
        argv[2] = NULL;

        char ** envCgi = mapCgiParams();
        
        //TODO: MAP FD
        close(pipeToCgi[1]);
        close(pipeFromCgi[0]);
        dup2(pipeToCgi[0], STDIN_FILENO);
        dup2(pipeFromCgi[1], STDOUT_FILENO);

        execve(argv[0], argv, envCgi);

        delete [] cgiPathCopy;
        delete [] scriptCopy;
        delete [] argv;
        close(pipeFromCgi[0]);
        close(pipeFromCgi[1]);
        close(pipeToCgi[0]);
        close(pipeToCgi[1]);
        exit(EXIT_FAILURE);
    }
    
    close(pipeFromCgi[1]);
    close(pipeToCgi[0]);
    _fdRead = pipeFromCgi[0];
    _fdWrite = pipeToCgi[1];
    close(_fdWrite);
    if (fcntl(_fdRead, F_SETFL, O_NONBLOCK) < 0)
	{
		std::cerr << "Fcntl error" << std::endl;
		return -1;
	}

    return (0);
}

/// @brief 
/// @param epoll 
void    Cgi::doOnRead(std::map<int, AFileDescriptor *> & mapFd)
{
    unsigned char buffer[BUFFER_SIZE];
    ssize_t n;
    size_t start;

    if ((n = read(_fdRead, buffer, BUFFER_SIZE)) > 0)
        _rawData.insert(_rawData.end(), buffer, buffer + n);
    if (n == 0)
    {
        std::string str(_rawData.begin(), _rawData.end());
        start = str.find("\r\n\r\n") + 4;
        str = str.substr(start);
        str = Response::cgiSimpleResponse(str);
        _socketInfo->responseCgi(str);
        close(_fdRead);
        mapFd.erase(_fdRead);
        WebServ::updateEpoll(_epollFd, _socketInfo->getFd(), EPOLLOUT, EPOLL_CTL_MOD);
    }
}

/// @brief 
void    Cgi::doOnWrite(std::map<int, AFileDescriptor *> & mapFd)
{
    (void) mapFd;
}

/// @brief 
/// @param mapFd 
/// @param event 
void	Cgi::doOnError(std::map<int, AFileDescriptor *> & mapFd, uint32_t event)
{
	std::cout << "SocketFd on error, event = " << event << std::endl;
    this->doOnRead(mapFd);
}
/******************************************************************************/

/*****************
* PRIVATE METHODS
*****************/

char **    Cgi::mapCgiParams()
{
    ServerConf const &  serverInfo = _socketInfo->getServerInfo();
    Request const & request = _socketInfo->getRequest();
    std::map<std::string, std::string> const & headers = request.getHeaders();
    char * cwd = get_current_dir_name();
    
    std::string tab[19] = {
        std::string("AUTH_TYPE=") + "",
        std::string("CONTENT_LENGTH=") + (headers.find("CONTENT_LENGTH") != headers.end() ? headers.find("CONTENT_LENGTH")->second : ""),
        std::string("CONTENT_TYPE=") + (headers.find("CONTENT_TYPE") != headers.end() ? headers.find("CONTENT_TYPE")->second : ""),
        std::string("GATEWAY_INTERFACE=CGI/1.1"),
        std::string("PATH_INFO=/"),
        std::string("PATH_TRANSLATED="),
        std::string("QUERY_STRING=") + request.getQueryParam(), // Set query string in request object
        std::string("REMOTE_ADDR="), // Set remote addr in request object
        std::string("REMOTE_HOST="),
        std::string("REMOTE_IDENT="),
        std::string("REMOTE_USER="),
        std::string("REQUEST_METHOD=") + request.getHttpMethod(),
        std::string("SCRIPT_NAME=") + request.getFileName(),
        std::string("PHP_SELF=") + request.getFileName(),
        std::string("SCRIPT_FILENAME=") + cwd + "/" + request.getFileName(),
        std::string("SERVER_NAME=") + serverInfo.getName(), // Get to header host ?
        std::string("SERVER_PORT=4242"), // TODO USE ITOA
        std::string("SERVER_PROTOCOL=") + request.getHttpVersion(),
        std::string("SERVER_SOFTWARE=webserv"),
        //std::string("REQUEST_URI=") + request.getPathRequest()
    };

    free(cwd);

    char ** env = new char* [headers.size() + 19 + 1];
    char * var = NULL;
    int i = 0;

    while (i < 19)
    {
        var = new char[tab[i].size() + 1];
        std::strcpy(var, tab[i].c_str());
        env[i] = var;
        i++;
    }

    std::map<std::string, std::string>::const_iterator it = headers.begin();
    for (it = headers.begin(); it != headers.end(); it++)
    {
        std::string headerName = it->first;
        std::replace(headerName.begin(), headerName.end(), '-', '_');
        std::for_each(headerName.begin(), headerName.end(), toupper);
        headerName = "HTTP_" + headerName + "=" + it->second;
        var = new char[headerName.size() + 1];
        std::strcpy(var, headerName.c_str());
        env[i] = var;
        i++;
    }
    env[i] = NULL;
    return (env);
}

int    Cgi::initPipe(int toCgi[2], int fromCgi[2])
{
    int pid;

    if (pipe(toCgi) < 0)
    {
        DEBUG_COUT(strerror(errno));
        return (-1);
    }
    if (pipe(fromCgi) < 0)
    {
        DEBUG_COUT(strerror(errno));
        close(toCgi[0]);
        close(toCgi[1]);
    }

    if ((pid = fork()) < 0)
    {
        DEBUG_COUT(strerror(errno));
        close(fromCgi[0]);
        close(fromCgi[1]);
        close(toCgi[0]);
        close(toCgi[1]);
        return (-1);
    }
    return (pid);
}

