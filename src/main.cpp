#include <iostream>
#include <cstring>

#include "archive.h"
#include "buffer.h"

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Commands: pack, unpack, list, extract, verify.";
    }
    else if(std::strcmp(argv[1], "pack") == 0)
    {
        pack(argv[2], argv[3]);
    }
    else if(std::strcmp(argv[1], "list") == 0)
    {
        list(argv[2]);
    }
    else if(std::strcmp(argv[1], "extract") == 0)
    {
        extract(argv[2], argv[3]);
    }
    else if(std::strcmp(argv[1], "print") == 0)
    {
        Buffer bytes{readFileToBuffer(argv[2])};
        printFile(&bytes);
    }
    else if(std::strcmp(argv[1], "diff") == 0)
    {
        diffFiles(argv[2], argv[3]);
    }
    else
    {
        std::cout << "Commands: pack, unpack, list, extract, verify.";
    }

    return 0;
}