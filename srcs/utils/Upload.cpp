/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Upload.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lfrederi <lfrederi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/01 15:57:52 by lfrederi          #+#    #+#             */
/*   Updated: 2023/08/02 21:39:21 by lfrederi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Upload.hpp"
#include "Exception.hpp"
#include "StringUtils.hpp"
#include "HttpUtils.hpp"

#include <algorithm> /* search */

/*****************
 * CANNONICAL FORM
 *****************/

Upload::Upload(void) : _uploading(false), _finish(false)
{
}

Upload::Upload(Upload const & copy)
	: _filePath(copy._filePath),
	  _fileName(copy._fileName),
	  _uploading(copy._uploading),
	  _finish(copy._finish)
{
}

Upload	& Upload::operator=(Upload const &rhs)
{
	if (this != &rhs)
	{
		_filePath = rhs._filePath;
		_fileName = rhs._fileName;
		_uploading = rhs._uploading;
		_finish = rhs._finish;
	}

	return (*this);
}

Upload::~Upload() {
	if (_file.is_open())
		_file.close();
}
/******************************************************************************/

/**************
 * CONSTRUCTORS
 ***************/

/******************************************************************************/

/***********
 * ACCESSORS
 ************/

bool	Upload::isUploading() const {
	return _uploading;
}

void	Upload::setBondary(std::string const & bondary) {
	_bondary = bondary;
}

void	Upload::setFilePath(std::string const & filePath) {
	_filePath = filePath;
}
/******************************************************************************/

/****************
 * PUBLIC METHODS
 ****************/


void    Upload::prepareUpload(std::vector<unsigned char> & data, size_t & bodySize) {
    
	std::vector<unsigned char>::iterator ite;

	ite = std::search(data.begin(), data.end(), HttpUtils::CRLFCRLF, HttpUtils::CRLFCRLF + 4);
	if (ite == data.end())
		throw RequestUncomplete();

	std::map<std::string, std::string> heads;
	std::string headers(data.begin(), ite);
	std::vector<std::string> vec = StringUtils::splitString(headers, "\r\n");

	for (std::vector<std::string>::iterator it = vec.begin(); it != vec.end(); it++) {
		/* Check if header key is valid */
		size_t sep = (*it).find(":");
		if (sep == std::string::npos)
			continue;
		if (std::find_if((*it).begin(), (*it).begin() + sep, isblank) != (*it).begin() + sep)
			continue;

		std::string key = (*it).substr(0, sep);
		std::string value = StringUtils::trimWhitespaces((*it).substr(sep + 1));
		heads[key] = value;
	}

	bodySize -= &(*(ite + 4)) - &(*data.begin());
	data.erase(data.begin(), ite + 4);
	openUploadFile(heads);	
}


void	Upload::upload(std::vector<unsigned char> & data, size_t & bodySize) {

	// Search end of data for keep in range of body size
	std::vector<unsigned char>::iterator end = data.end();
	if (data.size() >= bodySize) {
		end = data.end() - (data.size() - bodySize);
		_finish = true;
	}

	unsigned char hyphen[] = {'-', '-'};
	std::vector<unsigned char>::iterator ref = data.begin();
	std::vector<unsigned char>::iterator it = std::search(ref, end, hyphen, hyphen + 2);

	while (it != end) {

		std::string tmp = std::string(it, end);
		if (tmp.size() < _bondary.size() && _finish == false)
			throw RequestUncomplete();

		if (tmp.compare(0, _bondary.size(), _bondary) == 0) {
			// clear data en update body size
			if (it != data.begin()) {
				_file.write((char *) &(*data.begin()), (&(*it)) - &(*data.begin()));
				if (!_file.good())
					throw RequestError(INTERNAL_SERVER_ERROR, "");
			}
			bodySize -= ((&(*ref) + _bondary.size()) - &(*data.begin()));
			data.erase(data.begin(), (it + _bondary.size()));
			clear();
			if (!_finish)
				throw RequestUncomplete();
			return ;
		}
		ref = it + 1;
		it = std::search(ref, end, hyphen, hyphen + 2);
	}

	_file.write((char *) &(*data.begin()), (&(*end) - &(*data.begin())));
	if (!_file.good())
		throw RequestError(INTERNAL_SERVER_ERROR, "Failed to upload file");
	bodySize -= (&(*end) - &(*data.begin()));
	data.erase(data.begin(), end);
	if (!_finish)
		throw RequestUncomplete();
	_file.close();
}


void	Upload::clear() {
	_file.close();
	_fileName = "";
	_uploading = false;
}

void	Upload::openUploadFile(std::map<std::string, std::string> const & headers) {

	std::map<std::string, std::string>::const_iterator it = headers.find("Content-Disposition");
	if (it == headers.end())
		throw RequestError(BAD_REQUEST, "Missing content disposition header to handle upload");

	size_t pos = it->second.find("filename=\"");
	if (pos == std::string::npos)
		throw RequestError(BAD_REQUEST, "Missing filename in Content-Disposition header");

	_fileName = it->second.substr(pos + 10);
	_fileName = _fileName.substr(0, _fileName.size() - 1);

	if (_fileName.empty())
		throw RequestError(BAD_REQUEST, "Filename to upload is empty");

	_file.open((_filePath + "/" + _fileName).c_str(), std::fstream::out | std::fstream::trunc | std::fstream::binary);
	if (!_file.good())
		throw RequestError(INTERNAL_SERVER_ERROR, "Failed to open file for upload");
	_uploading = true;
}