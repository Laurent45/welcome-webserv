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

#define BUFFER_SIZE		128
#define	TIMEOUT			500000LL

class Client : public AFileDescriptor
{
	private:
		long long						_startTime;

		std::vector<unsigned char>		_inputData;
		std::vector<unsigned char>		_outputData;

		Server const * const			_server;
		ServerConf const * 				_serverConf; 
		Location const *				_location;
		std::string						_correctPathRequest;

		Request							_request;
		Cgi 							_cgi;

		bool							_responseReady;
		bool							_callCgi;
		bool							_close;

		Client(void);

		void				handleRequest();

		void				getCorrectServerConf();
		void				getCorrectLocationBlock();
		void				getCorrectPathRequest();
		std::string 		searchIndexFile(std::string path, std::vector<std::string> const &indexs, bool autoindex);

		void				handleScript(std::string const & fullPath);

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

		bool	timeoutReached();
		void	fillRawData(std::vector<unsigned char> const & data);
		void	readyToRespond();
		void	handleException(std::exception const & exception);
		void	closeClient();
		void	prepareToNextRequest();
};

#endif
