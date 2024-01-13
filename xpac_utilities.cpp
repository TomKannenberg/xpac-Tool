#include "xpac_utilities.hpp"

XU::XPACUtility(const std::string* folder) : folder(folder) {
    CreateFolder(*folder);
}

std::string XU::CreateFolders(std::string filePath) {

    std::string currentPath = *folder;
    for (const auto& part : path(filePath)) {
        std::string p2 = part.string();
        if (p2.find(".") != std::string::npos) {
            return currentPath + p2;
        }
        p2 = "_" + p2;
        currentPath += p2;
        CreateFolder(currentPath);
        currentPath += "\\";
    }

    return currentPath;
}

void XU::CreateFolder(const std::string& folderPath) {
    try {
        if (!exists(folderPath)) {
            create_directory(folderPath);
            //std::cout << "Created " << folderPath << " folder." << std::endl;
        } else {
            //std::cout << folderPath << " found." << std::endl;
        }
    }
    catch (const filesystem_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void XU::FixFolder(std::string& path) {
    for (int i = 0; i < path.length(); i++) {
        if (path[i] == '/') {
            path[i] = '\\';
        }
    }
}

void XU::save(const std::string& name, const std::vector<char>& fileData) {
    std::ofstream of(name, std::ios::binary);
    of.write(fileData.data(), fileData.size());
    of.close();
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
        std::cout << entry.first << ": " << (double)entry.second.count() / 1000000000 << " seconds\n";
        totalTime += entry.second;
    }

    std::cout << "Full time: " << (double)totalTime.count() / 1000000000 << " seconds\n";
}

std::string EM::printAllTimes(const std::string &name) {
    std::cout << "Extracted " + name + " in ";
    std::chrono::nanoseconds totalTime(0);

    for (const auto& entry : totalTimes) {
        totalTime += entry.second;
    }

    return std::to_string((double)totalTime.count() / 1000000000) + " seconds!";
}

std::string wts(const std::wstring& ws) {
    std::string sTmp(ws.begin(), ws.end());
    return sTmp;
}

std::string GetLocationXpac() {
    OPENFILENAMEW ofn;
    wchar_t szFile[260];
    szFile[0] = L'\0';

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);
    ofn.lpstrFilter = L"S&SASR Archive (*.xpac)\0*.xpac\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameW(&ofn) == TRUE) {
        return wts(szFile);

    } else {
        //error bla bla bla
    }

    return "0";
}

std::string GetLocationGpac() {
    OPENFILENAMEW ofn;
    wchar_t szFile[260];
    szFile[0] = L'\0';

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);
    ofn.lpstrFilter = L"GC's Xpac Repack Info (*.gpac)\0*.gpac\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameW(&ofn) == TRUE) {
        return wts(szFile);

    } else {
        //error bla bla bla
    }

    return "0";
}

void XPACUtility::ConditionalFree(const bool& condition, void* freemem) {
    while (!condition) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    free(freemem);
}

std::wstring stw(const std::string& s) {
    std::wstring wTmp(s.begin(), s.end());
    return wTmp;
}
