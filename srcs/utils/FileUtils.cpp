/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileUtils.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eantoine <eantoine@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/29 21:02:34 by lfrederi          #+#    #+#             */
/*   Updated: 2023/07/31 11:27:37 by eantoine         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "FileUtils.hpp"
#include <sys/stat.h> // stat
#include <unistd.h> // access

/*****************
* CANNONICAL FORM
*****************/

FileUtils::FileUtils()
{}

FileUtils::FileUtils(FileUtils const & copy)
{
    (void) copy;
}

FileUtils & FileUtils::operator=(FileUtils const & rhs)
{
    (void) rhs;
    return (*this);
}

FileUtils::~FileUtils()
{}
/******************************************************************************/

/****************
* STATIC METHODS
****************/

bool    FileUtils::fileExists(const char* path) {
    return access(path, F_OK) != -1;
}

bool    FileUtils::fileRead(const char* path) {
    return access(path, R_OK) != -1;
}

bool    FileUtils::fileExec(const char* path) {
    return access(path, X_OK) != -1;
}

bool    FileUtils::isDirectory(const char* path)
{
    struct stat statbuf;
    if (stat(path, &statbuf) != 0) {
        return false;
    }
    return S_ISDIR(statbuf.st_mode);
}


bool FileUtils::folderExistsAndWritable(const char* folderPath) {
    struct stat st;
    if (stat(folderPath, &st) == 0 && S_ISDIR(st.st_mode)) {
        return access(folderPath, W_OK) == 0;
    }
    return false;
}
