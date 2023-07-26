/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpUtils.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eantoine <eantoine@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/29 21:37:33 by lfrederi          #+#    #+#             */
/*   Updated: 2023/07/25 17:46:22 by eantoine         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpUtils.hpp"

std::map<status_code_t, std::string> const	HttpUtils::RESPONSE_STATUS = initResponseStatus();
std::map<std::string, std::string> const	HttpUtils::MIME_TYPES = initMimeTypes();

/*****************
* CANNONICAL FORM
*****************/

HttpUtils::HttpUtils()
{}

HttpUtils::HttpUtils(HttpUtils const & copy)
{
    (void) copy;
}

HttpUtils & HttpUtils::operator=(HttpUtils const & rhs)
{
    (void) rhs;
    return (*this);
}

HttpUtils::~HttpUtils()
{}
/******************************************************************************/

/******************
* STATIC ATTRIBUTES
******************/

const HttpUtils::method_tab_entry_t HttpUtils::METHODS[] = 
{
	{GET, "GET"},
	{HEAD, "HEAD"},
	{POST, "POST"},
	{PUT, "PUT"},
	{DELETE, "DELETE"},
	{CONNECT, "CONNECT"},
	{OPTIONS, "OPTIONS"},
	{TRACE, "TRACE"},
};


const HttpUtils::request_header_tab_entry_t HttpUtils::REQUEST_HEADERS[] =
{
	{ACCEPT_CHARSET, "Accept-Charset"},
	{CONTENT_LENGTH, "Content-Length"},
	{AUTHORIZATION, "Authorization"},
	{CONTENT_TYPE, "Content-Type"},
	{DATE, "Date"},
	{HOST, "Host"},
	{REFERER, "Referer"},
	{TRANSFER_ENCODING, "Transfer-Encoding"},
	{USER_AGENT, "User-Agent"},
};

const HttpUtils::answer_header_tab_entry_t  HttpUtils::ANSWER_HEADERS[] =
{
	{ALLOW, "Allow"},
	{CONTENT_LANGUAGE, "Content-Language"},
	{A_CONTENT_LENGTH, "Content-Length"},
	{CONTENT_LOCATION, "Content-Location"},
	{A_CONTENT_TYPE, "Content-Type"},
	{A_DATE, "Date"},
	{LAST_MODIFIED, "Last-Modified"},
	{LOCATION, "Location"},
	{RETRY_AFTER, "Retry-After"},
	{SERVER, "Server"},
	{A_TRANSFER_ENCODING, "Transfer-Encoding"},
	{WWW_AUTHENTICATE, "WWW-Authenticate"},
};

