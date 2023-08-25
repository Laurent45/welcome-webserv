/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfrederi <lfrederi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/03 19:19:08 by lfrederi          #+#    #+#             */
/*   Updated: 2023/08/25 14:25:53 by lfrederi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "HttpUtils.hpp"

class Client;

class Response {

    private:

        static std::string  commonResponse(status_code_t status, bool alive);
        static std::string  bodyHeaders(std::string extension, unsigned int size);
        static void         getFileContent(std::string const & path, std::vector<unsigned char> & response);

    public:
        static void     getResponse(std::string const & path, Client & client);
        static void     cgiResponse(Client & client, std::string headers, std::vector<unsigned char> & body);
        static void     errorResponse(status_code_t code, Client & client);
		static void     deleteResponse(const std::string &path, Client & client);
        static void     postResponse(Client & client);
};

#endif