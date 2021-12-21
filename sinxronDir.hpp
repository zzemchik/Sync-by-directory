#pragma once

#include <string>
#include <map>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <io.h>
#include <sys\stat.h>
#include <chrono>
#include <iomanip>
#include <fstream>
#include <windows.h>

#define BUFFER_SIZE 1000
const unsigned FNV_32_PRIME = 0x01000193;
const unsigned HVAL_START = 0x811c9dc5;
class SDir
{
	private:

		struct infoFile
		{
			std::string							_path;
			FILE 								*_fd;
			time_t								_lastChange;
			unsigned int						_hash;
			std::string							_dirName;
			bool								_fileChange = true;
			bool								_isHash = false;
			
			infoFile(const std::string & path, FILE *fd, time_t lastChange, const std::string & dir) 
			: _path(path), _fd(fd), _lastChange(lastChange), _dirName(dir)
			{
				_hash = HVAL_START;
			}

			void printParam(void)
			{
				std::cout << "Path: " <<  _path << " Hash: " << _hash << " Dir name: " << _dirName << " ";
				std::cout << _lastChange;
			}
			~infoFile() 
			{
			}
		};
		std::string																	_firstDir;
		std::string																	_secondDir;
		std::map<std::string, std::vector<std::pair<std::string, std::string> > >	_sameDir;
		std::map<std::string, std::vector<infoFile> >								_doubleFileMap;
		std::string																	_LogFile;
		char 																		buff[BUFFER_SIZE + 1];
	public:

		SDir(const std::string & firstDir, const std::string & secondDir, const std::string & logFile, size_t time = 0, size_t countSinhron = 0) 
		: _firstDir(firstDir), _secondDir(secondDir), _LogFile(logFile)
		{
			__checkDirExist();
			FILE * fd = fopen(_LogFile.c_str(), "w");
			if (fd == NULL)
				throw("Incorect log file!");
			fclose(fd);
			__initDir(_firstDir);
			__initDir(_secondDir);
			__checkDir();
			__checkFile();
			std::cout << "Dir sinhron" << std::endl;

			if (time != 0)
			{
				if (countSinhron == 0)
				{
					while (1)
					{
						Sleep(time * 1000);
						__updateMap(_secondDir);
						__checkDir();
						__checkFile();
						std::cout << "Dir sinhron" << std::endl;
					}
				}
				else
				{
					while (--countSinhron)
					{
						Sleep(time * 1000);
						__updateMap(_secondDir);
						__checkDir();
						__checkFile();
						std::cout << "Dir sinhron" << std::endl;
					}
				}
			}
		}
		
		~SDir()
		{
		}
		private:

		std::string __pathFileInDir(std::string path, std::string dir, int k)
		{
			int i = 0;
			while (path[i] == dir[i])
				++i;
			return (path.substr(i + k));
		}


		void __updateMapDir(std::vector<std::string> & newDir, const std::string & dir)
		{
			std::vector<int> vecDir;
			std::vector<std::string> vecNewDir;
			for (size_t i = 0; i < _sameDir.size(); ++i)
				vecDir.push_back(0);
			for (size_t i = 0; i < newDir.size(); ++i)
			{
				int flag = 0;
				int iterVec = 0;
				for (auto it = _sameDir.begin(); it != _sameDir.end(); ++it)
				{

					if (it->first == __pathFileInDir(newDir[i], dir, 0))
					{
						vecDir[iterVec] = 1;
						flag = 1;
					}
					++iterVec;
				}
				if (flag == 0)
					vecNewDir.push_back(newDir[i]);
			}
			int n = 0;
			for (auto it = _sameDir.begin(); it != _sameDir.end(); ++it)
			{
				if (vecDir[n] == 0)
				{
					if (it->second[0].first == dir)
						it->second.erase(it->second.begin());
					else
						it->second.erase(++it->second.begin());
				}
				++n;
			}
			for (size_t i = 0; i < vecNewDir.size(); ++i)
			{
				_sameDir[__pathFileInDir(vecNewDir[i], dir, 0)].push_back(std::make_pair(vecNewDir[i], dir));
			}
		}
		
