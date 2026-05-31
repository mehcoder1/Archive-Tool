#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <iomanip>

#include "buffer.h"

uint8_t readU8(const Buffer* bytes, size_t offset)
{
    return bytes->data[offset];
}

void writeU8(const Buffer* bytes, size_t offset, uint8_t byte)
{
    bytes->data[offset] = byte;
}

uint32_t readU32LE(const Buffer* bytes, size_t offset)
{
    const std::uint32_t result{static_cast<uint32_t>(bytes->data[offset]) | (static_cast<uint32_t>(bytes->data[offset+1]) << 8) | (static_cast<uint32_t>(bytes->data[offset+2]) << 16) | (static_cast<uint32_t>(bytes->data[offset+3]) << 24)};
    return result;
}

void writeU32LE(Buffer* bytes, size_t offset, uint32_t byte)
{
    bytes->data[offset] = byte & 0xFF;
    bytes->data[offset+1] = (byte >> 8) & 0xFF;
    bytes->data[offset+2] = (byte >> 16) & 0xFF;
    bytes->data[offset+3] = (byte >> 24) & 0xFF;
}

Buffer readFileToBuffer(const char* fileName)
{
    FILE* file{fopen(fileName, "rb")};

    if (file == nullptr)
    {
        std::cout << "Failed to read " << fileName << "!\n";
        std::exit(-1);
    }

    fseek(file, 0, SEEK_END);

    long fileSize {ftell(file)};

    rewind(file);

    if (fileSize < 0)
    {
        std::cout << "Failed to fetch file size!";
        std::exit(-1);
    }

    Buffer bytes{initBuffer(fileSize)};

    size_t read {fread(bytes.data, 1, bytes.size, file)};

    if (read != bytes.size)
    {
        std::cout << "Failed to read file completely!";
        std::exit(-1);
    }

    fclose(file);

    return bytes;
}

void writeBufferToFile(Buffer* bytes, const char* fileName)
{
    FILE* file{fopen(fileName, "wb")};

    if (file == nullptr)
    {
        std::cout << "Failed to create/open file!";
        std::exit(-1);
    }

    size_t wrote{fwrite(bytes->data, 1, bytes->size, file)};

    if (wrote != bytes->size)
    {
        std::cout << "Failed to completely write file, this might've corrupted it!";
        std::exit(-1);
    }

    fclose(file);
}

// TODO: Add file size check and find solution if they differ
void diffFiles(const char* fileName1, const char* fileName2)
{
    Buffer bytes1 {readFileToBuffer(fileName1)};
    Buffer bytes2 {readFileToBuffer(fileName2)};

    for (size_t i{0}; i < bytes1.size; i++)
    {
        if (bytes1.data[i] != bytes2.data[i])
        {
            std::cout << std::setw(8) << std::setfill('0') << std::uppercase << std::hex << i << ": ";
            std::cout << std::setw(2) << std::setfill('0') << std::uppercase << std::hex << int(bytes1.data[i]) << " -> ";
            std::cout << std::setw(2) << std::setfill('0') << std::uppercase << std::hex << int(bytes2.data[i]) << '\n';
        }
    }

    freeBuffer(&bytes1);
    freeBuffer(&bytes2);
}

void printFile(Buffer* bytes)
{
    std::uint64_t offset{0};

    for (size_t j{0}; j < bytes->size; j+=16)
    {
        for(size_t i{0}; i < 16; i++)
        {
            size_t index {j + i};

            if (index < bytes->size)
            {
                if (offset % 16 == 0)
                {
                    std::cout << '\n';

                    std::cout << std::setw(8) << std::setfill('0') << std::uppercase << std::hex << offset << ": ";
                }
                std::cout << std::setw(2) << std::setfill('0') << std::uppercase << std::hex << int(bytes->data[index]) << ' ';
                offset+=1;
            }
        }

        std::cout << '|';
        for(size_t i{0}; i < 16; i++)
        {
            size_t index {j + i};

            if (index < bytes->size)
            {
                if (bytes->data[index] >= 32 && bytes->data[index] <= 126)
                    std::cout << bytes->data[index];
                else
                    std::cout << '.';
            }
        }
        std::cout << "|\n";
    }
}

Buffer initBuffer(size_t size)
{
    Buffer bytes{(uint8_t*)malloc(size), size};

    if (bytes.data == nullptr)
    {
        std::cout << "Malloc failed, what the fuck";
        std::exit(-1);
    }

    return bytes;
}

void freeBuffer(Buffer* bytes)
{
    std::free(bytes->data);
    bytes->data = nullptr;
    bytes->size = 0;
}
