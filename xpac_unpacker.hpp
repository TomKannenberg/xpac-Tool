#pragma once
#define exc const std::exception &e
#define Catch catch(exc) {}
#define Catchd catch(exc) 

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <unordered_map>

#include <zlib.h>

#include "xpac_utilities.hpp"

namespace fs = std::filesystem;
using fs::path;

struct Zif {
    uint32_t size;


};

struct Zig {
    uint32_t size;
};

using DWORD = uint32_t;

struct XPACHeader {
    DWORD dwZero1;
    DWORD dwZero2;
    DWORD dwTableSize;
    DWORD dwTotalFiles;
    DWORD dwDirTable;
    DWORD dwZero3;
};

struct XPACEntry {
    DWORD dwHash;
    DWORD dwOffset;
    DWORD dwDecompressedSize;
    DWORD dwCompressedSize;
    DWORD dwNull;
};

struct uXPACEntry {
    bool isCompressed;
    bool hasTextures;
    uint32_t textures;
    uint32_t* textureOffsets;
    DWORD dwHash;
    DWORD dwOffset;
    DWORD dwDecompressedSize;
    DWORD dwCompressedSize;
    DWORD dwNull;
};

constexpr uint32_t DDS_HEADER = 0x20534444;
constexpr size_t headerSize = sizeof(DDS_HEADER);

class XPAC {
public:
    XPAC(const std::string* folder);
    ~XPAC();
    ExecutionMonitor* em = new ExecutionMonitor();
    XPACUtility* xu = nullptr;

    std::string Unpack(const char* xpacFileName);
    std::string UnpackZifFirst(const char* xpacFileName);
    std::string Repack(const char* gpacFileName);

    // up = unpack , rp = repack
    bool upAssetInfo;
    bool upCompressedFiles = false;
    bool rpDDSFiles = true;
    bool debug = false;

private:
    std::mutex mx;
    std::mutex mxUM;
    size_t ycpos = -1;
    size_t activeThreads = 0;
    std::unordered_map<std::string, std::string> unhashmap;
    std::vector<std::thread> threads;
    std::string cFile = "";

    std::unordered_map<DWORD, uint32_t> utextures;
    std::unordered_map<DWORD, uint32_t*> utextureOffsets;
    std::unordered_map<DWORD, bool> uhasTextures;
    std::unordered_map<DWORD, bool> uisCompressed;

    void loadUnHasher();

    void DecompThreaded(Bytef* compressedData, const XPACEntry entry, std::string unhash);
    bool Decompress(Bytef* compressedData, size_t compressedSize, std::vector<char>& output) const;
    void FindDDS(const std::vector<char>& data, std::vector<std::vector<char>>& ddsArray, const DWORD hash);
    void extractZif(const std::vector<char>& data, std::string foldername);

    void CompThreaded(path decPath, path filePath, DWORD hash);
    bool PackTextures(path filePath, const uXPACEntry& entry);
    bool Compress(std::vector<char>& input, std::vector<char>& compressedData);

    void CSaveThreaded(const std::vector<char>& data, std::string ddsFile) const;
    std::string repackString(const DWORD hash);
    int Hash(const char* pString);
    void printPercent(const double &d);
};

//std::string XPAC::Repack(const char* gpacFileName) {
//    //repack info function
//    std::ifstream gpac2(gpacFileName, std::ios::in | std::ios::binary);
//
//    if (!gpac2.is_open()) {
//        return "Error opening GPAC file for reading!";
//    }
//
//    XPACHeader header;
//    gpac2.read(reinterpret_cast<char*>(&header), sizeof(XPACHeader));
//    uXPACEntry* uEntries = (uXPACEntry*)malloc(sizeof(uXPACEntry) * header.dwTotalFiles);
//    gpac2.read(reinterpret_cast<char*>(uEntries), sizeof(uXPACEntry) * header.dwTotalFiles);
//    gpac2.close();
//    std::cout << "XPACHeader Members:\n";
//    std::cout << "dwZero1: " << header.dwZero1 << std::endl;
//    std::cout << "dwZero2: " << header.dwZero2 << std::endl;
//    std::cout << "dwTableSize: " << header.dwTableSize << std::endl;
//    std::cout << "dwTotalFiles: " << header.dwTotalFiles << std::endl;
//    std::cout << "dwDirTable: " << header.dwDirTable << std::endl;
//    std::cout << "dwZero3: " << header.dwZero3 << std::endl;
//    std::cout << "------------------------\n";
//
//    for (size_t i = 0; i < header.dwTotalFiles; ++i) {
//        std::cout << path(gpacFileName).filename().string() << " " << i + 1 << " Name: " << uEntries[i].dwName << std::endl;
//        std::cout << " isCompressed: " << uEntries[i].isCompressed << std::endl;
//        std::cout << " hasTextures: " << uEntries[i].hasTextures << std::endl;
//        std::cout << " textures: " << uEntries[i].textures << std::endl;
//        for (size_t j = 0; j < uEntries[i].textures; ++j) {
//            std::cout << " textureOffsets[" << j << "]: " << uEntries[i].textureOffsets[j] << std::endl;
//        }
//    }
//    free(uEntries);
//
//    return "";
//}