		void __updateMap(const std::string & dir)
		{
			
			for (auto it = _doubleFileMap.begin(); it != _doubleFileMap.end(); ++it)
			{
				for (size_t i = 0; i < it->second.size(); ++i)
				{
					it->second[i]._fileChange = false;
				}
			}
			std::vector<std::string> newPath;
			std::vector<std::string> newDir;
			for (std::filesystem::recursive_directory_iterator it(dir), end; it != end; ++it)
			{
				std::string path = it->path().generic_string();
                if (std::filesystem::is_regular_file(*it))
					newPath.push_back(path);
				else
					newDir.push_back(path);
			}

			std::vector<int> vecPath;
			for (size_t i = 0; i < _doubleFileMap.size(); ++i)
				vecPath.push_back(0);

			std::vector<std::string> createFile;
			for (size_t i = 0; i < newPath.size(); ++i)
			{
				int iterVec = 0;
				int flag = 0;
				for (auto it = _doubleFileMap.begin(); it != _doubleFileMap.end(); ++it)
				{
					if (__pathFileInDir(newPath[i], dir, 0) == it->first)
					{
						time_t tmp;
						if (it->second[0]._dirName == dir)
							tmp = it->second[0]._lastChange;
						else
							tmp = it->second[1]._lastChange;
						if (__last_write_time(std::filesystem::last_write_time(std::filesystem::path(newPath[i]))) != tmp)
						{
							if (it->second[0]._dirName == dir)
							tmp = it->second[0]._fileChange = true;
							else
							tmp = it->second[1]._fileChange = true;
						}
						flag = 1;
						vecPath[iterVec] = 1;
						break;
					}
					++iterVec;
				}
				if (flag == 0)
				{
					createFile.push_back(newPath[i]);
				}
			}
			int i = 0;
			for (auto it = _doubleFileMap.begin(); it != _doubleFileMap.end(); ++it)
			{
				if (vecPath[i] == 0)
				{
					if (it->second[0]._dirName == dir)
						it->second.erase(it->second.begin());
					else
						it->second.erase(++it->second.begin());
				}
				++i;
			}
			for (size_t i = 0; i < createFile.size(); ++i)
			{
				infoFile tmpInfoFile(createFile[i], 0, __last_write_time(std::filesystem::last_write_time(std::filesystem::path(createFile[i]))), dir);
				_doubleFileMap[__pathFileInDir(createFile[i], dir, 0)].push_back(tmpInfoFile);
			}
			__updateMapDir(newDir, dir);
		}

		void __LogFile(const std::string & log)
		{
			FILE * fd = fopen(_LogFile.c_str(), "a");
			if (fd == NULL)
				throw("Incorect log file!");
			fwrite(log.c_str(), sizeof(char), log.size(), fd);
			fclose(fd);
			
		}

		void __checkDirExist( void )
        {
            if (!(std::filesystem::is_directory(_firstDir) && std::filesystem::is_directory(_secondDir)))
                throw ("Directory not found!");
        }

     	unsigned int __hashFile(char * buff, unsigned int hval)
        {
            while (*buff)
            {
                hval ^= (unsigned int)*buff++;
                hval *= FNV_32_PRIME;
            }
            return hval;
        }

		bool __eraseOrCopy(std::vector<infoFile> & files, const std::string & fileName)
		{
			if (files[0]._dirName == _firstDir)
			{
				std::string log = "Delte new file: " + files[0]._path + "\n";
				__LogFile(log);
				remove(files[0]._path.c_str());
				return(true);
			}
			else
			{
				std::string newFile = _firstDir + __pathFileInDir(files[0]._path, _secondDir, 0);
				std::string log = "Create new file: " + newFile + "\n";
				__LogFile(log);

				FILE * fd = fopen(newFile.c_str(), "w");
				FILE * fdRD = fopen(files[0]._path.c_str(), "r");
				if (fd == NULL)
					throw("One or more file have bad permishion!1");
				if (fdRD == NULL)
					throw("One or more file have bad permishion!2");
				infoFile tmpInfoFile(newFile, 0, __last_write_time(std::filesystem::last_write_time(std::filesystem::path(newFile))), _firstDir);
				int size = 0;
				while (1)
				{
					size = fread(buff, sizeof(char), BUFFER_SIZE, fdRD);
					buff[size] = 0;
					if (size == 0)
						break;
					tmpInfoFile._hash = __hashFile(buff, tmpInfoFile._hash);
					files[0]._hash = tmpInfoFile._hash;
					fwrite(buff, sizeof(char), size, fd);
				}
				tmpInfoFile._isHash = true;
				files[0]._isHash = true;
				files[0]._lastChange = __last_write_time(std::filesystem::last_write_time(std::filesystem::path(files[0]._path)));
				_doubleFileMap[fileName].push_back(tmpInfoFile);
				fclose(fd);
				fclose(fdRD);


			}
			return false;
		}


