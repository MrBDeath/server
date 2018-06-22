#ifndef LOG_H__
#define LOG_H__


#include <cstdio>
#include <string>

#define ERROR(text,...)     (Log::Instance()->insert(text, LG_ERR, ##__VA_ARGS__))
#define WARNING(text,...)   (Log::Instance()->insert(text, LG_WRG, ##__VA_ARGS__))
#define INFO(text,...)      (Log::Instance()->insert(text, LG_INF, ##__VA_ARGS__))
#define LOG(text,...)       (Log::Instance()->insert(text, LG_EMP, ##__VA_ARGS__))

enum logs_type
{
    LG_ERR,
    LG_WRG,
    LG_INF,
    LG_EMP,
};

class Log
{
private:

    FILE *log_file;
    std::string log_file_path;
    static Log* _self;

    Log();
    ~Log();

public:

    static Log* Instance();
    void init(std::string path);
    void destroy() {if(_self) { delete _self; _self = nullptr; } }
    void insert(std::string text, logs_type l = LG_ERR/* int type = lg_err*/, ...);

};

#endif