/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConf.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eantoine <eantoine@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/23 18:24:01 by lfrederi          #+#    #+#             */
/*   Updated: 2023/07/31 11:32:20 by eantoine         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConf.hpp"
#include "StringUtils.hpp"
#include "Client.hpp"
#include "Exception.hpp"
#include "Debugger.hpp"
#include "WebServ.hpp"
#include "FileUtils.hpp"

#include <cstdlib>		// atoi
#include <unistd.h>		// close
#include <sys/socket.h> // accept
#include <fcntl.h>		// fcntl
#include <cstring>		// bzero
#include <sys/epoll.h>	// epoll_ctl
#include <algorithm>
#include <fstream>
#include <sstream>


/************************
 * INIT STATIC VARIABLES
 ***********************/

const ServerConf::server_instruction_tab_entry_t	ServerConf::SERVER_INSTRUCTIONS[] = 
{
	{S_ROOT, "root"},
	{LISTEN, "listen"},
	{SERVER_NAME, "server_name"},
	{S_ERROR_PAGE, "error_page"},
	{S_INDEX, "index"},
	{S_AUTOINDEX, "autoindex"},
	{S_CLIENT_MAX_BODY_SIZE, "client_max_body_size"},
	{S_CGI, "cgi"},
	{LOCATION_INSTRUCTION, "location"}
};
/******************************************************************************/

/*****************
 * CANNONICAL FORM
 *****************/

ServerConf::ServerConf(void)
	: _root("html"),
	  _port(8080),
	  _server_name(),
	  _IP("0.0.0.0"),
	  _error_pages("error.html"),
	  _autoindex(false),
	  _client_body_size(1000000)
{
	_index.push_back("index");
	_index.push_back("index.html");
	_server_name.push_back("");
	_cgi["php"] = "/usr/bin/php-cgi";
}

ServerConf::ServerConf(ServerConf const &src)
	: _root(src._root),
	  _port(src._port),
	  _server_name(src._server_name),
	  _IP(src._IP),
	  _error_pages(src._error_pages),
	  _index(src._index),
	  _autoindex(src._autoindex),
	  _client_body_size(src._client_body_size),
	  _cgi(src._cgi),
	  _location(src._location)
{
}

ServerConf &ServerConf::operator=(ServerConf const &src)
{
	if (this != &src)
	{
		_root = src._root;
		_port = src._port;
		_server_name = src._server_name;
		_IP = src._IP;
		_error_pages = src._error_pages;
		_index = src._index;
		_autoindex = src._autoindex;
		_client_body_size = src._client_body_size;
		_cgi = src._cgi;
		_location = src._location;
	}
	return *this;
}

ServerConf::~ServerConf()
{
}
/******************************************************************************/

/***********
 * ACCESSORS
 ************/

std::string const &ServerConf::getRoot() const { return (_root); }
std::map<std::string, std::string> const &ServerConf::getCgi() const { return (_cgi); }
int const &ServerConf::getPort() const { return (_port); }
std::vector<std::string> const &ServerConf::getName() const { return (_server_name); }
std::vector<std::string> const &ServerConf::getIndex() const { return (_index); }
bool const &ServerConf::getAutoindex() const { return (_autoindex); }
std::string const &ServerConf::getIp() const { return (_IP); }
std::string const &ServerConf::getError() const { return (_error_pages); }
long int const &ServerConf::getClientBodySize() const { return (_client_body_size); }
std::vector<Location> const &ServerConf::getLocation() const { return (_location); }

void ServerConf::setRoot(std::vector<std::string> token)
{
	if (token.size() > 2)
		throw(ConfFileParseError("Only one root allowed"));
	_root = token[1].erase(token[1].size() - 1);
	StringUtils::addCwd(_root);
	if (_root[0] != '/')
		_root="/" + _root;
	if (!FileUtils::isDirectory(_root.c_str()))

		throw(ConfFileParseError(_root + "Root directive is wrong"));
}

void ServerConf::setCgi(std::vector<std::string> token)
{
	if (token.size() != 3)
		throw(ConfFileParseError("cgi argument problem"));
	else if (token[1].compare("php"))
		throw(ConfFileParseError("Our server deals only with php"));
	_cgi.insert(std::pair<std::string, std::string>(token[1], token[2].erase(token[2].size() - 1)));
}

