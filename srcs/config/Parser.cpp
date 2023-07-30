/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eantoine <eantoine@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/31 19:39:21 by lfrederi          #+#    #+#             */
/*   Updated: 2023/07/30 17:36:36 by eantoine         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser.hpp"
#include "StringUtils.hpp"
#include "FileUtils.hpp"
#include "Exception.hpp"
#include "Debugger.hpp"

#include <sys/socket.h> // socket
#include <netinet/in.h> // sockaddr_in
#include <arpa/inet.h> // inet_addr
#include <netdb.h> // getprotobyname
#include <fstream>
#include <algorithm> // sort

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

	testPath(this->_configFile);
	conf_string = getStringConf();
	formatConfFile(conf_string);

	int j = StringUtils::nbLines(conf_string);
	for (int i = 0; i < j; i++){
		temp = StringUtils::trimWhitespaces(StringUtils::getLine(conf_string, i));
		conf_string_formated += temp + checkChar(temp);
	}

	formatConfFile(conf_string_formated);
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

		temp = StringUtils::trimComments(line);
		temp = StringUtils::trimWhitespaces(temp);
		content += temp;
		content +="\n";
	}
	if (!content.empty())
		content.erase(content.end() - 1);
//	if (DEBUG_STATUS)
  //  	std::cout << content << std::endl;
	file.close();
	if (!StringUtils::checkBrackets(content))
		throw ConfFileParseError("Error in the backets");
	return content;
}

/**
 * @brief 
 * Parse server info in vector<Server>
 * server_info is updated with correct info
 * @param std::vector<server> server_info 
 * @param std::string rawConfig 
 */
void    Parser::parseServers(std::vector<ServerConf> & serverConfs, std::string rawConfig)
{
	size_t i = 0;
	if (DEBUG_STATUS)
	{
		//DEBUG_COUT("conf file nb lines :" + Syntax::intToString(Syntax::nb_lines(rawConfig)));
		//DEBUG_COUT("\n****Server to parse without line server {****  \n\n" + rawConfig);
	}
	while (i < StringUtils::nbLines(rawConfig))
	{
		if (!StringUtils::isNothing(rawConfig, i))
		{
			std::string	line = StringUtils::getLine(rawConfig, i);
			if (!line.compare(0, 6, "server") && line.size() == 6)
			{
				i++;
				std::string	line = StringUtils::getLine(rawConfig, i);
				if (line.compare("{"))
					throw(ConfFileParseError("Invalid Server Header"));
				ServerConf tempServer;
				tempServer.setServerConf( StringUtils::trimLineToI(rawConfig, i + 1));
				tempServer.cleanDupServerConf(serverConfs);
				tempServer.sortLocationBlock();
				serverConfs.push_back(tempServer);
			}
		}
		i++;
	}

	if (DEBUG_STATUS)
		std::cout << serverConfs << std::endl;
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

		if (!(proto = getprotobyname("tcp")))
			throw(SetServerException("Problem using protobyname, protocol not found"));
		if ((socketFd = socket(AF_INET, SOCK_STREAM, proto->p_proto)) == -1)
			throw(SetServerException("Problem creating Socket"));
		DEBUG_COUT("Server " + it->first.first + ":" + StringUtils::intToString(it->first.second) + " created");
		if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(enabled)) == -1)
			throw(SetServerException("Problem setting Socket options"));//

		sockaddr.sin_family = AF_INET;
		sockaddr.sin_port = htons(it->first.second);
		sockaddr.sin_addr.s_addr = inet_addr(it->first.first.c_str());
		if (bind(socketFd, reinterpret_cast<struct sockaddr *>(&sockaddr), sizeof(sockaddr)) == -1)
			throw(SetServerException("Problem binding socket"));
		DEBUG_COUT("Server with file descriptor " <<  socketFd << " has been successfully bind on port: " << StringUtils::intToString(it->first.second));

		if (listen(socketFd, MAX_CLIENT))
			throw(SetServerException("Problem with listen")); 

		webServ.addFd(socketFd, new Server(socketFd, webServ, it->second));  
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

/**
 * @brief tool used to format properly the conf string
 * returning \n or " "
 * 
 * @param str 
 * @return char 
 */
char	Parser::checkChar(std::string str)
{
		if (str[str.size()-1] == ';')
			return '\n';
		if (!str.compare("server") || !str.compare("{") || !str.compare("}") )
			return '\n';
		return (' ');
}

/**
 * @brief makes sure the conf file lines finishes by ;
 * and { are isolated in one line
 * deleting the single \n
 * 
 * @param std::string &conf 
 */
void	Parser::formatConfFile(std::string &conf)
{
	int i = 0;
	int count = 0;
	int j = conf.size();
	while (i < j){
		if (conf[i] == ';'){
			conf.replace(i, 1, ";\n");
			j++;
			count++;
		}
		i++;
	}
	i = 0;
	j = conf.size();
	while (i < j){
		if (conf[i] == '{'){
			conf.replace(i, 1, "\n{\n");
			j +=2;
			i +=2;
		}
		else i++;
	}
	i = 0;
	j = conf.size();
	while (i < j){
		if (conf[i] == '}'){
			conf.replace(i, 1, "\n}\n");
			j+=2;
			i+=2;
		}
		else
			i++;
	}
	i = 0;
	j = conf.size();
	while (i < j){
		if (conf[i] == '\n' && conf[i+1] && conf[i+1] == '\n'){
			conf.replace(i, 2, "\n");
			j-=1;
			i-=1;
		}
		else
			i++;
	}
	if (count != StringUtils::nbDeclarations(conf))
		throw(ConfFileParseError("problem with instructions or semi-columns") );
}

/**
 * @brief 
 * test if path to .conf is correct, testing
 * emptypath, extension, readable, if directory
 * trying to open file
 * @param path 
 */
void    Parser::testPath(const std::string &path)
{
	size_t ext_pos;
	std::ifstream file;
	
	if (path.empty())
		throw (EmptyConfPath());
	ext_pos = path.find(".conf");
	if (ext_pos == std::string::npos || ext_pos != path.size() - 5)
		throw (BadExtensionConfFile());
	if (!FileUtils::fileExists(path.c_str()))
		throw (FileDoesNotExist());
	if (!FileUtils::fileRead(path.c_str()))
		throw (FileNotReadable());
	if (FileUtils::isDirectory(path.c_str()))
		throw (PathIsDir());
	file.open(path.c_str(), std::ios_base::in);
	if (!file) {
		throw (InvalidConfFilePath());
	}
}