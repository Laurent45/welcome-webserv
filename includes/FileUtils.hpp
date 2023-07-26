/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileUtils.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eantoine <eantoine@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/29 20:56:47 by lfrederi          #+#    #+#             */
/*   Updated: 2023/07/25 12:44:26 by eantoine         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FILE_UTILS_HPP
#define FILE_UTILS_HPP

class FileUtils
{
    private:

        FileUtils();
		FileUtils(const FileUtils &copy);
		FileUtils &operator=(const FileUtils &copy);
		~FileUtils();

    public:

        static bool		fileExists(const char* path);
        static bool		fileRead(const char* path);
        static bool		fileExec(const char* path);
        static bool		isDirectory(const char* path);
		static bool 	folderExistsAndWritable(const char* folderPath);
};

#endif