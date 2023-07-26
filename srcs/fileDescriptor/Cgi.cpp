/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eantoine <eantoine@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/03 23:51:46 by lfrederi          #+#    #+#             */
/*   Updated: 2023/07/23 23:00:48 by eantoine         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cgi.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Debugger.hpp"
#include "WebServ.hpp"
#include "Client.hpp"
#include "StringUtils.hpp"
#include "Exception.hpp"

#include <algorithm> // replace
#include <cstring>   // toupper
#include <unistd.h>  // pipe, read
#include <errno.h>
#include <sys/fcntl.h>

/*****************
 * CANNONICAL FORM
 *****************/

Cgi::Cgi(void)
    : AFileDescriptor(),
      _clientInfo(NULL),
      _fdRead(-1),
      _fdWrite(-1),
      _pidChild(-1)
{
}

Cgi::Cgi(Cgi const &copy)
    : AFileDescriptor(copy),
      _rawData(copy._rawData),
      _clientInfo(copy._clientInfo),
      _fullPath(copy._fullPath),
      _fdRead(copy._fdRead),
      _fdWrite(copy._fdWrite),
      _pidChild(copy._pidChild)
{
}

Cgi &Cgi::operator=(Cgi const &rhs)
{
    if (this != &rhs)
    {
        _fd = rhs._fd;
        _webServ = rhs._webServ;
        _rawData = rhs._rawData;
        _clientInfo = rhs._clientInfo;
        _fullPath = rhs._fullPath;
        _fdRead = rhs._fdRead;
        _fdWrite = rhs._fdWrite;
        _pidChild = rhs._pidChild;
    }

    return (*this);
}

Cgi::~Cgi()
{
    if (_fdRead != -1)
        close(_fdRead);
    if (_fdWrite != -1)
        close(_fdWrite);
}
/******************************************************************************/

/**************
 * CONSTRUCTORS
 ***************/
Cgi::Cgi(WebServ &webServ, Client &client, std::string const &fullPath)
    : AFileDescriptor(-1, webServ),
      _clientInfo(&client),
      _fullPath(fullPath),
      _fdRead(-1),
      _fdWrite(-1),
      _pidChild(-1)
{
}
/******************************************************************************/

/***********
 * ACCESSORS
 ************/

int Cgi::getReadFd() const
{
    return _fdRead;
}

int Cgi::getWriteFd() const
{
    return _fdWrite;
}

int Cgi::getPidChild() const
{
    return _pidChild;
}
/******************************************************************************/

/****************
 * PUBLIC METHODS
 ****************/

/**
 * @brief Run cgi script
 * @return
 */
int Cgi::run()
{
    int pipeToCgi[2];
    int pipeFromCgi[2];

    if ((_pidChild = initChildProcess(pipeToCgi, pipeFromCgi)) < 0)
        return (-1);

    if (_pidChild == 0)
        runChildProcess(pipeToCgi, pipeFromCgi);

    close(pipeFromCgi[1]);
    close(pipeToCgi[0]);
    _fdRead = pipeFromCgi[0];
    _fdWrite = pipeToCgi[1];

    if (fcntl(_fdRead, F_SETFL, O_NONBLOCK) < 0)
    {
        std::cerr << "Fcntl error" << std::endl;
        return (-1);
    }
    return (0);
}

/**
 * @brief Read data from cgi and construct response if EOF is reached
 * @param webServ
 */
void Cgi::doOnRead()
{
    unsigned char buffer[BUFFER_SIZE];
    ssize_t n;

    if ((n = read(_fdRead, buffer, BUFFER_SIZE)) > 0)
        _rawData.insert(_rawData.end(), buffer, buffer + n);
    // TODO: Always check the body size    

    if (n == 0)
    {
        try
        {
            processCgiResponse();
        }
        catch (const std::exception &e)
        {
           _clientInfo->handleException(e); 
        }

        _webServ->updateEpoll(_fdRead, 0, EPOLL_CTL_DEL);
		_webServ->removeFd(_fdRead);
        close(_fdRead);
        _fdRead = -1;

        _clientInfo->readyToRespond();
    }
}

/**
 * @brief Send data to CGI and handle fd
 * @param webServ
 */
void Cgi::doOnWrite()
{
    std::vector<unsigned char> body = _clientInfo->getRequest().getMessageBody();

    write(_fdWrite, &body[0], body.size());

    _webServ->updateEpoll(_fdWrite, 0, EPOLL_CTL_DEL);
	_webServ->removeFd(_fdWrite);
    close(_fdWrite);
    _fdWrite = -1;

    _webServ->updateEpoll(_fdRead, EPOLLIN, EPOLL_CTL_MOD);
}

