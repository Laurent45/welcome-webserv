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

std::string     Response::cgiSimpleResponse(std::string & body)
{
    std::string response = std::string("HTTP/1.1 ") + "200 " + "Ok\r\n";
    std::stringstream ss;
    ss << body.size();
    
    response += "Server: webserv (Ubuntu)\r\n";
    response += "Date: Sat, 10, Jun 2023 09:15:38 GMT\r\n";
    response += "Content-Type: text/html\r\n";
    response += "Content-Length: " + ss.str() + "\r\n";
    response += "Connection: keep-alive\r\n\r\n";
    response += body;

    return (response);
}

std::string     Response::commonResponse(status_code_t status)
{
    std::string common = std::string("HTTP/1.1 ");
    common += StringUtils::intToString(status) + " " + HttpUtils::RESPONSE_STATUS.at(status) + "\r\n";
    common += "Server: webserv (Ubuntu)\r\n";
    common += "Date: " + TimeUtils::getFormattedDate(time(NULL)) + "\r\n";

    return (common);
}

std::string     Response::bodyHeaders(std::string extension, unsigned int size)
{
    std::string headers = "";
    headers += "Content-Type: " + HttpUtils::MIME_TYPES.at(extension) + "\r\n";
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
	std::string error = "<html>\n<head><title>" + StringUtils::intToString(code);
    error += " " + HttpUtils::RESPONSE_STATUS.at(code);
    error += "</title></head>\n<body>\n<center><h1>";
    error += StringUtils::intToString(code);
    error += " " + HttpUtils::RESPONSE_STATUS.at(code);
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
