
#include "Location.hpp"
#include "Exception.hpp"
#include "StringUtils.hpp"
#include "HttpUtils.hpp"
#include "FileUtils.hpp"

#include <cstdlib> // atoi

/************************
 * INIT STATIC VARIABLES
 ***********************/

const Location::location_instruction_tab_entry_t	Location::LOCATION_INSTRUCTIONS[] = 
{
	{L_ROOT, "root"},
	{METHODS, "methods"},
	{L_INDEX, "index"},
	{L_CGI, "cgi"},
	{L_AUTOINDEX, "autoindex"},
	{UPLOAD_DIR, "upload_dir"},
	{RETURN,"return"},
	{L_CLIENT_MAX_BODY_SIZE, "client_max_body_size"},
	{L_ERROR_PAGE, "error_page"},
	//{URI, "uri"},
};
/******************************************************************************/


Location::Location(int port, std::map<std::string, std::string> cgi, bool autoindex,
	std::vector<std::string> index, std::string root, int client_body_size, std::string error_pages)
	: _port(port),
	  _locRoot(root),
	  _index(index),
	  _cgi(cgi),
	  _autoindex(autoindex),
	  _client_body_size(client_body_size),
	  _error_pages(error_pages)
	{
		_allow_method.push_back("GET");
		_upload_dir = "";
	}

Location::Location(int port, std::map<std::string, std::string> cgi, bool autoindex,
	std::vector<std::string> index, std::string root, int client_body_size, std::string error_pages, std::string uri)
	: _port(port),
	  _locRoot(root),
	  _index(index),
	  _cgi(cgi),
	  _autoindex(autoindex),
	  _client_body_size(client_body_size),
	  _error_pages(error_pages),
	  _uri(uri)
	{
		_allow_method.push_back("GET");
		_upload_dir = "";
	}

Location::Location()
{}

Location::Location(const Location &src)
	: _port(src._port),
	  _locRoot(src._locRoot),
	  _allow_method(src._allow_method),
	  _index(src._index),
	  _cgi(src._cgi),
	  _autoindex(src._autoindex),
	  _upload_dir(src._upload_dir),
	  _return(src._return),
	  _client_body_size(src._client_body_size),
	  _error_pages(src._error_pages),
	  _uri(src._uri)
{}

Location &Location::operator=(const Location &src)
{
	if (this != &src)
	{
		_port = src._port;
		_locRoot = src._locRoot;
		_allow_method = src._allow_method;
		_index = src._index;
		_cgi = src._cgi;
		_autoindex = src._autoindex;
		_upload_dir = src._upload_dir;
		_return = src._return;
		_client_body_size = src._client_body_size;
		_error_pages = src._error_pages;
		_uri = src._uri;
	}

	return (*this);
}

Location::~Location() {}

/*
** Location getters
*/
int const &Location::getPort() const { return (_port); }
std::string const &Location::getUri() const { return _uri; }
bool const &Location::getAutoindex() const { return _autoindex; }
std::vector<std::string> const &Location::getIndex() const { return _index; }
std::map<int, std::string> const &Location::getReturn() const { return _return; }
std::vector<std::string> const &Location::getAllowMethod() const { return _allow_method; }
std::string const &Location::getLocRoot() const { return _locRoot; }
std::string const &Location::getUploadDir() const { return _upload_dir; }
std::map<std::string, std::string> const &Location::getCgi() const { return _cgi; }
long int const &Location::getClientBodySize() const { return (_client_body_size); }
std::string const &Location::getError() const { return (_error_pages); }

