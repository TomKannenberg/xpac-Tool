#pragma once
#define exc const std::exception &e
#define Catch catch(exc) {}
#define Catchd catch(exc) 

#include <algorithm>
#include <conio.h>
#include <sstream>
#include <vector>
#include <chrono>
#include <mutex>
#include <Windows.h>
//#include <wincodec.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <wrl/client.h>

#include "resource.h"
#include "xpac_utilities.hpp"

using namespace std::experimental::filesystem;

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
        std::string Repack(const char* gpacFileName);
    
        // up = unpack , rp = repack
        bool upCompressedFiles = false;
        bool rpDDSFiles = true;
        bool debug = false;
    
    private:
        std::mutex mx;
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
    
        void CompThreaded(path decPath, path filePath, DWORD hash);
        bool PackTextures(path filePath, const uXPACEntry& entry);
        bool Compress(std::vector<char>& input, std::vector<char>& compressedData);
    
        void CSaveThreaded(const std::vector<char>& data, std::string ddsFile) const;

        std::string repackString(const DWORD hash);
        int Hash(const char* pString);
};

HMODULE GCM();