std::map<status_code_t, std::string>	HttpUtils::initResponseStatus()
{
	std::map<status_code_t, std::string> map;

	map.insert(std::pair<status_code_t, std::string>(CONTINUE,"Continue"));
	map.insert(std::pair<status_code_t, std::string>(SWITCHING_PROTOCOLS,"Switching Protocols"));
	map.insert(std::pair<status_code_t, std::string>(OK,"OK"));
	map.insert(std::pair<status_code_t, std::string>(CREATED,"Created"));
	map.insert(std::pair<status_code_t, std::string>(ACCEPTED,"Accepted"));
	map.insert(std::pair<status_code_t, std::string>(NON_AUTHORITATIVE_INFORMATION,"Non-Authoritative Information"));
	map.insert(std::pair<status_code_t, std::string>(NO_CONTENT,"No Content"));
	map.insert(std::pair<status_code_t, std::string>(RESET_CONTENT,"Reset Content"));
	map.insert(std::pair<status_code_t, std::string>(PARTIAL_CONTENT,"Partial Content"));
	map.insert(std::pair<status_code_t, std::string>(MULTIPLE_CHOICES,"Multiple Choices"));
	map.insert(std::pair<status_code_t, std::string>(MOVED_PERMANENTLY,"Moved Permanently"));
	map.insert(std::pair<status_code_t, std::string>(FOUND,"Found"));
	map.insert(std::pair<status_code_t, std::string>(SEE_OTHER,"See Other"));
	map.insert(std::pair<status_code_t, std::string>(NOT_MODIFIED,"Not Modified"));
	map.insert(std::pair<status_code_t, std::string>(USE_PROXY,"Use Proxy"));
	map.insert(std::pair<status_code_t, std::string>(TEMPORARY_REDIRECT,"Temporary Redirect"));
	map.insert(std::pair<status_code_t, std::string>(BAD_REQUEST,"Bad Request"));
	map.insert(std::pair<status_code_t, std::string>(UNAUTHORIZED,"Unauthorized"));
	map.insert(std::pair<status_code_t, std::string>(PAYMENT_REQUIRED,"Payment Required"));
	map.insert(std::pair<status_code_t, std::string>(FORBIDDEN,"Forbidden"));
	map.insert(std::pair<status_code_t, std::string>(NOT_FOUND,"Not Found"));
	map.insert(std::pair<status_code_t, std::string>(METHOD_NOT_ALLOWED,"Method Not Allowed"));
	map.insert(std::pair<status_code_t, std::string>(NOT_ACCEPTABLE,"Not Acceptable"));
	map.insert(std::pair<status_code_t, std::string>(PROXY_AUTHENTICATION_REQUIRED,"Proxy Authentication Required"));
	map.insert(std::pair<status_code_t, std::string>(REQUEST_TIMEOUT,"Request Timeout"));
	map.insert(std::pair<status_code_t, std::string>(CONFLICT,"Conflict"));
	map.insert(std::pair<status_code_t, std::string>(GONE,"Gone"));
	map.insert(std::pair<status_code_t, std::string>(LENGTH_REQUIRED,"Length Required"));
	map.insert(std::pair<status_code_t, std::string>(PRECONDITION_FAILED,"Precondition Failed"));
	map.insert(std::pair<status_code_t, std::string>(PAYLOAD_TOO_LARGE,"Payload Too Large"));
	map.insert(std::pair<status_code_t, std::string>(URI_TOO_LONG,"URI Too Long"));
	map.insert(std::pair<status_code_t, std::string>(UNSUPPORTED_MEDIA_TYPE,"Unsupported Media Type"));
	map.insert(std::pair<status_code_t, std::string>(RANGE_NOT_SATISFIABLE,"Range Not Satisfiable"));
	map.insert(std::pair<status_code_t, std::string>(EXPECTATION_FAILED,"Expectation Failed"));
	map.insert(std::pair<status_code_t, std::string>(INTERNAL_SERVER_ERROR,"Internal Server Error"));
	map.insert(std::pair<status_code_t, std::string>(NOT_IMPLEMENTED,"Not Implemented"));
	map.insert(std::pair<status_code_t, std::string>(BAD_GATEWAY,"Bad Gateway"));
	map.insert(std::pair<status_code_t, std::string>(SERVICE_UNAVAILABLE,"Service Unavailable"));
	map.insert(std::pair<status_code_t, std::string>(GATEWAY_TIMEOUT,"Gateway Timeout"));
	map.insert(std::pair<status_code_t, std::string>(HTTP_VERSION_NOT_SUPPORTED,"HTTP Version Not Supported"));
	return map;
}

