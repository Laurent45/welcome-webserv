/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpUtils.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfrederi <lfrederi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/29 21:35:05 by lfrederi          #+#    #+#             */
/*   Updated: 2023/08/02 18:38:24 by lfrederi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_UTILS_HPP
#define HTTP_UTILS_HPP

#include <string>
#include <map>
#include <vector>

enum method_t {
	GET,
	POST,
	DELETE,
	TOTAL_METHODS_INSTRUCTIONS
};

enum request_header_name_t {
	ACCEPT_CHARSET,
	CONTENT_LENGTH,
	AUTHORIZATION,
	CONTENT_TYPE,
	DATE,
	HOST,
	REFERER,
	TRANSFER_ENCODING,
	USER_AGENT
};

enum answer_header_name_t {
	ALLOW,
	CONTENT_LANGUAGE,
	A_CONTENT_LENGTH,
	CONTENT_LOCATION,
	A_CONTENT_TYPE,
	A_DATE,
	LAST_MODIFIED,
	LOCATION,
	RETRY_AFTER,
	SERVER,
	A_TRANSFER_ENCODING,
	WWW_AUTHENTICATE
};

enum status_code_t {
	UNDEFINED,
	CONTINUE = 100,
	SWITCHING_PROTOCOLS,
	OK = 200,
	CREATED,
	ACCEPTED,
	NON_AUTHORITATIVE_INFORMATION,
	NO_CONTENT,
	RESET_CONTENT,
	PARTIAL_CONTENT,
	MULTIPLE_CHOICES = 300,
	MOVED_PERMANENTLY,
	FOUND,
	SEE_OTHER,
	NOT_MODIFIED,
	USE_PROXY,
	TEMPORARY_RETURN = 307,
	BAD_REQUEST = 400,
	UNAUTHORIZED,
	PAYMENT_REQUIRED,
	FORBIDDEN,
	NOT_FOUND,
	METHOD_NOT_ALLOWED,
	NOT_ACCEPTABLE,
	PROXY_AUTHENTICATION_REQUIRED,
	REQUEST_TIMEOUT,
	CONFLICT,
	GONE,
	LENGTH_REQUIRED,
	PRECONDITION_FAILED,
	PAYLOAD_TOO_LARGE,
	URI_TOO_LONG,
	UNSUPPORTED_MEDIA_TYPE,
	RANGE_NOT_SATISFIABLE,
	EXPECTATION_FAILED,
	INTERNAL_SERVER_ERROR = 500,
	NOT_IMPLEMENTED,
	BAD_GATEWAY,
	SERVICE_UNAVAILABLE,
	GATEWAY_TIMEOUT,
	HTTP_VERSION_NOT_SUPPORTED
};

class HttpUtils
{
    private:

        HttpUtils();
		HttpUtils(const HttpUtils &copy);
		HttpUtils &operator=(const HttpUtils &copy);
		~HttpUtils();

		static const std::map<status_code_t, std::string> 	RESPONSE_STATUS;
		static const std::map<std::string, std::string>		MIME_TYPES;

		static std::map<status_code_t, std::string>	initResponseStatus();
		static std::map<std::string, std::string> 	initMimeTypes();

    public:

		static const unsigned char *						CRLF;
		static const unsigned char *						CRLFCRLF;

		struct method_tab_entry_t 
		{
			method_t		method_index;
			std::string		name;
		};

		struct request_header_tab_entry_t {
			request_header_name_t	header_index;
			std::string             name;
		};

		struct answer_header_tab_entry_t {
			answer_header_name_t	header_index;
			std::string             name;
		};

		static const method_tab_entry_t						METHODS[];
		static const request_header_tab_entry_t				REQUEST_HEADERS[];
		static const answer_header_tab_entry_t				ANSWER_HEADERS[];



		static int 	correctMethodInstruction(std::vector<std::string> token);

		static std::pair<status_code_t, std::string>
											getResponseStatus(status_code_t statusCode);
		static std::string	getMimeType(std::string extension);
};

#endif