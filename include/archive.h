#pragma once

#include <cstddef>

constexpr size_t FILE_TABLE_OFFSET{8};
constexpr size_t FILE_TABLE_ENTRY_SIZE{40};

void pack(const char* directory, const char* fileName);
void list(const char* fileName);
void extract(const char* pakName, const char* fileName);
