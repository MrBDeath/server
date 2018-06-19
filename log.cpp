//
// Created by vadim on 30.05.18.
//


#include "log.h"

Log* Log::_self=nullptr;

Log::Log() : log_file(nullptr), log_file_path("")
{

}

void Log::insert(std::string text, logs_type l, ...)
{
    if(access(log_file_path.c_str(),0) == -1)
    {
        DEBUG("Log file is broken or deleted. Reopening...");
        fclose(log_file);
        log_file = fopen(log_file_path.c_str(),"a");
        if(!log_file) {
            DEBUG("Can't reopen file %s", log_file_path.c_str());
            log_file = nullptr;
            return;
        }
        DEBUG("Success");
    }
    std::string insert_type;
    switch(l)
    {
        case LG_ERR: insert_type = "ERROR"; break;
        case LG_WRG: insert_type = "WARNING"; break;
        case LG_INF: insert_type = "INFO"; break;
        default: insert_type = "UNDEFINED"; break;
    }
    fseek(log_file, 0, SEEK_END);

    tm *time_st;
    time_t tm_lc = time(nullptr);
    time_st = localtime(&tm_lc);

    fprintf(log_file,"[%02d/%02d/%d] %02d:%02d:%02d ",time_st->tm_mday,time_st->tm_mon,
            time_st->tm_year+1900, time_st->tm_hour,time_st->tm_min,time_st->tm_sec);


    if(l != LG_EMP)
    {
        fprintf(log_file, "%s: ", insert_type.c_str());
        va_list argptr;
        va_start(argptr, l);
        vfprintf(log_file, text.c_str(), argptr);
        va_end(argptr);
    }
    else
        fputs(text.c_str(),log_file);

    fputs("\n",log_file);
    fflush(log_file);

}

Log::~Log()
{
    fclose(log_file);
}

void Log::init(std::string path)
{
    std::string lg = path + "server.log";
    log_file_path = lg;

    log_file = fopen(log_file_path.c_str(),"a");
    if(log_file == nullptr)
    {
        DEBUG("Error init %s", log_file_path.c_str());
        return;
    }
}

Log *Log::Instance() {
    if(!_self) _self = new Log();
    return _self;
}
