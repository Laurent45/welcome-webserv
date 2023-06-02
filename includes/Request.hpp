/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfrederi <lfrederi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/18 18:20:19 by lfrederi          #+#    #+#             */
/*   Updated: 2023/06/02 21:30:29 by lfrederi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <map>
#include <vector>

#define REQUEST_COMPLETE	1
#define REQUEST_UNCOMPLETE	2

class Request
{
	private:
		
		std::string							_httpMethod;
		std::string							_pathRequest;
		std::string							_httpVersion;
		std::map<std::string, std::string>	_headers;
		std::vector<std::string>			requestline_extracted;
		std::string							_messageBody;
		bool								_hasMessageBody;
		int									status_code_so_far;
		std::vector<std::string>			authorised_methods;


	public:
		
		static int const	requestUncomplete;
		static int const	requestComplete;

		// Cannonical Form
		Request();
		Request(const std::string &buffer);
		Request(Request const & copy);
		Request & operator=(Request const & rhs);
		~Request();

		// Geters
		std::string const & getHttpMethod() const;
		std::string const & getPathRequest() const;
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
		void	setStatusCode(int code);

		// Members methods
		// void	fillHeaders(std::string const & data);
		// void	fillMessageBody(std::string const & data);
		// bool	isMessageBodyTerminated();

		bool		create_request(const std::string &buffer);
		void		parseRawRequest(const std::string& request);
		bool		isRequestLineAccepted(void);
		void		init_authorised_methods(void);
		void 		print_Pending_Request(void);
		void		init_authorised_methods(std::vector<std::string> authorised_methods);
		void 		logRequest(const Request &request);
		bool		check_if_token_is_accepted(void);
		void		fill_from_request_line(void);

};

#endif
