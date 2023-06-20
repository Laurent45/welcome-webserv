/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfrederi <lfrederi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/31 19:39:21 by lfrederi          #+#    #+#             */
/*   Updated: 2023/06/19 20:00:08 by lfrederi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser.hpp"
#include "Syntax.hpp"
#include "Exception.hpp"
#include "Debugger.hpp"

#include <sys/socket.h> // socket
#include <netinet/in.h> // sockaddr_in
#include <arpa/inet.h> // inet_addr
#include <netdb.h> // getprotobyname

/*****************
* CANNONICAL FORM
*****************/

Parser::Parser()
{}

Parser::Parser(Parser const & copy) : _configFile(copy._configFile)
{}

Parser & Parser::operator=(Parser const & rhs)
{
    (void) rhs;
    return (*this);
}

Parser::~Parser()
{}

Parser::Parser(std::string configFile) : _configFile(configFile)
{}

/******************************************************************************/

/****************
* PUBLIC METHODS
****************/

void    Parser::parseConfFile(WebServ & webServ)
{
    std::vector<ServerConf> server_list;
	std::string conf_string;
	std::string conf_string_formated = "";
	std::string temp;

	Syntax::testPath(this->_configFile);
	conf_string = getStringConf();
	Syntax::formatConfFile(conf_string);

	int j = Syntax::nbLines(conf_string);
	for (int i = 0; i < j; i++){
		temp = Syntax::trimWhitespaces(Syntax::getLine(conf_string, i));
		conf_string_formated += temp + Syntax::checkChar(temp);
	}

	Syntax::formatConfFile(conf_string_formated);
	conf_string_formated.erase(conf_string_formated.size() - 1);
	parseServers(server_list, conf_string_formated);
	fillServersMap(server_list);
    createServerSockets(webServ);
}

/******************************************************************************/

/****************
* PRIVATE METHODS
****************/

/**
 * @brief 
 *	gets the file .conf transformed in a single string 
 *	with #comments removed, whitespaces begin and end removed
 *	and many whitespaces reduced to a space
 *	return : std::string
 * @param std::string const &path 
 * @return std::string 
 */
std::string     Parser::getStringConf() const
{
	std::string content;
	std::string line;
	std::vector<std::string> tokens;
	std::ifstream file;
	
    file.open(this->_configFile.c_str(), std::ifstream::in);
	if (!file)
    	throw (CantOpenConfFile());
	while (std::getline(file, line))
	{
		std::string temp;

		temp = Syntax::trimComments(line);
		temp = Syntax::trimWhitespaces(temp);
		content += temp;
		content +="\n";
	}
	if (!content.empty())
		content.erase(content.end() - 1);
//	if (DEBUG_STATUS)
  //  	std::cout << content << std::endl;
	file.close();
	if (!Syntax::checkBrackets(content))
		throw ConfFileParseError("Error in the backets");
	return content;
}

/**
 * @brief 
 * Parse server info in vector<Server>
 * server_info is updated with correct info
 * @param std::vector<server> server_info 
 * @param std::string str_config 
 */
void    Parser::parseServers(std::vector<ServerConf> & server_info, std::string str_config)
{
	size_t i = 0;
	if (DEBUG_STATUS)
	{
		//DEBUG_COUT("conf file nb lines :" + Syntax::intToString(Syntax::nb_lines(str_config)));
		//DEBUG_COUT("\n****Server to parse without line server {****  \n\n" + str_config);
	}
	while (i < Syntax::nbLines(str_config))
	{
		if (!Syntax::isNothing(str_config, i))
		{
			std::string	line = Syntax::getLine(str_config, i);
			if (!line.compare(0, 6, "server") && line.size() == 6)
			{
				i++;
				std::string	line = Syntax::getLine(str_config, i);
				if (line.compare("{"))
					throw(ConfFileParseError("Invalid Server Header"));
				ServerConf temp_server;
				temp_server.setServerConf( Syntax::trimLineToI(str_config, i + 1));
				temp_server.cleanDupServerConf(server_info);
				server_info.push_back(temp_server);
			}
		}
		i++;
	}

	if (DEBUG_STATUS)
		std::cout << server_info << std::endl;
}

void    Parser::createServerSockets(WebServ & webServ) const
{
	std::map<std::pair<std::string, int>, std::vector<ServerConf> >::const_iterator it;

	it = _map.begin();
	for (; it != _map.end(); it++)
	{
		int	socketFd, enabled = 1;
		struct sockaddr_in sockaddr;
		struct protoent *proto;

		//The getprotobyname function is a part of the C library functions which is used to map a protocol name such as 
		// "tcp" to the corresponding protocol number defined in the netinet/in.h header file.
		if (!(proto = getprotobyname("tcp")))
			throw(SetServerException("Problem using protobyname, protocol not found"));
		if ((socketFd = socket(AF_INET, SOCK_STREAM, proto->p_proto)) == -1)
			throw(SetServerException("Problem creating Socket"));
		DEBUG_COUT("Server " + it->first.first + ":" + Syntax::intToString(it->first.second) + " created");
		// allows a socket to be bound to an address that is already in use, provided that the original socket using the address is no longer active. 
		//This behavior is useful in cases where the server needs to restart after a crash or when multiple instances of the server need to run on the same machine.
		//This option allows the socket to be bound to a previously used address and port, which is useful in cases where the socket is closed and then immediately reopened,
		// without waiting for the operating system to release the socket resources. Without this option,
		//the socket may fail to bind to the address and port, resulting in a "Address already in use" error.
		if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(enabled)) == -1)
			throw(SetServerException("Problem setting Socket options"));//
		sockaddr.sin_family = AF_INET;
		sockaddr.sin_port = htons(it->first.second);
		sockaddr.sin_addr.s_addr = inet_addr(it->first.first.c_str());
		if (bind(socketFd, reinterpret_cast<struct sockaddr *>(&sockaddr), sizeof(sockaddr)) == -1)
			throw(SetServerException("Problem binding socket"));
		DEBUG_COUT("Server with file descriptor " <<  socketFd << " has been successfully bind on port: " << Syntax::intToString(it->first.second));

		//if (fcntl(socketFd, F_SETFL, O_NONBLOCK) == -1) //makes the socket nonblock
		//    throw(SetServerException("Problem setting the socket"));
		if (listen(socketFd, MAX_CLIENT))
			throw(SetServerException("Problem with listen")); 
		webServ.addServer(socketFd, Server(it->second));  
    }
}

void	Parser::fillServersMap(std::vector<ServerConf> & serverConfs)
{
	std::vector<ServerConf>::const_iterator it = serverConfs.begin();

	for (; it != serverConfs.end(); it++)
	{
		std::pair<std::string, int> ipPort = make_pair(it->getIp(), it->getPort());
		_map[ipPort].push_back(*it);
	}
}
