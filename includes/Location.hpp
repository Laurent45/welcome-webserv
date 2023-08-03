/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfrederi <lfrederi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/23 18:18:12 by lfrederi          #+#    #+#             */
/*   Updated: 2023/08/02 10:23:08 by lfrederi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <string>
#include <map>
#include <vector>
#include <sys/stat.h>

enum location_instruction_t {
	L_ROOT,
	METHODS,
	L_INDEX,
	L_CGI,
	L_AUTOINDEX,
	UPLOAD_DIR,
	RETURN,
	L_CLIENT_MAX_BODY_SIZE,
	L_ERROR_PAGE,
	URI,
	TOTAL_LOCATION_INSTRUCTIONS
};

class Location
{

	private:
		int									_port;
		std::string							_locRoot;
		std::vector<std::string>			_allow_method;
		std::vector<std::string>			_index;
		std::map<std::string, std::string>	_cgi;
		bool								_autoindex;
		std::string							_upload_dir;
		std::map<int, std::string>			_return;
		size_t								_client_body_size;
		std::string							_error_pages;
		std::string							_uri;


		/*
		 ** Location setters
		 */
		void	setUri(std::vector<std::string> token);
		void	setAutoindex(std::vector<std::string> token);
		void	setIndex(std::vector<std::string> token);
		void	setReturn(std::vector<std::string> token);
		void	setAllowMethod(std::vector<std::string> token);
		void	setLocRoot(std::vector<std::string> token);
		void	setUploadDir(std::vector<std::string> token);
		void	setCgi(std::vector<std::string> token);
		void	setClientBodySize(std::vector<std::string> token);
		void	setErrorPages(std::vector<std::string> token);
		typedef void (Location::*loc_func)(std::vector<std::string> );
		void init_vector_loc_fct(std::vector<loc_func> &funcs);


	public:
		Location();
		Location(const Location &copy);
		Location &operator=(const Location &copy);
		~Location();

		Location(int port, std::map<std::string, std::string>	cgi, bool autoindex,
			std::vector<std::string> index, std::string locRoot, int client_body_size, std::string error_pages);
		Location(int port, std::map<std::string, std::string>	cgi, bool autoindex,
			std::vector<std::string> index, std::string locRoot, int client_body_size, std::string error_pages, std::string uri);

		void	setLocation(const std::string &str,  int &count, int &flag);
		void	parseLocation(std::string &line);

		/*
		 ** Location getters
		 */
		int									const &getLocIndex() const;
		int									const &getPort() const;
		std::string							const &getUri() const;
		bool								const &getAutoindex() const;
		std::vector<std::string>			const &getIndex() const;
		std::map<int, std::string>			const &getReturn() const;
		std::vector<std::string>			const &getAllowMethod() const;
		std::string							const &getLocRoot() const;
		std::string							const &getUploadDir() const;
		size_t								const &getClientBodySize() const;
		std::string							const &getError() const;
		std::map<std::string, std::string>	const &getCgi() const;
		
		struct location_instruction_tab_entry_t 
		{
			location_instruction_t	instruction_index;
			std::string 	name;
		};
		static const location_instruction_tab_entry_t 	LOCATION_INSTRUCTIONS[];
		static int	correctLocationInstruction(std::vector<std::string> token);

};

std::ostream    &operator<<(std::ostream &o, Location const &i);
bool	operator<(const Location & l1, const Location & l2);


#endif
