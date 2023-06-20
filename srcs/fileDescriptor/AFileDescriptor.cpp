/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AFileDescriptor.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfrederi <lfrederi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/18 15:40:16 by lfrederi          #+#    #+#             */
/*   Updated: 2023/06/14 17:23:08 by lfrederi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "AFileDescriptor.hpp"

/*****************
 * CANNONICAL FORM
 *****************/

AFileDescriptor::AFileDescriptor(void)
{}

AFileDescriptor::AFileDescriptor(AFileDescriptor const &copy)
	: _fd(copy._fd), _epollFd(copy._epollFd)
{}

AFileDescriptor &AFileDescriptor::operator=(AFileDescriptor const &rhs)
{
	if (this != &rhs)
	{
		_epollFd = rhs._epollFd;
		_fd = rhs._fd;
	}
	return (*this);
}

AFileDescriptor::~AFileDescriptor()
{}

/******************************************************************************/

/**************
 * CONSTRUCTORS
 ***************/

AFileDescriptor::AFileDescriptor(int epollFd, int fd)
	: _fd(fd), _epollFd(epollFd)
{}

/******************************************************************************/

/************
 * ACCESSORS
 ************/

int AFileDescriptor::getFd() const
{
	return (_fd);
}

int AFileDescriptor::getEpollFd() const
{
	return (_epollFd);
}

void	AFileDescriptor::setFd(int fd)
{
	_fd = fd;
}

void	AFileDescriptor::setEpollFd(int epollFd)
{
	_epollFd = epollFd;
}
/******************************************************************************/