void Location::setLocation(const std::string &str, int &count, int &flag)
{
	std::string line = StringUtils::getLine(str, count);
	std::vector<std::string> token;
	token = StringUtils::splitString(line, WHITESPACES);
	setUri(token);
	if (!_uri.compare("/"))
		flag = 1;
	count++;
	if (StringUtils::getLine(str, count).compare("{"))
		throw(ConfFileParseError("Invalid Location directive : no opening {"));
	count++;
	int i = 0;
	int ct = 0;
	int closing_line;
	while (ct < count)
	{
		if (str[i] == '\n')
			ct++;
		i++;
	}
	closing_line = StringUtils::findClosingBracket(str.substr(i));
	for (i = 0; i < closing_line; i++)
	{
		line = StringUtils::getLine(str, count++);
		parseLocation(line);
	}
}
void Location::init_vector_loc_fct(std::vector<loc_func> &funcs)
{
	funcs.push_back(&Location::setLocRoot);
	funcs.push_back(&Location::setAllowMethod);
	funcs.push_back(&Location::setIndex);
	funcs.push_back(&Location::setCgi);
	funcs.push_back(&Location::setAutoindex);
	funcs.push_back(&Location::setUploadDir);
	funcs.push_back(&Location::setReturn);
	funcs.push_back(&Location::setClientBodySize);
	funcs.push_back(&Location::setErrorPages);
}

void Location::parseLocation(std::string &line)
{
	std::vector<loc_func> funcs;
	init_vector_loc_fct(funcs);
	std::vector<std::string> token;
	int instruct;
	if ((token = StringUtils::splitString(line, WHITESPACES)).empty())
		return;
	else if ((instruct = correctLocationInstruction(token)) != -1 && instruct < TOTAL_LOCATION_INSTRUCTIONS)
		(this->*funcs[instruct])(token);
	else if (instruct == TOTAL_LOCATION_INSTRUCTIONS)
		return;
	else
		throw(ConfFileParseError("Wrong input in Location bloc : Directive " + token[0] + " invalid"));
}

/*
** Location setters
*/
void Location::setUri(std::vector<std::string> token)
{
	if (token.size() < 2)
		throw(ConfFileParseError("Invalid Location directive : not enough arguments"));
	else if (token.size() == 2)	{
		_uri = token[1];
	if (_uri[0] != '/')
		_uri="/" + _uri;
	if (_uri[_uri.size() - 1] != '/')
		_uri+="/";
	}
	else {
		throw(ConfFileParseError("Invalid Location directive "));
	}
}