		void __changeFile(infoFile & file1, infoFile & file2)
		{
			file1._fd = fopen(file1._path.c_str(), "w");
			file2._fd = fopen(file2._path.c_str(), "r");
			if (file2._fd == NULL || file1._fd == NULL)
					throw("One or more file have bad permishion!");
			std::string log = "Change file: " + file1._path + "\n";
			__LogFile(log);
			unsigned int hash = HVAL_START;
			while (1)
			{
				int size = fread(buff, sizeof(char), BUFFER_SIZE, file2._fd);
				buff[size] = 0;
				if (size == 0)
					break;
				hash = __hashFile(buff, hash);
				file1._hash = hash;
				file2._hash = hash;
				fwrite(buff, sizeof(char), size, file1._fd);
			}
			fclose(file1._fd);
			fclose(file2._fd);
		}

		void __compaereFileAndcopy(infoFile & file1, infoFile & file2)
		{
			if (file1._isHash == file2._isHash == true)
			{
				if (file1._hash == file2._hash && file2._fileChange == false)
					return ;
				else
				{
					int i = 1;
					file2._fd = fopen(file2._path.c_str(), "r");
					file1._fd = fopen(file1._path.c_str(), "r");
					if (file2._fd == NULL || file1._fd == NULL)
						throw("One or more file have bad permishion!");
					unsigned int hash1 = HVAL_START;
					unsigned int hash2 = HVAL_START;

					while (1)
					{
						int size = fread(buff, sizeof(char), BUFFER_SIZE, file2._fd);
						buff[size] = 0;
						hash2 = __hashFile(buff, hash2);
						int size1 = fread(buff, sizeof(char), BUFFER_SIZE, file1._fd);
						buff[size1] = 0;
						if (size == 0 && size1 == 0)
							break;
						hash1 = __hashFile(buff, hash1);
						if (hash1 != hash2)
						{
							__changeFile(file1, file2);
							break;
						}

						i++;
					}
					fclose(file2._fd);
					fclose(file1._fd);

				}
			}
			else
				__changeFile(file1, file2);
		}

		void __checkFile()
		{
			for (auto it = _doubleFileMap.begin(); it != _doubleFileMap.end();)
			{
				if (it->second.size() < 2)
				{
					if (__eraseOrCopy(it->second, it->first))
					{
						it = _doubleFileMap.erase(it);
						continue;
					}
				}
				else
				if (it->second[0]._dirName == _firstDir)
					__compaereFileAndcopy(it->second[0], it->second[1]);
				else
					__compaereFileAndcopy(it->second[1], it->second[0]);
				++it;
			}
		}

		bool __eraseOrCopyDir(const std::string & nameDir, const std::string & pathDir, const std::string & dirGlobal)
		{
			if (dirGlobal == _firstDir)
			{
				std::filesystem::remove_all(pathDir.c_str());
				return true;
			}
			else
			{
				std::string newPath = _firstDir + "/" + __pathFileInDir(pathDir, _secondDir, 0);
				std::filesystem::create_directories(newPath);
				_sameDir[__pathFileInDir(pathDir, _secondDir, 0)].push_back(std::make_pair(newPath, _firstDir));
				std::string log = "Create dir: " + _firstDir + __pathFileInDir(pathDir, _secondDir, 0) + "\n";
				__LogFile(log);
			}
			return false;
		}

		void __checkDir()
		{
			for (auto it = _sameDir.begin(); it != _sameDir.end();)
			{
				if (it->second.size() < 2)
				{
					if (__eraseOrCopyDir(it->first, it->second[0].first, it->second[0].second))
					{
						it = _sameDir.erase(it);
						continue;
					}
				}
				++it;
			}
		}


		time_t __last_write_time (std::filesystem::file_time_type const& ftime) 
		{
			using namespace std::chrono;
			auto sctp = time_point_cast<system_clock::duration>(ftime - std::filesystem::file_time_type::clock::now()
					+ system_clock::now());
			return system_clock::to_time_t(sctp);
		}

		void __initDir(const std::string & dir)
		{

			for (std::filesystem::recursive_directory_iterator it(dir), end; it != end; ++it)
			{
				std::string path = it->path().generic_string();
                if (std::filesystem::is_regular_file(*it))
				{
					infoFile tmpInfoFile(path, 0, __last_write_time(std::filesystem::last_write_time(it->path())), dir);
					_doubleFileMap[__pathFileInDir(path, dir, 0)].push_back(tmpInfoFile);
				}
				else
				{
					_sameDir[__pathFileInDir(path, dir, 0)].push_back(std::make_pair(path, dir));
				}
			}
		}
		
};



