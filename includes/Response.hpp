/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfrederi <lfrederi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/03 19:19:08 by lfrederi          #+#    #+#             */
/*   Updated: 2023/08/01 10:00:10 by lfrederi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "HttpUtils.hpp"

class Client;

class Response
{
    private:

        static std::string  commonResponse(status_code_t status);
        static std::string  bodyHeaders(std::string extension, unsigned int size);

    public:
  
        static void cgiResponse(Client & client, std::string headers,
                                            std::vector<unsigned char> & body);
        static void errorResponse(status_code_t code, Client & client);

};




#endif