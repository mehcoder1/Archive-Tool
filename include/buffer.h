#pragma once

#include <cstdint>

struct Buffer
{
    uint8_t* data;
    size_t size;
};

uint8_t readU8(const Buffer* bytes, size_t offset);
void writeU8(Buffer* bytes, size_t offset, uint8_t byte);

uint32_t readU32LE(const Buffer* bytes, size_t offset);
void writeU32LE(Buffer* bytes, size_t offset, uint32_t byte);

Buffer readFileToBuffer(const char* fileName);
void writeBufferToFile(Buffer* bytes, const char* fileName);

void diffFiles(const char* fileName1, const char* fileName2);

void printFile(Buffer* bytes);

Buffer initBuffer(size_t size);
void freeBuffer(Buffer* bytes);
