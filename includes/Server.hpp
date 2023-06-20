/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfrederi <lfrederi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/19 18:06:05 by lfrederi          #+#    #+#             */
/*   Updated: 2023/06/19 18:33:10 by lfrederi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "AFileDescriptor.hpp"
#include "ServerConf.hpp"
#include <vector>

#define MAX_CLIENT 10 

class Server : public AFileDescriptor
{
	private:

		std::vector<ServerConf> _serverConfs;

	public:
		Server(void);
		Server(Server const &src);
		Server &operator=(Server const &src);
		virtual ~Server();

		Server(std::vector<ServerConf> const & serverConfs);

		std::vector<ServerConf> const & getServerConfs() const;

		virtual void doOnRead(std::map<int, AFileDescriptor *> & mapFd);
		virtual void doOnWrite(std::map<int, AFileDescriptor *> & mapFd);
		virtual void doOnError(std::map<int, AFileDescriptor *> & mapFd, uint32_t event);

};

#endif