void ServerConf::setPort(std::vector<std::string> token)
{
	setIp(token);
	std::string str;
	if (token.size() > 2)
		throw(ConfFileParseError("Only one port allowed"));
	std::vector<std::string> tmp = StringUtils::splitString(token[1], ":");
	if (tmp.size() == 1)
		str = tmp[0].substr(0, tmp[0].size() - 1);
	else
		str = tmp[1].substr(0, tmp[1].size() - 1);
	for (size_t i = 0; i < str.size(); i++)
		if (str[i] < 48 || str[i] > 57)
			throw(ConfFileParseError("put only digits for port"));
	_port = atoi(str.c_str());
}

void ServerConf::setIp(std::vector<std::string> token)
{
	std::string str;
	if (token.size() > 2)
		throw(ConfFileParseError("Only one IP:port allowed"));
	std::vector<std::string> tmp = StringUtils::splitString(token[1], ":");
	if ((!tmp[0].compare("*") && tmp[0].size() == 1))
	{
		_IP = tmp[0];
		return;
	}
	if ((!tmp[0].compare("localhost") && tmp[0].size() == 9))
	{
		_IP = "127.0.0.1";
		return;
	}
	std::vector<std::string> tmp1 = StringUtils::splitString(tmp[0], ".");
	if (tmp1.size() != 4)
		throw(ConfFileParseError("IP formating problem"));
	for (int j = 0; j < 4; j++)
	{
		if (atoi(tmp1[j].c_str()) < 0 || atoi(tmp1[j].c_str()) > 255)
			throw(ConfFileParseError("IP formating problem"));
	}
	_IP = tmp[0];
}

void ServerConf::setName(std::vector<std::string> token)
{
	_server_name.push_back("");
	if (token.size() < 2)
		throw(ConfFileParseError("Parameter problem with ServerConf name"));
	token.erase(token.begin());
	for (std::vector<std::string>::iterator it = token.begin(); it != token.end(); ++it)
	{
		std::string &str = *it;
		str.erase(std::remove(str.begin(), str.end(), ';'), str.end());
	}
	std::sort(token.begin(), token.end());
	token.erase(std::unique(token.begin(), token.end()), token.end()); // enleve les noms en doublons dans chaque serveur
	_server_name = token;
}

void ServerConf::setError(std::vector<std::string> token)
{
	if (token.size() != 2)
		throw(ConfFileParseError("problem with number of arguments for error_page"));
	_error_pages = token[1].erase(token[1].size() - 1);
	StringUtils::addCwd(_error_pages);
}

void ServerConf::setIndex(std::vector<std::string> token)
{
	_index.clear();
	size_t i = 1;
	if (token.size() < 2)
		throw(ConfFileParseError("problem with number of arguments for index"));
	for (; i < token.size(); i++)
	{
		if (token[i].compare(" "))
		{
			if (i == token.size() - 1)
				token[i].erase(token[i].size() - 1);
			_index.push_back(token[i]);
		}
	}
}

void ServerConf::setAutoindex(std::vector<std::string> token)
{
	if (token.size() != 2 || !(token[1].compare("on") || token[1].compare("off")))
		throw(ConfFileParseError("problem with autoindex argument"));
	switch (token[1].erase(token[1].size() - 1).compare("on"))
	{
	case 0:
		_autoindex = true;
		break;

	default:
		_autoindex = false;
		break;
	}
}

void ServerConf::setClientBodySize(std::vector<std::string> token)
{
	if (token.size() > 2)
		throw(ConfFileParseError("Only one client body size max"));
	std::string str = token[1].erase(token[1].size() - 1);
	size_t i = 0;
	while (i < str.length() && (std::isspace(str[i]) || std::isdigit(str[i])))
		i++;
	if (str.length() != i && str[i] != 'M' && str[i] != 'm' && str[i] != 'G' && str[i] != 'g' && str[i] != 'k' && str[i] != 'K')
		throw(ConfFileParseError("error client body size syntax"));
	if (str.length() != i && str.length() != i + 1)
		throw(ConfFileParseError("error client body size syntax"));
	_client_body_size = atoi(token[1].erase(token[1].size() - 1).c_str());
	if (str[i] && (str[i] == 'M' || str[i] == 'm'))
		_client_body_size *= 1000000;
	else if (str[i] && (str[i] == 'G' || str[i] == 'g'))
		_client_body_size *= 1000000000;
	else if (str[i] && (str[i] == 'k' || str[i] == 'K'))
		_client_body_size *= 1000;
}

void	ServerConf::setErrorContent(std::string const path){
	std::ifstream fileStream(path.c_str()); 
	if (fileStream) {
		std::stringstream buffer;
		buffer << fileStream.rdbuf();
		_errorContent = buffer.str();
		fileStream.close();
	} else {
	throw std::runtime_error("Impossible d'ouvrir le fichier : " + path);
	}
}

