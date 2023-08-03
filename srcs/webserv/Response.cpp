/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfrederi <lfrederi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/03 19:19:11 by lfrederi          #+#    #+#             */
/*   Updated: 2023/08/03 16:56:03 by lfrederi         ###   ########.fr       */
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
#include <cstring>   // strncmp
#include <cstdlib>   // atoi

/***********************
 * STATIC PUBLIC METHODS
 ***********************/

void Response::cgiResponse(Client &client, std::string headers, std::vector<unsigned char> &body) {

    int statusCode = 200;

    std::vector<std::string> lines = StringUtils::splitString(headers, "\r\n");

    std::string statusLine = lines[0];
    std::string cmp = "Status: ";
    if (std::strncmp(cmp.c_str(), statusLine.c_str(), cmp.size()) == 0)
    {
        statusCode = std::atoi(statusLine.substr(cmp.size() - 1, 4).c_str());
        statusCode = HttpUtils::getResponseStatus(static_cast<status_code_t>(statusCode)).first;
        if (statusCode >= 400)
            throw RequestError(static_cast<status_code_t>(statusCode), "Cgi error response status");
    }

    std::string common = Response::commonResponse(static_cast<status_code_t>(statusCode), true);
    for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); it++)
    {
        common.append(it->begin(), it->end());
        common += "\r\n";
    }
    common += "Content-Length: " + StringUtils::intToString(body.size()) + "\r\n\r\n";

    std::vector<unsigned char> response;
    response.assign(common.begin(), common.end());
    response.insert(response.end(), body.begin(), body.end());

    client.fillRawData(response);
    client.readyToRespond();
}


void Response::errorResponse(status_code_t code, Client &client) {

    client.closeClient();

    std::vector<unsigned char> error;

    getFileContent(client.getServerConf()->getError(), error);
    std::pair<status_code_t, std::string> statusCode = HttpUtils::getResponseStatus(code);

    std::string headers = commonResponse(code, false);
    headers += bodyHeaders("html", error.size()) + "\r\n";

    std::vector<unsigned char> data;
    data.assign(headers.begin(), headers.end());

    client.fillRawData(data);
    client.fillRawData(error);
    client.readyToRespond();
}

/**
 * @brief
 *
 */
void Response::getResponse(std::string const &path, Client &client)
{

    struct stat stat;
    std::vector<unsigned char> body;
    std::string headers;

    bzero(&stat, sizeof(stat));
    if (lstat(path.c_str(), &stat) == -1)
        throw RequestError(NOT_FOUND, "Impossible to get information about path");

    if (S_ISDIR(stat.st_mode))
        throw RequestError(INTERNAL_SERVER_ERROR, "Should open default directory file");
    // getDirectoryStructure();
    else
        getFileContent(path, body);

    headers = commonResponse(OK, true) + bodyHeaders("html", body.size()) + "\r\n";
    std::vector<unsigned char> data;
    data.assign(headers.begin(), headers.end());

    client.fillRawData(data);
    client.fillRawData(body);
    client.readyToRespond();
}

/**
 * @brief
 *
 * @param path
 * @param client
 */
void Response::deleteResponse(const std::string &path, Client &client)
{
    struct stat stat;
    std::string response;

    bzero(&stat, sizeof(struct stat));
    if (lstat(path.c_str(), &stat) == -1)
        throw RequestError(NOT_FOUND, "Impossible to delete path");

    if (S_ISDIR(stat.st_mode) == true && *(--path.end()) != '/')
        throw RequestError(CONFLICT, "Conflict deleting path");

    if (S_ISREG(stat.st_mode) || S_ISLNK(stat.st_mode))
    {
        if (unlink(path.c_str()) == -1)
            throw RequestError(FORBIDDEN, "No access authorisation to path");
    }
    else if (S_ISDIR(stat.st_mode))
    {
        if (FileUtils::_removeDir(path.c_str()) == -1)
            throw RequestError(FORBIDDEN, "No access authorisation to path");
    }
    else
        throw RequestError(NOT_IMPLEMENTED, "Unable to delete path");

    response = commonResponse(NO_CONTENT, true);

    std::vector<unsigned char> data;
    data.assign(response.begin(), response.end());
    client.fillRawData(data);
    client.readyToRespond();
}
/******************************************************************************/

/***********************
 * STATIC PRIVATE METHODS
 ***********************/

/**
 * @brief
 *
 * @param status
 * @param alive
 * @return std::string
 */
std::string Response::commonResponse(status_code_t status, bool alive)
{
    std::pair<status_code_t, std::string> statusCode = HttpUtils::getResponseStatus(status);

    std::string common = std::string("HTTP/1.1 ");
    common += StringUtils::intToString(statusCode.first) + " " + statusCode.second + "\r\n";
    common += "Server: webserv (Ubuntu)\r\n";
    common += "Date: " + TimeUtils::getFormattedDate(time(NULL)) + "\r\n";
    if (alive)
        common += "Connection: keep-alive\r\n";
    else
        common += "Connection: close\r\n";

    return (common);
}

/**
 * @brief
 *
 * @param extension
 * @param size
 * @return std::string
 */
std::string Response::bodyHeaders(std::string extension, unsigned int size)
{
    std::string headers = "Content-Type: " + HttpUtils::getMimeType(extension) + "\r\n";
    headers += "Content-Length: " + StringUtils::intToString(size) + "\r\n";
    return headers;
}

/**
 * @brief
 *
 * @param path
 * @param response
 */
void Response::getFileContent(std::string const &path, std::vector<unsigned char> &response)
{

    std::ifstream is(path.c_str(), std::ifstream::binary);

    if (!is.good())
        throw RequestError(NOT_FOUND, "File not found or impossible to read: " + path);

    is.seekg(0, is.end);
    std::streampos length = is.tellg();
    is.seekg(0, is.beg);

    char *buffer = new char[length];
    is.read(buffer, length);

    if (!is.good())
        throw RequestError(INTERNAL_SERVER_ERROR, "Faile while reading file: " + path);

    is.close();

    response.assign(buffer, buffer + length);
    delete[] buffer;
}