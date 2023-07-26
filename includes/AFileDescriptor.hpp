/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AFileDescriptor.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfrederi <lfrederi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/18 14:06:22 by lfrederi          #+#    #+#             */
/*   Updated: 2023/07/23 16:52:35 by lfrederi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef A_FILE_DESCRIPTOR_HPP
#define A_FILE_DESCRIPTOR_HPP

#include <sys/epoll.h>
class WebServ;

class AFileDescriptor
{
	protected:

		int			_fd;
		WebServ *	_webServ;

		// Cannonical form
		AFileDescriptor(void);
		AFileDescriptor(AFileDescriptor const & copy);
		AFileDescriptor & operator=(AFileDescriptor const & rhs);

		// Constructors
		AFileDescriptor(int fd, WebServ & webServ);

	public:
		virtual ~AFileDescriptor();

		// Accessors
		int			getFd() const;

		// Public methods
		virtual void doOnRead() = 0;
		virtual void doOnWrite() = 0;
		virtual void doOnError(uint32_t event) = 0;

};


#endif