/**
 * @brief
 * @param webServ
 * @param event
 */
void Cgi::doOnError(uint32_t event)
{
    std::cout << "Client on error, event = " << event << std::endl;
    _pidChild = -1;
    this->doOnRead();
}
/******************************************************************************/

/*****************
 * PRIVATE METHODS
 *****************/

int Cgi::initChildProcess(int toCgi[2], int fromCgi[2])
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

void Cgi::runChildProcess(int pipeToCgi[2], int pipeFromCgi[2])
{
    char *cgiPathCopy = NULL;
    char *scriptCopy = NULL;

    std::string cgiPath = _clientInfo->getServerInfo().getCgi().find("php")->second;
    cgiPathCopy = new char[cgiPath.size() + 1];
    strcpy(cgiPathCopy, cgiPath.c_str());

    std::string script = _clientInfo->getRequest().getFileName();
    scriptCopy = new char[script.size() + 1];
    strcpy(scriptCopy, script.c_str());

    char **argv = new char *[3];
    argv[0] = cgiPathCopy;
    argv[1] = scriptCopy;
    argv[2] = NULL;

    char **envCgi = mapCgiParams();

    close(pipeToCgi[1]);
    close(pipeFromCgi[0]);
    dup2(pipeToCgi[0], STDIN_FILENO);
    dup2(pipeFromCgi[1], STDOUT_FILENO);

    execve(argv[0], argv, envCgi);

    delete[] cgiPathCopy;
    delete[] scriptCopy;
    delete[] argv;
    for (int i = 0; envCgi[i]; i++)
        delete[] envCgi[i];
    delete[] envCgi;
    close(pipeToCgi[0]);
    close(pipeFromCgi[1]);
    _webServ->~WebServ();
    cgiPath.~basic_string();
    exit(EXIT_FAILURE);
}

char **Cgi::mapCgiParams()
{
    ServerConf const &serverInfo = _clientInfo->getServerInfo();
    Request const &request = _clientInfo->getRequest();
    std::map<std::string, std::string> const &headers = request.getHeaders();
    char *cwd = get_current_dir_name();

    std::string tab[19] = {
        std::string("AUTH_TYPE="),
        std::string("CONTENT_LENGTH=") + ((headers.find("Content-Length") != headers.end()) ? headers.find("Content-Length")->second : ""),
        std::string("CONTENT_TYPE=") + (headers.find("Content-Type") != headers.end() ? headers.find("Content-Type")->second : ""),
        std::string("GATEWAY_INTERFACE=CGI/1.1"),
        std::string("PATH_INFO=/"),
        std::string("PATH_TRANSLATED="),
        std::string("QUERY_STRING=") + request.getQueryParam(),
        std::string("REMOTE_ADDR="),
        std::string("REMOTE_HOST="),
        std::string("REMOTE_IDENT="),
        std::string("REQUEST_METHOD=") + request.getHttpMethod(),
        std::string("SCRIPT_NAME=") + "index.php",
        std::string("PHP_SELF=") + "index.php",
        std::string("SCRIPT_FILENAME=") + cwd + "/" + _fullPath,
        std::string("SERVER_NAME=") + serverInfo.getName()[0], // TODO: Set dynamically
        std::string("SERVER_PORT=") + StringUtils::intToString(serverInfo.getPort()),
        std::string("SERVER_PROTOCOL=") + request.getHttpVersion(),
        std::string("SERVER_SOFTWARE=webserv"),
        std::string("REQUEST_URI=") + request.getPathRequest()};

    free(cwd);

    char **env = new char *[headers.size() + 19 + 1];
    char *var = NULL;
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

void    Cgi::processCgiResponse()
{
	unsigned char src[] = {'\r', '\n', '\r', '\n'};
	std::vector<unsigned char>::iterator ite;

	ite = std::search(_rawData.begin(), _rawData.end(), src, src + 4);
	if (_rawData.empty() || ite == _rawData.end())
		throw RequestError(INTERNAL_SERVER_ERROR, "Failed to read data from cgi");

    std::string headers(_rawData.begin(), ite);
    std::vector<unsigned char> body(ite + 4, _rawData.end());

    Response::cgiResponse(_rawData, headers, body);

    _clientInfo->responseCgi(_rawData);
}