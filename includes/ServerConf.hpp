/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConf.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eantoine <eantoine@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/23 18:12:31 by lfrederi          #+#    #+#             */
/*   Updated: 2023/07/25 02:33:33 by eantoine         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_CONF_HPP
#define SERVER_CONF_HPP

#include "Location.hpp"
#include <string>
#include <vector>
#include <map>

#define MAX_CLIENT 10 

enum server_instruction_t {
	S_ROOT,
	LISTEN,
	SERVER_NAME,
	S_ERROR_PAGE,
	S_INDEX,
	S_AUTOINDEX,
	S_CLIENT_MAX_BODY_SIZE,
	S_CGI,
	LOCATION_INSTRUCTION,
	TOTAL_SERVER_INSTRUCTIONS
};

class ServerConf
{
	private:

		std::string							_root;
		int									_port;
		std::vector<std::string>			_server_name;
		std::string							_IP;
		std::string							_error_pages;
		std::vector<std::string>			_index;
		bool								_autoindex;
		long int							_client_body_size;
		std::map<std::string, std::string>	_cgi;
		std::vector<Location>				_location;
		std::string							_errorContent;
		std::string							_folderContent;


		void	setCgi(std::vector<std::string> token);
		void	setPort(std::vector<std::string> token);
		void	setRoot(std::vector<std::string> token);
		void	setIp(std::vector<std::string> token);
		void	setIndex(std::vector<std::string> token);
		void	setAutoindex(std::vector<std::string> token);
		void	setErrorContent(std::string  const path);
		void 	setFolderContent(std::string const path);
		
		void	setError(std::vector<std::string> token);
		void	setClientBodySize(std::vector<std::string> token);

		int		skipLocationBlock(std::string str, int count);
		void	parseServerConf(std::string str, int &count);
		int		getLocationBloc(std::string str, int &count);
		int		correctServerInstruction(std::vector<std::string> token);

		typedef void (ServerConf::*ServerConf_func)(std::vector<std::string>);
		void init_vector_ServerConf_fct(std::vector<ServerConf_func> &funcs);


	public:
		ServerConf(void);
		ServerConf(ServerConf const &src);
		ServerConf &operator=(ServerConf const &src);
		virtual ~ServerConf();


		/*
		 ** ServerConf getters
		 */
		int							const &getPort() const;
		std::string					const &getRoot() const;
		std::vector<std::string>	const &getName() const;
		std::string					const &getIp() const;
		std::string					const &getError() const;
		std::vector<std::string>	const &getIndex() const;
		bool						const &getAutoindex() const;
		long int					const &getClientBodySize() const;
		std::vector<Location>		const &getLocation() const;
		std::map<std::string, std::string>	const &getCgi() const;
		std::string 				const &getErrorContent() const;
		std::string 				const &getFolderContent() const;

		void	setServerConf(const std::string &str);
		void	addLocation(std::string str, int &count, int &flag);
		void 	cleanDupServerConf(std::vector<ServerConf> ServerConfInfo);
		void	sortLocationBlock();
		void	setName(std::vector<std::string> token);

		struct server_instruction_tab_entry_t
		{
			server_instruction_t	instruction_index;
			std::string 			name;
		};
		static const server_instruction_tab_entry_t		SERVER_INSTRUCTIONS[];

};

std::ostream    &operator<<(std::ostream &o, ServerConf const &i);
std::ostream    &operator<<(std::ostream &o, std::vector<ServerConf>  const &srv);
std::ostream    &operator<<(std::ostream &o, std::vector<std::string>  const &str);

#endif