void Location::setAutoindex(std::vector<std::string> token)
{
	if (token.size() != 2 || !(token[1].compare("on") || token[1].compare("off")))
		throw(ConfFileParseError("Location bloc : problem with autoindex argument, on or off only"));
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
void Location::setIndex(std::vector<std::string> token)
{
	_index.clear();
	size_t i = 1;
	if (token.size() < 2)
		throw(ConfFileParseError("Location bloc : problem with number of arguments for index"));
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

void Location::setAllowMethod(std::vector<std::string> token)
{
	_allow_method.clear();
	if (HttpUtils::correctMethodInstruction(token) == -1)
		throw(ConfFileParseError("Location bloc : Method not allowed"));
	size_t i = 1;
	for (; i < token.size() - 1; i++)
		_allow_method.push_back(token[i]);
	_allow_method.push_back(token[i].erase(token[i].size() - 1));
}

void Location::setLocRoot(std::vector<std::string> token)
{
	if (token.size() > 2)
		throw(ConfFileParseError("Location bloc : Only one root allowed"));
	_locRoot = token[1].erase(token[1].size() - 1);
	StringUtils::addCwd(_locRoot);
	if (_locRoot[0] != '/')
		_locRoot="/" + _locRoot;
	if (!FileUtils::isDirectory(_locRoot.c_str()))
		throw(ConfFileParseError("Root directive is wrong"));

}

void Location::setUploadDir(std::vector<std::string> token)
{
	_upload_dir = "";
	size_t i = 1;
	if (token.size() > 2)
		throw(ConfFileParseError("Location bloc : problem with number of arguments for upload dir"));
	_upload_dir += token[i].erase(token[i].size() - 1);
	StringUtils::addCwd(_upload_dir);
	if (!FileUtils::folderExistsAndWritable(_upload_dir.c_str()))
		throw(ConfFileParseError("Location bloc : Upload dir not accessible"));

}

void Location::setCgi(std::vector<std::string> token)
{
	if (token.size() != 3)
		throw(ConfFileParseError("Location bloc : cgi argument problem"));
	else if (token[1].compare("php"))
		throw(ConfFileParseError("Our server deals only with php"));
	_cgi.clear();
	_cgi.insert(std::pair<std::string, std::string>(token[1], token[2].erase(token[2].size() - 1)));
}
void Location::setErrorPages(std::vector<std::string> token)
{
	if (token.size() != 2)
		throw(ConfFileParseError("Location bloc : problem with number of arguments for error_page"));
	_error_pages = token[1].erase(token[1].size() - 1);
	StringUtils::addCwd(_error_pages);
}
void Location::setClientBodySize(std::vector<std::string> token)
{
	if (token.size() > 2)
		throw(ConfFileParseError("Location bloc : Only one client body size max"));
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

void Location::setReturn(std::vector<std::string> token)
{
	int val;
	if (_return.size() == 0 && token.size() == 2 && ((val = atoi(token[1].erase(token[1].size() - 1).c_str())) >= 200) && val < 700)
	{
		_return[val] = "";
		return;
	}
	if (token.size() != 3)
		throw(ConfFileParseError("Location bloc : problem with number of arguments for return"));
	val = atoi(token[1].erase(token[1].size()).c_str());
	for (size_t i = 0; i < token[1].size(); i++)
		if (token[1][i] < 48 && token[1][i] > 57)
			throw(ConfFileParseError("Location bloc : return : fisrt argument must be numeric"));
	if ((val < 301 && val > 307) || (val > 303 && val < 307))
		throw(ConfFileParseError("Location bloc : return redirection: first argument must be between 301 and 307"));
	if (_return.size() == 0 || ((_return.size() == 1) && !_return.begin()->second.compare("")))
		_return[val] = token[2].erase(token[2].size() - 1);
}

/**
 * @brief Checks if the location directive are part of location block
 *
 * @param token
 * @return true
 * @return false
 */
int		Location::correctLocationInstruction(std::vector<std::string> token)
{
	int i = 0;
	// check if the token corresponds to a valid instruction in server block
	while (i < TOTAL_LOCATION_INSTRUCTIONS)
	{
		if (!token[0].compare(LOCATION_INSTRUCTIONS[i].name))
			return i;
		i++;
	}
	if (StringUtils::isNothing(token[0]) || !token[0].compare("{"))
		return TOTAL_LOCATION_INSTRUCTIONS;
	return -1;
}

std::ostream &operator<<(std::ostream &o, Location const &i)
{
	o << "************* Location bloc  *************" << std::endl;
	if (i.getUri().empty() == false)
		o << "    uri				=	[" << i.getUri() << "] :" << std::endl;
	o << "    portNumber			=	[" << i.getPort() << "]" << std::endl;
	o << "    autoindex			=	[" << i.getAutoindex() << "]" << std::endl;
	if (i.getIndex().empty() == false)
		o << "    index			=	[" << i.getIndex() << "]" << std::endl;
	if (i.getLocRoot().empty() == false)
		o << "    root			=	[" << i.getLocRoot() << "]" << std::endl;
	if (i.getReturn().empty() == false)
	{
		std::map<int, std::string>::const_iterator itr;
		for (itr = i.getReturn().begin(); itr != i.getReturn().end(); itr++)
			o << "    return			=	[" << itr->first << " ; " << itr->second << "]" << std::endl;
	}
	if (i.getAllowMethod().empty() == false)
		o << "    methods			=	[" << i.getAllowMethod() << "]" << std::endl;
	if (i.getUploadDir().empty() == false)
		o << "    uploadDir			=	[" << i.getUploadDir() << "]" << std::endl;
	if (i.getCgi().empty() == false)
	{
		std::map<std::string, std::string>::const_iterator ite;
		for (ite = i.getCgi().begin(); ite != i.getCgi().end(); ite++)
			o << "    cgi				=	[" << ite->first << " ; " << ite->second << "]" << std::endl;
	}
	if (i.getClientBodySize() > 0)
		o << "    clientMaxBodySize		=	[" << i.getClientBodySize() << "]" << std::endl;
	if (i.getError().empty() == false)
		o << "    errorPage			=	[" << i.getError() << "]" << std::endl;
	return (o);
};


bool	operator<(const Location & l1, const Location & l2)
{
	return (l1.getUri().size() < l2.getUri().size());
}