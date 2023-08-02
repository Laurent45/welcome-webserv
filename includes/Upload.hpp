/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Upload.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfrederi <lfrederi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/01 15:58:31 by lfrederi          #+#    #+#             */
/*   Updated: 2023/08/02 21:38:39 by lfrederi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UPLOAD_HPP
#define UPLOAD_HPP

#include <string>
#include <vector>
#include <fstream>
#include <map>

class Upload
{

    private:
        std::string     _filePath;
        std::string     _bondary;

        std::string     _fileName;
        std::fstream    _file;
        bool            _uploading;
        bool            _finish;

        void    openUploadFile(std::map<std::string, std::string> const & headers);

    public:

        Upload(void);
        Upload(Upload const & copy);
        Upload & operator=(Upload const & rhs);
        ~Upload(void);

        bool    isUploading() const;
        void	setBondary(std::string const & bondary);
        void    setFilePath(std::string const & path);

        void    prepareUpload(std::vector<unsigned char> & data, size_t & bodySize);
        void    upload(std::vector<unsigned char> & data, size_t & bodySize);
        void    clear();
};

#endif