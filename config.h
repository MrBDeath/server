#ifndef CONFIG_H__
#define CONFIG_H__

#include <cstring>
#include <cstdio>
#include <iostream>
#include <cstdlib>
#include "debug.h"

class Config
{

private:
	std::string _trim(const std::string& str);
	std::string address;
	std::string port;
	std::string resource_dir;
	std::string log_path;
	std::string error_path;
    std::string standard_error_html;
    std::string path_COMPort;
    long int req_timeout;
    bool enable_fork;
public:

	Config();
	bool GetSettsFromFile(const char *dir);
	std::string GetHostName()           const;
	std::string GetResourcePath()       const;
	std::string GetHostPort()           const;
	std::string GetLogPath()            const;
	std::string GetErrorHTMLPath()      const;
	std::string GetStndrdErrorHTML()    const;
	std::string GetPathCOMPort()		const;
	long int GetTimeout()               const;
	bool GetForkEnabled()				const;
	~Config() = default;
};

#endif