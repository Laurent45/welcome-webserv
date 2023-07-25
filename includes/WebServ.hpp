/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServ.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfrederi <lfrederi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/01 19:39:10 by lfrederi          #+#    #+#             */
/*   Updated: 2023/07/25 20:57:32 by lfrederi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEB_SERV_HPP
#define WEB_SERV_HPP

#include "Server.hpp"
#include "AFileDescriptor.hpp"

#include <map>
#include <vector>
#include <sys/epoll.h>

#define MAX_EVENTS 1024

extern bool g_run;

class Client;

class WebServ
{
    private:

        int                                     _epollFd;
        std::map<int, AFileDescriptor *>        _mapFd;
        std::vector<Client *>                   _clients;

    public:

        // Cannonical Form
		WebServ();
		WebServ(WebServ const & copy);
		WebServ & operator=(WebServ const & rhs);
		~WebServ();

        void    addFd(int fd, AFileDescriptor * server);
        void    addClient(Client * client);
        void	removeFd(int fd);
        void	removeClient(int fd);
        void    clearFd(int fd);
        void    epollInit();
        void	updateEpoll(int fd, u_int32_t event, int mod);
        void    start();

};

#endif