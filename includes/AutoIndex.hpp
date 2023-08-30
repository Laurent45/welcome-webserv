#ifndef AUTO_INDEX_HPP
# define AUTO_INDEX_HPP

#include <string>

class AutoIndex
{
	private:
			std::string     _dirPath;
			std::string     _indexPage;

	public:

			AutoIndex(const std::string& path);
			AutoIndex(const AutoIndex& other);
			~AutoIndex();
			AutoIndex&      operator=(const AutoIndex& other);

			std::string     getIndexPage() const;
			std::string     getDirPath() const;

	private:

        void            _generateIndexPage();
		std::string		_generateHtmlLink(const unsigned char fileType, const std::string& fileName);
		std::string	    _generateHtmlHeader();
        std::string	    _generateHtmlFooter();
		std::string		_getFileLink(const unsigned char fileType, std::string fileName);

		void			_formatCell(std::string* data);
		std::string		_getFileSize(struct stat s);
		std::string		_getFileModTime(struct stat s);
};
#endif