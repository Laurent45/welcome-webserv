/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eantoine <eantoine@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/03 19:19:11 by lfrederi          #+#    #+#             */
/*   Updated: 2023/07/31 22:19:38 by eantoine         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"
#include "StringUtils.hpp"
#include "TimeUtils.hpp"
#include "Client.hpp"
#include "Exception.hpp"
#include "FileUtils.hpp"

#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm> // search
#include <cstring> // strncmp
#include <cstdlib> // atoi

void	ft_bzero(void *s, size_t n)
{
	size_t	i;
	char	*str;

	if (n == 0)
		return ;
	i = 0;
	str = (char *)s;
	while (i < n)
		str[i++] = 0;
	return ;
}


void    Response::cgiResponse(std::vector<unsigned char> & clientRawData,
                              std::string headers, std::vector<unsigned char> & body)
{
    int statusCode = 200;

    std::vector<std::string> lines = StringUtils::splitString(headers, "\r\n");
    std::string statusLine = lines[0];
    std::string cmp = "Status: ";
    if (std::strncmp(cmp.c_str(), statusLine.c_str(), cmp.size()) == 0)
    {
        statusCode = std::atoi(statusLine.substr(cmp.size() - 1, 4).c_str());
        statusCode =  HttpUtils::getResponseStatus(static_cast<status_code_t>(statusCode))
                        .first;
        if (statusCode >= 400)
            throw RequestError(static_cast<status_code_t>(statusCode), "Cgi error response status");
    }
    
    std::string common = Response::commonResponse(static_cast<status_code_t>(statusCode));
    for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); it++)
    {
        common.insert(common.end(), it->begin(), it->end());
        common.push_back('\r');
        common.push_back('\n');
    }
    std::string size = "Content-Length: " + StringUtils::intToString(body.size()) + "\r\n\r\n";
    common.insert(common.end(), size.begin(), size.end());

    std::vector<unsigned char> response;
    response.assign(common.begin(), common.end());
    response.insert(response.end(), body.begin(), body.end());
    clientRawData.assign(response.begin(), response.end());
}

std::string     Response::commonResponse(status_code_t status)
{
    std::pair<status_code_t, std::string> statusCode = 
                                HttpUtils::getResponseStatus(status);

    std::string common = std::string("HTTP/1.1 ");
    common += StringUtils::intToString(statusCode.first) + " " + statusCode.second + "\r\n";
    common += "Server: webserv (Ubuntu)\r\n";
    common += "Date: " + TimeUtils::getFormattedDate(time(NULL)) + "\r\n";

    return (common);
}

std::string     Response::bodyHeaders(std::string extension, unsigned int size)
{
    std::string headers = "";
    headers += "Content-Type: " + HttpUtils::getMimeType(extension) + "\r\n";
    headers += "Content-Length: " + StringUtils::intToString(size) + "\r\n";
    return headers;
}

/* void    Response::createResponse(resp_t resp)
{
    std::string response = commonResponse(resp.status);
    response += bodyHeaders(resp.body, resp.extension);
    
    if (resp.keepAlive)
        response += "Connection: keep-alive\r\n";
    else
        response += "Connection: close\r\n";

    response += "\r\n";
    if (resp.body.empty())
        resp.rawData.assign(response.begin(), response.end());
    else
    {
        resp.rawData.assign(response.begin(), response.end());
        resp.rawData.insert(resp.rawData.end(), resp.body.begin(), resp.body.end());
    }
} */

void    Response::errorResponse(status_code_t code, Client & client) 
{
    std::pair<status_code_t, std::string> statusCode = 
                                HttpUtils::getResponseStatus(code);

	std::string error = "<html>\n<head><title>" + StringUtils::intToString(statusCode.first);
    error += " " + statusCode.second;
    error += "</title></head>\n<body>\n<center><h1>";
    error += StringUtils::intToString(code);
    error += " " + statusCode.second;
    error += "</h1></center>\n<hr><center>webserv (Ubuntu)</center>\n</body>\n</html>\n";

    std::string response = commonResponse(code);
    response += "Connection: close\r\n";
    response += bodyHeaders("html", error.size());
    response += "\r\n" + error;

    std::vector<unsigned char> data;
    data.assign(response.begin(), response.end());

    client.fillRawData(data);
    client.readyToRespond();
}

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

int Response::deleteResponse(std::string path)
{
	struct stat         stat;
	status_code_t		statCode;
    ft_bzero(&stat, sizeof(struct stat));
    if (!lstat(path.c_str(), &stat)) {
        if (S_ISREG(stat.st_mode) || S_ISDIR(stat.st_mode))
           statCode=NO_CONTENT;
    }
    else
        statCode=NOT_FOUND;
    if (S_ISDIR(stat.st_mode) == true &&
       *(--path.end()) != '/'){
        statCode=CONFLICT;}
	if (!lstat(path.c_str(), &stat)) {
        if (S_ISREG(stat.st_mode) || S_ISLNK(stat.st_mode))
            unlink(path.c_str());
        else if (S_ISDIR(stat.st_mode))
            FileUtils::_removeDir(path.c_str());
    }
	return statCode;
}
