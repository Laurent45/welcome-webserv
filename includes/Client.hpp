/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfrederi <lfrederi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/18 16:02:13 by lfrederi          #+#    #+#             */
/*   Updated: 2023/06/19 20:13:42 by lfrederi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "AFileDescriptor.hpp"
#include "HttpUtils.hpp"
#include "Request.hpp"
#include "Server.hpp"
#include "Cgi.hpp"

#define BUFFER_SIZE		1024

class Client : public AFileDescriptor
{
	private:
		std::vector<unsigned char>		_rawData;
		std::vector<ServerConf> 		_serverInfo;
		ServerConf 						_serverInfoCurr; 
		Request							_request;
		bool							_responseReady;
		Cgi 							_cgi;

		Client(void);

		ServerConf			getCorrectServer();
		void				handleScript(std::string const & fullPath);
		Location const *	getLocationBlock();
		void				handleRequest(Location const * location);
		std::string 		searchIndexFile(std::string path, std::vector<std::string> const &indexs, bool autoindex);

	public:
		
		Client(Client const & copy);
		Client & operator=(Client const & rhs);
		virtual ~Client();

		// Constructors
		Client(int fd, WebServ & webServ, std::vector<ServerConf> const & serverInfo);

		// Geters
		Request const & getRequest() const;
		ServerConf const &	getServerInfo() const;

		// Public methods
		virtual void doOnRead();
		virtual void doOnWrite();
		virtual void doOnError(uint32_t event);

		void	errorResponse(status_code_t status);
		void	getResponse();
		void	responseCgi(std::vector<unsigned char> const & cgiRawData);
		void	handleException(std::exception const & exception);
};

#endif
