/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AFileDescriptor.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfrederi <lfrederi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/18 15:40:16 by lfrederi          #+#    #+#             */
/*   Updated: 2023/07/23 16:52:30 by lfrederi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "AFileDescriptor.hpp"
#include <cstddef>	// NULL

/*****************
 * CANNONICAL FORM
 *****************/

AFileDescriptor::AFileDescriptor(void) : _fd(-1), _webServ(NULL)
{}

AFileDescriptor::AFileDescriptor(AFileDescriptor const &copy)
	: _fd(copy._fd), _webServ(copy._webServ)
{}

AFileDescriptor &AFileDescriptor::operator=(AFileDescriptor const &rhs)
{
	if (this != &rhs)
	{
		_fd = rhs._fd;
		_webServ = rhs._webServ;
	}
	return (*this);
}

AFileDescriptor::~AFileDescriptor()
{}
/******************************************************************************/

/**************
 * CONSTRUCTORS
 ***************/

AFileDescriptor::AFileDescriptor(int fd, WebServ & webServ)
	: _fd(fd), _webServ(&webServ)
{}

/******************************************************************************/

/************
 * ACCESSORS
 ************/

int AFileDescriptor::getFd() const
{
	return (_fd);
}
/******************************************************************************/
