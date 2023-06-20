/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfrederi <lfrederi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/31 19:40:20 by lfrederi          #+#    #+#             */
/*   Updated: 2023/06/19 19:32:56 by lfrederi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include "ServerConf.hpp"
#include "WebServ.hpp"

class Parser
{
    private:

        std::string const                                               _configFile;
        std::map< std::pair<std::string, int>, std::vector<ServerConf> > _map;

        std::string     getStringConf() const;
        void            parseServers(std::vector<ServerConf> & servers, std::string strConfig);
        void            createServerSockets(WebServ & webServ) const;
		void            fillServersMap(std::vector<ServerConf> & serverConfs);

    public:

    	// Cannonical Form
		Parser();
		Parser(Parser const & copy);
		Parser & operator=(Parser const & rhs);
		~Parser();

        Parser(std::string configFile);

        void    parseConfFile(WebServ & webServ);
};

#endif