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
#define	TIMEOUT			5000LL

class Client : public AFileDescriptor
{
	private:
		long long						_startTime;
		std::vector<unsigned char>		_rawData;
		Server const * const			_server;
		ServerConf const * 				_serverConf; 
		Location const *				_location;
		Request							_request;
		std::string						_correctPathRequest;
		bool							_responseReady;
		Cgi 							_cgi;

		Client(void);

		ServerConf const *	getCorrectServerConf();
		void				handleScript(std::string const & fullPath);
		void				getCorrectLocationBlock();
		void				getCorrectPathRequest();
		std::string 		searchIndexFile(std::string path, std::vector<std::string> const &indexs, bool autoindex);

	public:
		
		Client(Client const & copy);
		Client & operator=(Client const & rhs);
		virtual ~Client();

		// Constructors
		Client(int fd, WebServ & webServ, Server const * server);

		// Geters
		Request const &		getRequest() const;
		ServerConf const *	getServerConf() const;

		// Public methods
		virtual void doOnRead();
		virtual void doOnWrite();
		virtual void doOnError(uint32_t event);

		void	responseCgi(std::vector<unsigned char> const & cgiRawData);
		bool	timeoutReached();
		void	fillRawData(std::vector<unsigned char> const & data);
		void	readyToRespond();
		void	handleException(std::exception const & exception);
};

#endif
