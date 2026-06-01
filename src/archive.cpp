#include <filesystem>
#include <cstdio>
#include <string>
#include <iostream>
#include <cstring>

#include "archive.h"
#include "buffer.h"

/*

FILE FORMAT:

Header:
First 4 bytes, magic header MARC
Next 4 bytes, file count

Filetable (for each item):
32 bytes for the name
4 bytes for the size
4 bytes for the data offset

Then the big data blob stores file data contiguously

*/

void getString(const Buffer* bytes, size_t offset, char* name)
{
    for (size_t idx{0}; idx < 32; idx++)
    {
        name[idx] = readU8(bytes, offset+idx);
    }
}

void pack(const char* directory, const char* fileName)
{
    FILE* file{fopen(fileName, "wb")};

    if (file == nullptr)
    {
        std::cout << "Failed to create/open file while packing!";
        std::exit(-1);
    }

    size_t fileCount{0};
    for (const auto& entry : std::filesystem::directory_iterator(directory))
    {
        std::string name {entry.path().filename().string()};
        size_t lenName = name.size();
        if (!entry.is_regular_file() || lenName >=32)
            continue;

        fileCount++;
    }

    fwrite("MARC", 1, 4, file);

    Buffer bytesFileCount(initBuffer(4));
    writeU32LE(&bytesFileCount, 0, uint32_t(fileCount));
    fwrite(bytesFileCount.data, 1, bytesFileCount.size, file);

    Buffer byteArray[fileCount];
    size_t offset{FILE_TABLE_OFFSET + (FILE_TABLE_ENTRY_SIZE * fileCount)};

    size_t idx{0};
    for (const auto& entry : std::filesystem::directory_iterator(directory))
    {
        if (!entry.is_regular_file())
            continue;

        std::string name {entry.path().filename().string()};
        size_t lenName = name.size();
        char cName[32];

        if (lenName >= 32)
        {
            std::cout << "Filename " << name << " is too long, skipping...\n";
            continue;
        }

        for(size_t i{0}; i < 31; i++)
        {
            if (i < lenName)
                cName[i] = name[i];
            else
                cName[i] = 0;
        }

        std::cout << "Reading " << cName << " ...\n";
        byteArray[idx] = readFileToBuffer(entry.path().string().c_str());

        Buffer bytesFileSize{initBuffer(4)};
        writeU32LE(&bytesFileSize, 0, uint32_t(byteArray[idx].size));

        fwrite(cName, 1, 32, file);
        fwrite(bytesFileSize.data, 1, 4, file);

        Buffer bytesOffset{initBuffer(4)};
        writeU32LE(&bytesOffset, 0, uint32_t(offset));
        fwrite(bytesOffset.data, 1, 4, file);

        offset += byteArray[idx].size;

        idx++;
    }

    for (size_t i{0}; i < fileCount; i++)
    {
        size_t wrote {fwrite(byteArray[i].data, 1, byteArray[i].size, file)};

        if (wrote != byteArray[i].size)
        {
            std::cout << "Failed to pack file! Wrote " << wrote << " bytes while total array byte size was " << byteArray[i].size;
            std::exit(-1);
        }
    }
}

void list(const char* fileName)
{
    Buffer bytes{readFileToBuffer(fileName)};

    uint32_t fileCount {readU32LE(&bytes, 4)};

    for (int i{0}; i < fileCount; i++)
    {
        char name[32]{};
        getString(&bytes, FILE_TABLE_OFFSET+(FILE_TABLE_ENTRY_SIZE*i), name);
        std::cout << "File: " << name << '\n';
        std::cout << "   Size: " << readU32LE(&bytes, 32+FILE_TABLE_OFFSET+(FILE_TABLE_ENTRY_SIZE*i)) << '\n';
        std::cout << "   Offset: " << readU32LE(&bytes, 36+FILE_TABLE_OFFSET+(FILE_TABLE_ENTRY_SIZE*i)) << "\n\n";
    }
}

