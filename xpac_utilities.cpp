#include "xpac_utilities.hpp"

XU::XPACUtility(const std::string* folder) : folder(folder) {
    CreateFolder(*folder);
}

std::string XU::CreateFolders(std::string filePath) {
    std::string currentPath = *folder;
    for (const auto& part : fs::path(filePath)) {
        std::string p2 = part.string();
        if (p2.find('.') != std::string::npos) {
            return currentPath + p2;
        }
        p2 = "_" + p2;
        currentPath += p2;
        CreateFolder(currentPath);
        currentPath += '/';
    }
    return currentPath;
}

void XU::CreateFolder(const std::string& folderPath) {
    try {
        if (!fs::exists(folderPath)) {
            fs::create_directory(folderPath);
        }
    }
    catch (const fs::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void XU::FixFolder(std::string& path) {
    for (auto& ch : path) {
        if (ch == '\\') {
            ch = '/';
        }
    }
}

void XU::save(const std::string& name, const std::vector<char>& fileData) {
    std::ofstream of(name, std::ios::binary);
    of.write(fileData.data(), fileData.size());
}

void EM::start(const std::string& functionName) {
    isMonitoring[functionName] = true;
    startTimes[functionName] = std::chrono::high_resolution_clock::now();
}

void EM::stop(const std::string& functionName, std::chrono::high_resolution_clock::time_point endTime) {
    if (isMonitoring[functionName]) {
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTimes[functionName]);
        totalTimes[functionName] += duration;
        isMonitoring[functionName] = false;
    }
}

void EM::toggle(const std::string& functionName) {
    auto time = std::chrono::high_resolution_clock::now();
    if (isMonitoring[functionName]) {
        stop(functionName, time);
    } else {
        start(functionName);
    }
}

void EM::printAllTimes() {
    std::cout << "Execution times:\n";
    std::chrono::nanoseconds totalTime(0);

    for (const auto& entry : totalTimes) {
        std::cout << entry.first << ": " << (double)entry.second.count() / 1'000'000'000 << " seconds\n";
        totalTime += entry.second;
    }

    std::cout << "Full time: " << (double)totalTime.count() / 1'000'000'000 << " seconds\n";
}

std::string EM::printAllTimes(const std::string& name) {
    std::cout << "Extracted " << name << " in ";
    std::chrono::nanoseconds totalTime(0);

    for (const auto& entry : totalTimes) {
        totalTime += entry.second;
    }

    std::string returns = std::to_string((double)totalTime.count() / 1'000'000'000) + " seconds!";

    printAllTimes();

    std::cout << returns << std::endl;

    return returns;
}

void XPACUtility::ConditionalFree(const bool& condition, void* freemem) {
    while (!condition) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    free(freemem);
}
