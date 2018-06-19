#include "config.h"
#include <fstream>


Config::Config() : address("localhost"), port("7777"), resource_dir("/"), log_path(""), error_path("/"),
				   standard_error_html(""), req_timeout(30), enable_fork(true)
{

}

std::string Config::GetHostName() const
{
	return address;
}

bool Config::GetSettsFromFile(const char *dir)
{
	using namespace std;
	ifstream file(dir);
	if(!file)
		return false;

	while(!file.eof())
	{
		string buffer;
		std::getline(file, buffer);

		buffer = _trim(buffer);
		if(buffer.empty() || buffer[0] == '#')
			continue;

		auto pos = buffer.find('=');
		if(pos == string::npos)
			return false;

		string key   = _trim(buffer.substr(0, pos));
		string value = _trim(buffer.substr(pos + 1));

		if(key == "listening_address")
            address = value;
		else if(key == "listening_port")
            port = value;
        else if(key == "resources_dir")
            resource_dir = value;
		else if(key == "log_path")
            log_path = value;
        else if(key == "error_path")
			error_path = value;
        else if(key == "standard_error_html")
			standard_error_html = value;
        else if(key == "request_timeout")
        {
            long int timeout = strtol(value.c_str(),nullptr,10);
            if(timeout < 1)
            {
                DEBUG("Timeout must be > 0");
                return false;
            }
            req_timeout = timeout;
        }
        else if(key == "enable_fork")
        {
            long int fork = strtol(value.c_str(),nullptr,10);
            if(fork == 1 || fork == 0)
                enable_fork = (bool)fork;
            else
                return false;
        }
        else if(key == "hPort_path")
            path_COMPort = value;
        else
        {
            DEBUG("Error key or value %s:%s",key.c_str(), value.c_str());
            return false;
        }

		DEBUG("%s: %s",key.c_str(), value.c_str());
	}

	return true;
}

std::string Config::_trim(const std::string& str)
{
	size_t begin = 0;
	for(; begin < str.length() && isspace(str[begin]); ++begin);

	size_t end = str.length();
	for(; end > begin && isspace(str[end - 1]); --end);

	return str.substr(begin, end);
}

std::string Config::GetResourcePath() const
{
	return resource_dir;
}

std::string Config::GetHostPort() const
{
	return port;
}

std::string Config::GetLogPath() const
{
	return log_path;
}

std::string Config::GetErrorHTMLPath() const
{
	return error_path;
}

std::string Config::GetStndrdErrorHTML() const
{
	return standard_error_html;
}

long int Config::GetTimeout() const
{
    return req_timeout;
}

std::string Config::GetPathCOMPort() const
{
    return path_COMPort;
}

bool Config::GetForkEnabled() const
{
    return enable_fork;
}