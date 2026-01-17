#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <numeric>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
#include <zlib.h>

#define EM ExecutionMonitor
#define XU XPACUtility

namespace fs = std::filesystem;

class ExecutionMonitor {
private:
    std::unordered_map<std::string, std::chrono::high_resolution_clock::time_point> startTimes;
    std::unordered_map<std::string, std::chrono::nanoseconds> totalTimes;
    std::unordered_map<std::string, bool> isMonitoring;

public:
    void start(const std::string& functionName);

    void stop(const std::string& functionName, std::chrono::high_resolution_clock::time_point endTime = std::chrono::high_resolution_clock::now());

    void toggle(const std::string& functionName);

    void printAllTimes();
    std::string printAllTimes(const std::string& name);
};

class XPACUtility {
    public:

        XPACUtility(const std::string* folder);

        const std::string* folder;
        
        void FixFolder(std::string& path);
        std::string CreateFolders(std::string filePath);
        void CreateFolder(const std::string& folderPath);

        void save(const std::string& name, const std::vector<char>& fileData);
        void ConditionalFree(const bool& condition, void* freemem);
};

std::string wts(const std::wstring& ws);
std::wstring stw(const std::string& s);

std::string GetLocationXpac();
std::string GetLocationGpac();
