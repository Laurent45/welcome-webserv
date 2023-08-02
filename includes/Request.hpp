/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfrederi <lfrederi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/18 18:20:19 by lfrederi          #+#    #+#             */
/*   Updated: 2023/08/02 21:37:04 by lfrederi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "Upload.hpp"
#include <string>
#include <map>
#include <vector>

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
		std::vector<unsigned char>			_messageBody;
		bool								_hasMessageBody;
		bool								_encode;
		size_t								_bodySize;
		Upload								_upload;

	public:
		
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
		std::vector<unsigned char> const & getMessageBody() const;
		bool				hasMessageBody() const;
		bool				isEncoded() const;
		size_t				getBodySize() const;
		Upload &			getUpload();

		// Members methods
		void	handleRequestLine(std::vector<unsigned char> & rawData);
		void	handleHeaders(std::vector<unsigned char> & rawData);
		void	handleMessageBody(std::vector<unsigned char> & rawData);
		void	searchBondary();
		void	uploadFiles(std::vector<unsigned char> & rawData);

};

std::ostream    &operator<<(std::ostream & o, Request const & r);

#endif