void 	ServerConf::setFolderContent(std::string const path){
	std::ifstream fileStream(path.c_str()); 
	if (fileStream) {
		std::stringstream buffer;
		buffer << fileStream.rdbuf();
		_folderContent = buffer.str();
		fileStream.close();
	} 
	else {
	throw std::runtime_error("Impossible d'ouvrir le fichier : " + path);
	}
}





/******************************************************************************/

/****************
 * PUBLIC METHODS
 ****************/

/**
 * @brief 
 * @param Server_info 
 */
void ServerConf::cleanDupServerConf(std::vector<ServerConf> Server_info)
{
	if (Server_info.size() == 0)
		return;
	std::vector<ServerConf>::iterator it = Server_info.begin();
	std::vector<std::string>::iterator it2;
	std::string list_names;
	std::string res;
	std::vector<std::string> vect_names1;
	std::vector<std::string> vect_names2 = _server_name;

	for (; it != Server_info.end(); it++)
	{
		if (!(this->_IP.compare(it->_IP)) && (this->_port == (it->_port)))
		{
			//list_names = (*it).getName();
			//vect_names1 = StringUtils::splitString(list_names);
			vect_names1 = (*it).getName();
			std::vector<std::string> vect_names4(vect_names1.size() + vect_names2.size());
			it2 = std::set_intersection(vect_names1.begin(), vect_names1.end(), vect_names2.begin(), vect_names2.end(), vect_names4.begin());
			vect_names4.resize(it2 - vect_names4.begin());
			it2 = vect_names4.begin();
			for (; it2 != vect_names4.end(); it2++)
				std::cerr << "Duplicate ServerConf name : [" << *it2 << "] only the first instance will be kept, the other will be ignored\n";

			std::vector<std::string> vect_names3;
			std::set_difference(vect_names2.begin(), vect_names2.end(),
								vect_names1.begin(), vect_names1.end(), std::inserter(vect_names3, vect_names3.begin()));
			this->_server_name = vect_names3;
		}
	}
}

/**
 * @brief from the config string
 * parse the ServerConf and locations blocs
 *
 * @param str
 */
void ServerConf::setServerConf(const std::string &str)
{
	int count = 0;
	int pos_end = StringUtils::findClosingBracket(str);
	// DEBUG_COUT("\n****ServerConf to parse without line ServerConf {  \n\n" + str);
	while (count < pos_end)
	{
		parseServerConf(str, count);
		count++;
	}
	count = 0;
	int flag = 0;
	while (count < pos_end)
	{
		if (getLocationBloc(str, count) == -1)
			count++;
		else
			addLocation(str, count, flag);
	}
	if (!flag){
		Location loc(getPort(), getCgi(), getAutoindex(), getIndex(), getRoot(), getClientBodySize(), getError(), "/");
		this->_location.push_back(loc);
	} //cree un location bloc / si inexistant
	setErrorContent(getError());

	char wd[FILENAME_MAX];
	if(getcwd(wd, sizeof(wd)) != NULL)
	{
		std::string cwd(wd);
		std::string str2(cwd + "/folder.html");
		setFolderContent(str2);}
		else {
			std::cerr << "Error getting current directory." << std::endl;	
	}
}

/**
 * @brief 
 * @param str 
 * @param count 
 * @param Server_ct 
 */
void ServerConf::addLocation(std::string str, int &count, int &flag)
{
	Location loc(getPort(), getCgi(), getAutoindex(), getIndex(), getRoot(), getClientBodySize(), getError());
	loc.setLocation(str, count, flag);
	this->_location.push_back(loc);
}
/******************************************************************************/

/*****************
 * PRIVATE METHODS
 *****************/

/**
 * @brief returns the number of line of location bloc
 *
 * @param str
 * @param count
 * @return int
 */
int ServerConf::skipLocationBlock(std::string str, int count)
{
	int i = 0;
	int in = 0;
	int ct = 0;
	while (str[i] && ct < count)
	{
		if (str[i] == '\n')
			ct++;
		i++;
	}
	i += 8;
	while (str[i] && str[i] != '\n' && in == 0)
	{
		if (str[i] == '{')
			in = 1;
		i++;
	}
	ct += StringUtils::findClosingBracket(str.substr(i + 2));
	return (ct - 1 - count);
}

/// @brief
/// @param str
/// @param count
/// @return
int ServerConf::getLocationBloc(std::string str, int &count)
{
	std::vector<std::string> token;
	std::string line = StringUtils::getLine(str, count);
	if ((token = StringUtils::splitString(line, WHITESPACES)).empty())
		return -1;
	else if (correctServerInstruction(token) != LOCATION_INSTRUCTION)
		return -1;
	else
		return 1;
}

