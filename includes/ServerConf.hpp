/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConf.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfrederi <lfrederi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/23 18:12:31 by lfrederi          #+#    #+#             */
/*   Updated: 2023/06/19 18:20:52 by lfrederi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_CONF_HPP
#define SERVER_CONF_HPP

#include "Location.hpp"
#include "AFileDescriptor.hpp"
#include <string>
#include <vector>
#include <map>

#define MAX_CLIENT 10 

class ServerConf
{
	private:

		std::string							_root;
		int									_port;
		std::string							_server_name;
		std::string							_IP;
		std::string							_error_pages;
		std::vector<std::string>			_index;
		bool								_autoindex;
		long int							_client_body_size;
		std::map<std::string, std::string>	_cgi;
		std::vector<Location>				_location;


		void						setCgi(std::vector<std::string> token);
		void						setPort(std::vector<std::string> token);
		void						setRoot(std::vector<std::string> token);
		void						setIp(std::vector<std::string> token);
		void						setIndex(std::vector<std::string> token);
		void						setAutoindex(std::vector<std::string> token);
		void						setName(std::vector<std::string> token);
		void						setError(std::vector<std::string> token);
		void						setClientBodySize(std::vector<std::string> token);

		int							skipLocationBlock(std::string str, int count);
		void						parseServerConf(std::string str, int &count);
		int							getLocationBloc(std::string str, int &count);

		typedef void (ServerConf::*ServerConf_func)(std::vector<std::string>);
		void init_vector_ServerConf_fct(std::vector<ServerConf_func> &funcs);


	public:
		ServerConf(void);
		ServerConf(ServerConf const &src);
		ServerConf &operator=(ServerConf const &src);
		virtual ~ServerConf();

		void						addLocation(std::string str, int &count, int &ServerConf_ct);

		/*
		 ** ServerConf getters
		 */
		int							const &getPort() const;
		std::string					const &getRoot() const;
		std::string					const &getName() const;
		std::string					const &getIp() const;
		std::string					const &getError() const;
		std::vector<std::string>	const &getIndex() const;
		bool						const &getAutoindex() const;
		long int					const &getClientBodySize() const;
		std::vector<Location>		const &getLocation() const;
		std::map<std::string, std::string>	const &getCgi() const;

		void						setServerConf(const std::string &str);
		void 						cleanDupServerConf(std::vector<ServerConf> ServerConfInfo);

};

std::ostream    &operator<<(std::ostream &o, ServerConf const &i);
std::ostream    &operator<<(std::ostream &o, std::vector<ServerConf>  const &srv);
std::ostream    &operator<<(std::ostream &o, std::vector<std::string>  const &str);
#endif
