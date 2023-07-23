/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServ.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfrederi <lfrederi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/01 19:39:10 by lfrederi          #+#    #+#             */
/*   Updated: 2023/07/23 17:57:19 by lfrederi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEB_SERV_HPP
#define WEB_SERV_HPP

#include <map>
#include <sys/epoll.h>
#include "Server.hpp"
#include "AFileDescriptor.hpp"

#define MAX_EVENTS 1024

extern bool g_run;

class WebServ
{
    private:

        int                                     _epollFd;
        std::map<int, AFileDescriptor *>        _mapFd;
        std::vector<std::pair<int, long long> > _clientTimes;

        //void    handleTimeout();

    public:

        // Cannonical Form
		WebServ();
		WebServ(WebServ const & copy);
		WebServ & operator=(WebServ const & rhs);
		~WebServ();

        void    addFd(int fd, AFileDescriptor * server);
        void	eraseFd(int fd);
        void    removeFd(int fd);
        void    addClientTimes(std::pair<int, long long> clientInfo);
        void    epollInit();
        void	updateEpoll(int fd, u_int32_t event, int mod);
        void    start();

};

#endif