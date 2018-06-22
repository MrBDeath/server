//
// Created by vadim on 6/22/18.
//

#include "executor.h"
#include <fstream>
#include <sstream>
#include "file_reader.h"
// -------------------------------

// -------------------------------

void CFileReader::Run()
{

    std::ifstream n_file(cfg.GetResourcePath() + uri,
                         std::ifstream::binary | std::ifstream::in);
    if(!n_file)
    {
        stRes = {404, "Not Found"};
        n_file.open(cfg.GetErrorHTMLPath() + "/" + std::to_string(stRes.Type) + ".html",
                    std::ifstream::binary | std::ifstream::in);
    }
    if(!n_file)
        n_file.open(cfg.GetErrorHTMLPath() + "/" + cfg.GetStndrdErrorHTML() + ".html",
                    std::ifstream::binary | std::ifstream::in);
    if(!n_file)
    {
        response =  "<html>\n"
                    "    <head>\n"
                    "        <title>Error</title>\n"
                    "    </head>\n"
                    "    <body>\n"
                    "        <center>\n"
                    "            <h1>Something went wrong</h1>\n"
                    "        </center>\n"
                    "    </body>\n"
                    "</html>";
    }
    else
    {
        std::stringstream res;
        res << n_file.rdbuf();
        n_file.close();
        response = res.str();
    }

}

CFileReader::CFileReader(const Config &conf) : IExecutor(conf)
{
}

