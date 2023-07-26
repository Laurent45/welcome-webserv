/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfrederi <lfrederi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/03 19:19:11 by lfrederi          #+#    #+#             */
/*   Updated: 2023/07/25 16:47:02 by lfrederi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"
#include "StringUtils.hpp"
#include "TimeUtils.hpp"
#include "Client.hpp"

#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm> // search
#include <cstring> // strncmp
#include <cstdlib> // atoi

void    Response::cgiResponse(std::vector<unsigned char> & rawData,
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
    rawData.assign(response.begin(), response.end());
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

/* 	if (method == "GET")
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
	} */