std::map<std::string, std::string>		HttpUtils::initMimeTypes()
{
	std::map<std::string, std::string> map;

	map.insert(std::pair<std::string, std::string>("aac", "audio/aac"));
	map.insert(std::pair<std::string, std::string>("abw", "application/x-abiword"));
	map.insert(std::pair<std::string, std::string>("arc", "application/octet-stream"));
	map.insert(std::pair<std::string, std::string>("avi", "video/x-msvideo"));
	map.insert(std::pair<std::string, std::string>("azw", "application/vnd.amazon.ebook"));
	map.insert(std::pair<std::string, std::string>("bin", "application/octet-stream"));
	map.insert(std::pair<std::string, std::string>("bmp", "image/bmp"));
	map.insert(std::pair<std::string, std::string>("bz", "application/x-bzip"));
	map.insert(std::pair<std::string, std::string>("bz2", "application/x-bzip2"));
	map.insert(std::pair<std::string, std::string>("csh", "application/x-csh"));
	map.insert(std::pair<std::string, std::string>("css", "text/css"));
	map.insert(std::pair<std::string, std::string>("csv", "text/csv"));
	map.insert(std::pair<std::string, std::string>("txt", "text/plain"));
	map.insert(std::pair<std::string, std::string>("doc", "application/msword"));
	map.insert(std::pair<std::string, std::string>("docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"));
	map.insert(std::pair<std::string, std::string>("eot", "application/vnd.ms-fontobject"));
	map.insert(std::pair<std::string, std::string>("epub", "application/epub+zip"));
	map.insert(std::pair<std::string, std::string>("gif", "image/gif"));
	map.insert(std::pair<std::string, std::string>("htm", "text/html"));
	map.insert(std::pair<std::string, std::string>("html", "text/html;charset=utf-8"));
	map.insert(std::pair<std::string, std::string>("ico", "image/x-icon"));
	map.insert(std::pair<std::string, std::string>("ics", "text/calendar"));
	map.insert(std::pair<std::string, std::string>("jar", "application/java-archive"));
	map.insert(std::pair<std::string, std::string>("jpeg", "image/jpeg"));
	map.insert(std::pair<std::string, std::string>("jpg", "image/jpeg"));
	map.insert(std::pair<std::string, std::string>("js", "application/javascript"));
	map.insert(std::pair<std::string, std::string>("json", "application/json"));
	map.insert(std::pair<std::string, std::string>("mid", "audio/midi"));
	map.insert(std::pair<std::string, std::string>("midi", "audio/midi"));
	map.insert(std::pair<std::string, std::string>("mpeg", "video/mpeg"));
	map.insert(std::pair<std::string, std::string>("mpkg", "application/vnd.apple.installer+xml"));
	map.insert(std::pair<std::string, std::string>("odp", "application/vnd.oasis.opendocument.presentation"));
	map.insert(std::pair<std::string, std::string>("ods", "application/vnd.oasis.opendocument.spreadsheet"));
	map.insert(std::pair<std::string, std::string>("odt", "application/vnd.oasis.opendocument.text"));
	map.insert(std::pair<std::string, std::string>("oga", "audio/ogg"));
	map.insert(std::pair<std::string, std::string>("ogv", "video/ogg"));
	map.insert(std::pair<std::string, std::string>("ogx", "application/ogg"));
	map.insert(std::pair<std::string, std::string>("otf", "font/otf"));
	map.insert(std::pair<std::string, std::string>("png", "image/png"));
	map.insert(std::pair<std::string, std::string>("pdf", "application/pdf"));
	map.insert(std::pair<std::string, std::string>("ppt", "application/vnd.ms-powerpoint"));
	map.insert(std::pair<std::string, std::string>("pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation"));
	map.insert(std::pair<std::string, std::string>("rar", "application/x-rar-compressed"));
	map.insert(std::pair<std::string, std::string>("rtf", "application/rtf"));
	map.insert(std::pair<std::string, std::string>("sh", "application/x-sh"));
	map.insert(std::pair<std::string, std::string>("svg", "image/svg+xml"));
	map.insert(std::pair<std::string, std::string>("swf", "application/x-shockwave-flash"));
	map.insert(std::pair<std::string, std::string>("tar", "application/x-tar"));
	map.insert(std::pair<std::string, std::string>("tif", "image/tiff"));
	map.insert(std::pair<std::string, std::string>("tiff", "image/tiff"));
	map.insert(std::pair<std::string, std::string>("ts", "application/typescript"));
	map.insert(std::pair<std::string, std::string>("ttf", "font/ttf"));
	map.insert(std::pair<std::string, std::string>("vsd", "application/vnd.visio"));
	map.insert(std::pair<std::string, std::string>("wav", "audio/x-wav"));
	map.insert(std::pair<std::string, std::string>("weba", "audio/webm"));
	map.insert(std::pair<std::string, std::string>("webm", "video/webm"));
	map.insert(std::pair<std::string, std::string>("webp", "image/webp"));
	map.insert(std::pair<std::string, std::string>("woff", "font/woff"));
	map.insert(std::pair<std::string, std::string>("woff2", "font/woff2"));
	map.insert(std::pair<std::string, std::string>("xhtml", "application/xhtml+xml"));
	map.insert(std::pair<std::string, std::string>("xls", "application/vnd.ms-excel"));
	map.insert(std::pair<std::string, std::string>("xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"));
	map.insert(std::pair<std::string, std::string>("xml", "application/xml"));
	map.insert(std::pair<std::string, std::string>("xul", "application/vnd.mozilla.xul+xml"));
	map.insert(std::pair<std::string, std::string>("zip", "application/zip"));
	map.insert(std::pair<std::string, std::string>("3gp", "video/3gpp"));
	map.insert(std::pair<std::string, std::string>("3g2", "video/3gpp2"));
	map.insert(std::pair<std::string, std::string>("7z", "application/x-7z-compressed"));
	return map;
}

/**
 * @brief
 * @param token
 * @return
 */
int		HttpUtils::correctMethodInstruction(std::vector<std::string> token)
{
	size_t i = 0;
	size_t j = 1;
	int valid;
	token[token.size() - 1] = token[token.size() - 1].erase(token[token.size() - 1].size() - 1);
	while (j < token.size())
	{
		i = 0;
		valid = 0;
		while (i < TOTAL_METHODS_INSTRUCTIONS)
		{
			if (!token[j].compare(HttpUtils::METHODS[i].name))
				valid = 1;
			i++;
		}
		if (valid == 0)
			return (-1);
		j++;
	}
	return 1;
}


std::pair<status_code_t, std::string>	HttpUtils::getResponseStatus(status_code_t statusCode)
{
	std::map<status_code_t, std::string>::const_iterator it = 
												HttpUtils::RESPONSE_STATUS.find(statusCode);
	if (it != HttpUtils::RESPONSE_STATUS.end())
		return *it;
	return std::make_pair(INTERNAL_SERVER_ERROR, "Internal Server Error");
}


std::string		HttpUtils::getMimeType(std::string extension)
{
	std::map<std::string, std::string>::const_iterator it = 
												HttpUtils::MIME_TYPES.find(extension);
	if (it != HttpUtils::MIME_TYPES.end())
		return it->second;
	return "application/octet-stream";
}