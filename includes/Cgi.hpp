/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfrederi <lfrederi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/03 23:39:09 by lfrederi          #+#    #+#             */
/*   Updated: 2023/07/31 17:01:39 by lfrederi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_HPP
#define CGI_HPP

#include "AFileDescriptor.hpp"
#include <vector>
#include <string>

class Client;

class Cgi : public AFileDescriptor
{
    private:

        std::vector<unsigned char>	_rawData;
        Client *                    _clientInfo;
        std::string                 _fullPath;
        int                         _fdRead;
        int                         _fdWrite;
        int                         _pidChild; // Can remove

        int         initChildProcess(int toCgi[2], int fromCgi[2]);
        void        runChildProcess(int pipeToCgi[2], int pipeFromCgi[2]);        
        char **     mapCgiParams(std::string const & script);
        void        processCgiResponse();

    public:
        
        Cgi(void);
		Cgi(Cgi const & copy);
		Cgi & operator=(Cgi const & rhs);
		virtual ~Cgi();

        Cgi(WebServ & webServ, Client & client, std::string const & fullPath);

        int     getReadFd() const;
        int     getWriteFd() const;
        int     getPidChild() const;
        void    setReadFd(int fd);
        void    setWriteFd(int fd);

       	virtual void doOnRead();
		virtual void doOnWrite();
		virtual void doOnError(uint32_t event);

        int     run();

};

#endif