void extract(const char* pakName, const char* fileName)
{
    Buffer bytes{readFileToBuffer(pakName)};

    uint32_t fileCount {readU32LE(&bytes, 4)};

    for (uint32_t i{0}; i < fileCount; i++)
    {
        char* name{(char*)malloc(32)};
        getString(&bytes, FILE_TABLE_OFFSET+(FILE_TABLE_ENTRY_SIZE*i), name);

        if (strcmp(name, fileName) == 0)
        {
            size_t size {readU32LE(&bytes, 32+FILE_TABLE_OFFSET+(FILE_TABLE_ENTRY_SIZE*i))};
            uint32_t offset {readU32LE(&bytes, 36+FILE_TABLE_OFFSET+(FILE_TABLE_ENTRY_SIZE*i))};

            Buffer fileBytes{initBuffer(size)};

            for (size_t j{0}; j < size; j++)
            {
                fileBytes.data[j] = bytes.data[offset+j];
            }

            writeBufferToFile(&fileBytes, fileName);

            freeBuffer(&bytes);
            freeBuffer(&fileBytes);
            std::free(name);

            std::cout << "Extracted " << fileName << " from " << pakName << ".\n";

            return;
        }
    }

    std::cout << "Did not find " << fileName << " in the archive!";
}

void unpack(const char* pakName, const char* folderName)
{
    if (!(std::filesystem::exists(folderName) || std::filesystem::is_directory(folderName)))
        std::filesystem::create_directories(folderName);

    Buffer bytes{readFileToBuffer(pakName)};

    uint32_t fileCount {readU32LE(&bytes, 4)};

    for (int i{0}; i < fileCount; i++)
    {
        char name[32]{};
        getString(&bytes, FILE_TABLE_OFFSET+(FILE_TABLE_ENTRY_SIZE*i), name);

        size_t size {readU32LE(&bytes, 32+FILE_TABLE_OFFSET+(FILE_TABLE_ENTRY_SIZE*i))};
        uint32_t offset {readU32LE(&bytes, 36+FILE_TABLE_OFFSET+(FILE_TABLE_ENTRY_SIZE*i))};

        Buffer fileBytes{initBuffer(size)};

        for (size_t j{0}; j < size; j++)
        {
            fileBytes.data[j] = bytes.data[offset+j];
        }

        size_t lenFolderName = strlen(folderName);

        char path[34+lenFolderName];
        path[0] = '\0';
        strcat(path, folderName);
        strcat(path, "/");
        strcat(path, name);

        std::cout << "Extracting " << path << "...\n";

        writeBufferToFile(&fileBytes, path);

        freeBuffer(&fileBytes);

        std::cout << "Extracted " << name << " from " << pakName << ".\n";
    }

    freeBuffer(&bytes);
}

bool verify(const char* pakName)
{
    Buffer bytes{readFileToBuffer(pakName)};

    if (bytes.data[0] != 'M' || bytes.data[1] != 'A' || bytes.data[2] != 'R' || bytes.data[3] != 'C')
        return false;

    uint32_t fileCount {readU32LE(&bytes, 4)};

    size_t calculatedSize {8 + FILE_TABLE_ENTRY_SIZE*fileCount};

    for (size_t i{0}; i < fileCount; i++)
    {
        char name[32]{};
        getString(&bytes, FILE_TABLE_OFFSET+(FILE_TABLE_ENTRY_SIZE*i), name);

        if (strcmp(name, "") == 0)
        {
            std::cout << "File name is empty.\n";
            return false;
        }

        if (name[31] != '\0')
        {
            std::cout << "File name is non null-terminated.\n";
            return false;
        }

        if (strchr(name, '/') != nullptr)
        {
            std::cout << "File name has '/' character.\n";
            return false;
        }

        if (strchr(name, ':') != nullptr)
        {
            std::cout << "File name has '/' character.\n";
            return false;
        }

        uint32_t fileSize {readU32LE(&bytes, 40+FILE_TABLE_ENTRY_SIZE*i)};
        uint32_t fileOffset {readU32LE(&bytes, 44+FILE_TABLE_ENTRY_SIZE*i)};

        if (fileOffset < 8+FILE_TABLE_ENTRY_SIZE*fileCount)
        {
            std::cout << "File offset is wrong/corrupted.";
            return false;
        }

        calculatedSize+=fileSize;
    }

    if (calculatedSize != bytes.size)
    {
        std::cout << "Calculated file size: " << calculatedSize << ", Actual file size: " << bytes.size << '\n';
        return false;
    }

    return true;
}
