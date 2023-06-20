/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AFileDescriptor.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfrederi <lfrederi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/18 14:06:22 by lfrederi          #+#    #+#             */
/*   Updated: 2023/06/14 17:20:14 by lfrederi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef A_FILE_DESCRIPTOR_HPP
#define A_FILE_DESCRIPTOR_HPP

#include "AFileDescriptor.hpp"

#include <map>
#include <sys/epoll.h>

class AFileDescriptor
{
	protected:

		int		_fd;
		int		_epollFd;

		// Cannonical form
		AFileDescriptor();
		AFileDescriptor(AFileDescriptor const & copy);
		AFileDescriptor & operator=(AFileDescriptor const & rhs);

		// Constructors
		AFileDescriptor(int epollFd, int fd);

	public:
		virtual ~AFileDescriptor();

		// Accessors
		int		getFd() const;
		int		getEpollFd() const;
		void	setFd(int fd);
		void	setEpollFd(int epollFd);

		// Public methods
		virtual void doOnRead(std::map<int, AFileDescriptor *> & mapFd) = 0;
		virtual void doOnWrite(std::map<int, AFileDescriptor *> & mapFd) = 0;
		virtual void doOnError(std::map<int, AFileDescriptor *> & mapFd, uint32_t event) = 0;

};


#endif
