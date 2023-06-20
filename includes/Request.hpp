/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfrederi <lfrederi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/18 18:20:19 by lfrederi          #+#    #+#             */
/*   Updated: 2023/06/11 20:45:21 by lfrederi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <map>
#include <vector>

#define REQUEST_COMPLETE	1
#define REQUEST_UNCOMPLETE	2
#define	CRLF		"\r\n"
#define CRLFCRLF	"\r\n\r\n"

class Request
{
	private:
		
		std::string							_httpMethod;
		std::string							_pathRequest;
		std::string							_fileName;
		std::string							_extension;
		std::string							_queryParam;
		std::string							_httpVersion;
		std::map<std::string, std::string>	_headers;
		std::string							_messageBody;
		bool								_hasMessageBody;


	public:
		
		static int const	requestUncomplete;
		static int const	requestComplete;

		// Cannonical Form
		Request();
		Request(Request const & copy);
		Request & operator=(Request const & rhs);
		~Request();

		// Geters
		std::string const & getHttpMethod() const;
		std::string const & getPathRequest() const;
		std::string const &	getFileName() const;
		std::string	const & getExtension() const;
		std::string const & getQueryParam() const;
		std::string const & getHttpVersion() const;
		std::map<std::string, std::string> const & getHeaders() const;
		std::string const & getMessageBody() const;
		bool				hasMessageBody() const;

		// Seters
		void	setHttpMethod(std::string const & httpMethod);
		void	setPathRequest(std::string const & pathRequest);
		void	setHttpVersion(std::string const & httpVersion);
		void	setHeaders(std::map<std::string, std::string> const & headers);
		void	setMessageBody(std::string const & messageBody);

		// Members methods
		bool	handleRequestLine(std::string requestLine);
		bool	handleHeaders(std::string headers);
		bool	handleMessageBody(std::vector<unsigned char> messageBody);

};

#endif
