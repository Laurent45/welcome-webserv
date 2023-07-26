/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StringUtils.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eantoine <eantoine@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/28 16:08:56 by lfrederi          #+#    #+#             */
/*   Updated: 2023/07/24 00:55:57 by eantoine         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "StringUtils.hpp"
#include <sstream> // stringstream
#include <algorithm> // replace if
#include <stack>

/*****************
* CANNONICAL FORM
*****************/

StringUtils::StringUtils()
{}

StringUtils::StringUtils(StringUtils const & copy)
{
    (void) copy;
}

StringUtils & StringUtils::operator=(StringUtils const & rhs)
{
    (void) rhs;
    return (*this);
}

StringUtils::~StringUtils()
{}
/******************************************************************************/

/****************
* STATIC METHODS
****************/

/**
 * @brief trim line from # until the end
 * 
 * @param str 
 * @return std::string 
 */
std::string		StringUtils::trimComments(const std::string &str) {
	std::string new_line;
	size_t hash_char_pos;

	new_line = str;
	hash_char_pos = new_line.find('#');
	if (hash_char_pos == std::string::npos)
		return new_line;
	return new_line.substr(0, hash_char_pos);
}

/**
 * @brief Delete extra whitespaces if more than one
 * 
 * @param str 
 * @return std::string 
 */
std::string		StringUtils::replaceConsecutiveSpaces(const std::string& str) 
{
    std::stringstream ss;
    bool previousCharIsSpace = false;
    for (std::string::const_iterator it = str.begin(); it!=str.end(); ++it) 
	{
        char c = *it;
        if ( !isspace(c) || !previousCharIsSpace)
            ss << c;
        previousCharIsSpace = isspace(c);
    }
    return ss.str();
}

/**
 * @brief Trim whitespaces at beginning, at end of line, 
 *	and replaces chain of whitespaces by a single space
 *	return : std::string
 * 
 * @param str 
 * @return std::string 
 */
std::string		StringUtils::trimWhitespaces(const std::string& str)
{
	std::string whitespaces;
	std::string new_line;
	size_t start, end;

	whitespaces = WHITESPACES;
	new_line = str;
	start = new_line.find_first_not_of(whitespaces);
	if (start != std::string::npos)
		new_line = new_line.substr(start);
	end = new_line.find_last_not_of(whitespaces);
	if (end != std::string::npos)
		new_line = new_line.substr(0, end + 1);
	new_line = replaceConsecutiveSpaces(new_line);
	std::replace_if(new_line.begin(), new_line.end(), isspace, ' ');
	return new_line;
}

/**
 * @brief gets a line into a line filles with "\n"
 * given a line number
 * @param str 
 * @param n quelle ligne?
 * @return std::string 
 */
std::string		StringUtils::getLine(std::string str, size_t n)
{
	int i = 0;
	size_t j = 0;
	size_t ct = 0;
	std::string temp;

	if (n >= nbLines(str))
		return std::string();
	while (ct < n)
	{
		if (str[i++] =='\n')
			ct++;
	}
	while (std::isspace(str[i]) && str[i] != '\n')
		++i;
	while (str[i + j] && str[i + j] != '\n')
		j++;
	while (j > 0 && std::isspace(str[i + j - 1]))
		--j;
	temp = std::string(str, i, j);
	return (temp);
}

/**
 * @brief return  number of lines in the string (\n separated)
 * 
 * @param str_config 
 * @return size_t 
 */
size_t			StringUtils::nbLines(std::string &str_config)
{
	size_t lines = 1;
	for (std::string::iterator it = str_config.begin(); it!=str_config.end(); it++)
		if (*it == '\n')
			lines++;
	return (lines);
}

/**
 * @brief Check if the line from conf file is of interest 
 * ie size > 0 and not }
 * @param str 
 * @param pos 
 * @return true 
 * @return false 
 */
bool   StringUtils::isNothing(std::string str, int pos)
{
	std::string line;

	line = getLine(str, pos);
	return (line.size() == 0 || line[0] == '}');
}

/**
 * @brief Check if brackets opened/closed is correct
 * 
 * @param config_string 
 * @return true 
 * @return false 
 */
bool    StringUtils::checkBrackets(std::string &config_string)
{
	std::stack<char>	bracket;
	std::string::iterator ite;
	ite = config_string.begin();
	for (;ite != config_string.end();ite++)
	{
		if (*ite == '{')
			bracket.push(*ite);
		if (*ite == '}')
		{
			if (bracket.empty() || bracket.top() != '{')
				return false;
			if (!bracket.empty() && bracket.top() == '{')
				bracket.pop();
		}
	}
	if (bracket.empty())
		return true;
	return false;
}

/**
 * @brief gets a line and returns a line, trimming the first i lines
 * 
 * @param string 
 * @param position
 * @return std::string 
 */
std::string 	StringUtils::trimLineToI(std::string &str, size_t pos)
{
	int i = 0;
	size_t ct = 0;
	std::string temp;

	if (pos >= nbLines(str))
		return temp;
	while (ct < pos)
	{
		if (str[i++] =='\n')
			ct++;
	}
	temp = std::string(str, i);
	return (temp);
}

/**
 * @brief Finds the closing bracket line in the 
 * string where the first opening bracket has already been
 * removed: needs to checked pairs of included brackets
 * 
 * @param str 
 * @return int 
 */
int				StringUtils::findClosingBracket(std::string str)
{
	int line = 1;
	int count = 1;
	for (size_t i = 0; i < nbLines(str); i++)
	{
		if (getLine(str, i).find("{")!= std::string::npos)
			count +=1;
		if (getLine(str, i).find("}")!= std::string::npos)
		{
			if ((count -=1) == 0)
				return line;
		}
		line ++;
	}
	return (-1);
}

/**
 * @brief splits a string around a character in the given charset
 * 
 * @param str 
 * @param charset 
 * @return std::vector<std::string> 
 */
std::vector<std::string> StringUtils::splitString(std::string str, const std::string &charset)
{
	std::vector<std::string> tokens;
    size_t pos = 0;
    std::string token;

    while ((pos = str.find_first_of(charset)) != std::string::npos)
	{
        token = str.substr(0, pos);
        if(token.compare(""))
			tokens.push_back(token);
        str.erase(0, pos + 1);
    }
    tokens.push_back(str);
    return tokens;
}

/**
 * @brief 
 * @param num 
 * @return 
 */
std::string StringUtils::intToString(int num)
{
  std::stringstream ss;
  ss << num;
  return ss.str();
}

void	StringUtils::addCwd(std::string &path)
{
	if(!path.compare(0, 2, "./"))
	{
		char wd[FILENAME_MAX];
		if(getcwd(wd, sizeof(wd)) != NULL)
		{
			std::string cwd(wd);
			path = cwd + path.erase(0,1);
		} 
		else {
			std::cerr << "Error getting current directory." << std::endl;
		}
	}
}