/**
 * @brief 
 * @param funcs 
 */
void ServerConf::init_vector_ServerConf_fct(std::vector<ServerConf_func> &funcs)
{
	funcs.push_back(&ServerConf::setRoot);
	funcs.push_back(&ServerConf::setPort);
	funcs.push_back(&ServerConf::setName);
	funcs.push_back(&ServerConf::setError);
	funcs.push_back(&ServerConf::setIndex);
	funcs.push_back(&ServerConf::setAutoindex);
	funcs.push_back(&ServerConf::setClientBodySize);
	funcs.push_back(&ServerConf::setCgi);
}

/**
 * @brief fills all the ServerConf data
 *
 * @param str
 */
void ServerConf::parseServerConf(std::string str, int &count)
{
	std::vector<ServerConf_func> funcs;
	init_vector_ServerConf_fct(funcs);

	std::vector<std::string> token;
	int instruct;
	std::string line = StringUtils::getLine(str, count);
	if ((token = StringUtils::splitString(line, WHITESPACES)).empty())
		return;
	else if ((instruct = correctServerInstruction(token)) != -1)
	{
		if (instruct == LOCATION_INSTRUCTION)
			count += skipLocationBlock(str, count);
		else if (instruct < TOTAL_SERVER_INSTRUCTIONS && instruct != LOCATION_INSTRUCTION)
			(this->*funcs[instruct])(token);
	}
	else
		throw(ConfFileParseError("Wrong input in ServerConf : Directive " + token[0] + " invalid"));
}

/**
 * @brief Checks if the server directive are part of server or location block
 *
 * @param token
 * @return true
 * @return false
 */
int ServerConf::correctServerInstruction(std::vector<std::string> token)
{
	int i = 0;
	// check if the token corresponds to a valid instruction in server block
	while (i < TOTAL_SERVER_INSTRUCTIONS)
	{
		if (!token[0].compare(SERVER_INSTRUCTIONS[i].name))
			return i;
		i++;
	}
	if (StringUtils::isNothing(token[0]) || !token[0].compare("{"))
		return TOTAL_SERVER_INSTRUCTIONS;
	return -1;
}


void	ServerConf::sortLocationBlock()
{
	std::sort(_location.begin(), _location.end());
}
/******************************************************************************/

/***********
 * FUNCTIONS
 ************/

std::ostream &operator<<(std::ostream &o, ServerConf const &i)
{
	o << "************* ServerConf bloc [" << i.getName() << "] *************" << std::endl;
	if (i.getIp().empty() == false)
		o << "    listen			=	[" << i.getIp() << "]" << std::endl;
	if (i.getPort() > 0)
		o << "    portNumber			=	[" << i.getPort() << "]" << std::endl;
	if (i.getName().empty() == false)
		o << "    ServerName			=	[" << i.getName() << "]" << std::endl;
	if (i.getRoot().empty() == false)
		o << "    root			=	[" << i.getRoot() << "]" << std::endl;
	if (i.getIndex().empty() == false)
	{
		o << "    indexPage			=	[" << i.getIndex() << "]" << std::endl;
	}
	o << "    autoindex			=	[" << i.getAutoindex() << "]" << std::endl;
	if (i.getError().empty() == false)
		o << "    errorPage			=	[" << i.getError() << "]" << std::endl;
	if (i.getCgi().empty() == false)
	{
		std::map<std::string, std::string>::const_iterator ite;
		for (ite = i.getCgi().begin(); ite != i.getCgi().end(); ite++)
			o << "    cgi 			=	[" << ite->first << " ; " << ite->second << "]" << std::endl;
	}
	if (i.getClientBodySize() > 0)
		o << "    clientMaxBodySize		=	[" << i.getClientBodySize() << "]" << std::endl;
	for (size_t j = 0; j < i.getLocation().size(); j++)
		o << i.getLocation()[j] << std::endl;
	return (o);
}

std::ostream &operator<<(std::ostream &o, std::vector<ServerConf> const &srv)
{
	for (size_t i = 0; i < srv.size(); i++)
		o << srv[i] << std::endl;
	return (o);
}

std::ostream &operator<<(std::ostream &o, std::vector<std::string> const &str)
{
	for (size_t i = 0; i < str.size(); i++)
		if (i < str.size() - 1)
			o << str[i] << " ; ";
		else
			o << str[i];
	return (o);
}