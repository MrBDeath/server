//
// Created by vadim on 6/22/18.
//

#ifndef SERVER_NEW_FILE_READER_H
#define SERVER_NEW_FILE_READER_H

#include "executor.h"

class CFileReader : public IExecutor
{
    virtual void Run();
public:
    CFileReader(const Config &conf);
};




#endif //SERVER_NEW_FILE_READER_H
