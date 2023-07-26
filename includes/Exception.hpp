#ifndef EXCEPTION_HPP
# define EXCEPTION_HPP

#include "HttpUtils.hpp"
#include <exception>
#include <string>
#include <iostream>

/*
Conf File
*/

class WrongInputInServer : public std::exception
{
	public:
		char const	*what() const throw()
		{
			return "wrong input in server";
		}
};

class InvalidConfFilePath : public std::exception
{
	public:
		char const	*what() const throw()
		{
			return "Invalid Configuration File Path";
		}
};

class PathIsDir : public std::exception
{
	public:
		char const	*what() const throw()
		{
			return "Path is a directory, not a file";
		}
};
class EmptyConfPath: public std::exception
{
	public:
		char const	*what() const throw()
		{
			return "Configuration Path is empty";
		}
};

class CantOpenConfFile : public std::exception
{
	public:
		char const	*what() const throw()
		{
			return "Can't open the config file";
		}
};

class FileDoesNotExist : public std::exception
{
	public:
		char const	*what() const throw()
		{
			return "The file doesn't exist";
		}
};

class FileNotReadable : public std::exception
{
	public:
		char const	*what() const throw()
		{
			return "The file can't be read";
		}
};
class BadExtensionConfFile: public std::exception
{
	public:
		char const	*what() const throw()
		{
			return "Configuration File has a bad extension (needs to be .conf)";
		}
};

class InvalidConfFile : public std::exception
{
	public:
		char const	*what() const throw()
		{
			return "Invalid Configuration File";
		}
};

class ConfFileParseError : public std::exception
{
	private:
		std::string	_msg;

	public:
		ConfFileParseError(std::string msg)
		:	_msg(msg) {}

		~ConfFileParseError() throw() {};

		char const	*what() const throw()
		{
			std::cerr << "Configuration File Parse Error :" << std::endl;
			return _msg.c_str();
		}
};

class SetServerException : public std::exception
{
	private:
		std::string	_msg;

	public:
		SetServerException(std::string msg)
		:	_msg(msg) {}

		~SetServerException() throw() {};

		char const	*what() const throw()
		{
			std::cerr << "Setting Server Error :" << std::endl;
			return _msg.c_str();
		}
};

class RunServerException : public std::exception
{
	private:
		std::string	_msg;

	public:
		RunServerException(std::string msg)
		:	_msg(msg) {}

		~RunServerException() throw() {};

		char const	*what() const throw()
		{
			std::cerr << "Running Server Error :" << std::endl;
			return _msg.c_str();
		}
};

class epollFail : public std::exception
{
	private:
		std::string	_msg;

	public:
		epollFail(std::string msg)
		:	_msg(msg) {}

		~epollFail() throw() {};

		char const	*what() const throw()
		{
			std::cerr << "Epoll Error :" << std::endl;
			return _msg.c_str();
		}
};

class FatalError : public std::exception
{
	private:
		std::string	_msg;

	public:
		FatalError(std::string msg)
		:	_msg(msg) {}

		~FatalError() throw() {};

		char const	*what() const throw()
		{
			std::cerr << "Fatal Error :" << std::endl;
			return _msg.c_str();
		}
};

class EpollInitError : public std::exception
{
	private:
		std::string	_msg;

	public:
		EpollInitError(std::string msg)
		:	_msg(msg) {}

		~EpollInitError() throw() {};

		char const	*what() const throw()
		{
			std::cerr << "Epoll init Error :" << std::endl;
			return _msg.c_str();
		}
};

class RequestError : public std::exception
{
	private:
		status_code_t	_statusCode;
		std::string		_message;

	public:
		RequestError(status_code_t statusCode, std::string message)
		:	_statusCode(statusCode), _message(message) {}

		~RequestError() throw() {};

		char const	*what() const throw()
		{
			return "Request error";
		}

		status_code_t	getStatusCode() {return _statusCode;}
		std::string		getCause() {return HttpUtils::getResponseStatus(_statusCode).second + " (" + _message + ")";}
};

class FileDescriptorError : public std::exception
{
	private:
		std::string	_msg;

	public:
		FileDescriptorError(std::string msg)
		:	_msg(msg) {}

		~FileDescriptorError() throw() {};

		char const	*what() const throw()
		{
			std::cerr << "Error while handle file descriptor" << std::endl;
			return _msg.c_str();
		}
};

class RequestUncomplete : public std::exception
{
	public:
		char const	*what() const throw()
		{
			return "Request Uncomplete";
		}
};

#endif
