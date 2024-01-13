#include "xpac_unpacker.hpp"

XPAC::XPAC(const std::string* folder) {
    xu = new XPACUtility(folder);
    loadUnHasher();
}

XPAC::~XPAC() {
    delete xu;
}

std::string XPAC::Unpack(const char* xpacFileName) {
    delete em;
    em = new ExecutionMonitor();
    em->start("f");

    cFile = path(xpacFileName).filename().string();
    cFile.erase(cFile.end() - 5, cFile.end());
    cFile += '/';

    std::string filename = *xu->folder + cFile.substr(0, cFile.length() - 1) + ".gpac";
    std::ifstream xpacFile(xpacFileName, std::ios::in | std::ios::binary);

    if (!xpacFile.is_open()) {
        return "Error opening XPAC file!";
    }

    XPACHeader header;
    xpacFile.read(reinterpret_cast<char*>(&header), sizeof(XPACHeader));

    //std::vector<XPACEntry> entries(header.dwTotalFiles);
    size_t s = sizeof(XPACEntry) * header.dwTotalFiles;
    XPACEntry* entries = (XPACEntry*)malloc(s);
    xpacFile.read(reinterpret_cast<char*>(entries), s);

    int cdecomp = 0, cdds = 0;

    std::vector<Bytef*> fileData;

    size_t total = header.dwTotalFiles;
    for (int i = 0; i < total; ++i) {

        xpacFile.seekg(entries[i].dwOffset, std::ios::beg);
        size_t csize = entries[i].dwCompressedSize;
        //fileData.emplace_back((Bytef*)malloc(csize));
        fileData.emplace_back(new Bytef[csize]);
        xpacFile.read(reinterpret_cast<char*>(fileData[i]), csize);
        std::string unhashn = "";
        try {
            unhashn = unhashmap[std::to_string(entries[i].dwHash)];
        } Catchd {
            unhashn = "";
        }
        
        activeThreads++;
        threads.emplace_back(&XPAC::DecompThreaded, this, fileData[i], entries[i], unhashn);
        threads.back().detach();
    }

    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    em->toggle("f");

    while (activeThreads != 0) {
        Sleep(1);
    }
     filename = *xu->folder + cFile.substr(0, cFile.length() - 1) + ".gpac";

    uXPACEntry* uEntries = (uXPACEntry*)malloc(sizeof(uXPACEntry) * header.dwTotalFiles);


    std::ofstream gpac(filename, std::ios::out | std::ios::binary);

    if (!gpac.is_open()) {
        return "Error opening output GPAC file!";
    }

    gpac.write(reinterpret_cast<char*>(&header), sizeof(XPACHeader));

    for (size_t i = 0; i < header.dwTotalFiles; ++i) {

        uEntries[i].isCompressed = uisCompressed[entries[i].dwHash];

        uEntries[i].hasTextures = uhasTextures[entries[i].dwHash];
        uEntries[i].textures = utextures[entries[i].dwHash];
        uEntries[i].textureOffsets = utextureOffsets[entries[i].dwHash];

        uEntries[i].dwHash = entries[i].dwHash;
        uEntries[i].dwOffset = entries[i].dwOffset;
        uEntries[i].dwDecompressedSize = entries[i].dwDecompressedSize;
        uEntries[i].dwCompressedSize = entries[i].dwCompressedSize;
        uEntries[i].dwNull = entries[i].dwNull;

        gpac.write(reinterpret_cast<char*>(&uEntries[i]), sizeof(uXPACEntry));

        std::string nameofit = unhashmap[std::to_string(uEntries[i].dwHash)];

        if (uEntries[i].hasTextures) {
            for (int j = 0; j < uEntries[i].textures + 1; j++) {
                gpac.write(reinterpret_cast<char*>(&uEntries[i].textureOffsets[j]), sizeof(uint32_t));
            }
        }

    }

    gpac.close();

    free(uEntries);

    xpacFile.close();
    free(entries);
    return em->printAllTimes(path(xpacFileName).filename().string());
}

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

std::string XPAC::Repack(const char* gpacFileName) {
    delete em;
    em = new ExecutionMonitor();
    em->start("f");
    std::ifstream gpac2(gpacFileName, std::ios::in | std::ios::binary);

    if (!gpac2.is_open()) {
        if (debug) {
            std::cout << "Error opening GPAC file for reading!";
        }
        return "Error opening GPAC file for reading!";
    }

    XPACHeader header;
    gpac2.read(reinterpret_cast<char*>(&header), sizeof(XPACHeader));

    uXPACEntry* uEntries = new uXPACEntry[sizeof(uXPACEntry) * header.dwTotalFiles];
    XPACEntry* Entries = (XPACEntry*)malloc(sizeof(XPACEntry) * header.dwTotalFiles);
    //gpac2.read(reinterpret_cast<char*>(uEntries), sizeof(uXPACEntry) * header.dwTotalFiles);

    for (size_t i = 0; i < header.dwTotalFiles; ++i) {
        gpac2.read(reinterpret_cast<char*>(&uEntries[i]), sizeof(uXPACEntry));
        uEntries[i].textureOffsets = (uint32_t*)malloc((uEntries[i].textures + 1) * sizeof(uint32_t));

        if (uEntries[i].hasTextures) {
            for (int j = 0; j <= uEntries[i].textures; j++) {
                gpac2.read(reinterpret_cast<char*>(&uEntries[i].textureOffsets[j]), sizeof(uint32_t));
            }
        }
    }

    gpac2.close();

    for (size_t i = 0; i < header.dwTotalFiles; ++i) {
        Entries[i].dwHash             = uEntries[i].dwHash;
        Entries[i].dwOffset           = uEntries[i].dwOffset;
        Entries[i].dwDecompressedSize = uEntries[i].dwDecompressedSize;
        Entries[i].dwCompressedSize   = uEntries[i].dwCompressedSize;
        Entries[i].dwNull             = uEntries[i].dwNull;
    }

    std::ofstream xpac(*xu->folder + "\\out.xpac", std::ios::out | std::ios::binary);
    if (!xpac.is_open()) {
        "Error opening XPAC file for writing!";
        return "Error opening XPAC file for writing!";
    }
    xpac.write(reinterpret_cast<char*>(&header), sizeof(XPACHeader));
    xpac.write(reinterpret_cast<char*>(Entries), sizeof(XPACEntry) * header.dwTotalFiles);
    
    for (size_t i = 0; i < header.dwTotalFiles; ++i) {
        std::string filePathS = *xu->folder;
        std::string uhn = repackString(Entries[i].dwHash);
        filePathS += uhn;
        path filePath = filePathS;
        std::string fname = filePath.filename().string();
        fname.erase(fname.end() - 4, fname.end());
        path decPath;

        if (uEntries[i].hasTextures) {
            decPath = path(filePath.string().substr(0, filePath.string().length() - 4) + "_Textures\\" +  fname + "_dec.zig");
            if (rpDDSFiles) {
                if (!PackTextures(decPath, uEntries[i])) {
                    if (debug) {
                        std::cout << std::string(uhn) + " didn't Repack as Expected...";
                    }
                    return std::string(uhn) + " didn't Repack as Expected...";
                }
            }
            filePath = path(filePath.string().substr(0, filePath.string().length() - 4) + "_Textures\\" + fname + "_c.zig");
        } else {
            decPath = path(filePath.string().substr(0, filePath.string().length() - 4) + "_dec" + filePath.string().substr(filePath.string().length() - 4));
            filePath = path(filePath.string().substr(0, filePath.string().length() - 4) + "_c" + filePath.string().substr(filePath.string().length() - 4));
        }

        if (uEntries[i].isCompressed) {
            CompThreaded(decPath, filePath, uEntries[i].dwHash);
        }

        std::ifstream compfile(filePath, std::ios::in | std::ios::binary);
        if (!compfile.is_open()) {
            if (debug) {
                std::cout << "Error opening compressed file for reading! " + filePath.string() << std::endl;
            }
            return "Error opening compressed file for reading! " + filePath.string();
        }

        compfile.seekg(0, std::ios::end);
        size_t sizecomp = compfile.tellg();
        compfile.seekg(0, std::ios::beg);

        if (sizecomp > uEntries[i].dwDecompressedSize) {
            if (debug) {
                std::cout << std::string(uhn) + " File too big!";
            }
            return std::string(uhn) + " File too big!";
        }

        xpac.seekp(uEntries[i].dwOffset, std::ios::beg);
        xpac << compfile.rdbuf();

    }
    xpac.close();

    for (size_t i = 0; i < header.dwTotalFiles; ++i) {
        delete[] uEntries[i].textureOffsets;
    }

    delete[] uEntries;
    free(Entries);
    em->stop("f");
    return em->printAllTimes("gpac");
}

bool XPAC::PackTextures(path filePath, const uXPACEntry& entry) {

    std::ofstream zig(filePath, std::ios::binary | std::ios::in | std::ios::out);

    if (entry.textures == 0) {
        if (debug) {
            std::cout << "ERROR AT " << filePath << std::endl;
        }
        return false;
    }

    size_t firstoff = entry.textureOffsets[0];

    path par = filePath.parent_path();
    
    for (int i = 0; i < entry.textures; i++) {
        std::string ddsname = par.string() + "\\" + std::to_string(i) + ".dds";
        std::ifstream dds(ddsname, std::ios::binary);
        size_t begindds = entry.textureOffsets[i];
        size_t enddds = entry.textureOffsets[i + 1];

        size_t sizedds = enddds - begindds;

        if (enddds - begindds < sizedds) {
            if (debug) {
                std::cout << "ERROR at " << ddsname << std::endl;
            }
            return false;
        }

        zig.seekp(begindds, std::ios::beg);
        zig << dds.rdbuf();
        dds.close();

    }

    zig.close();
    return true;
}

void XPAC::CompThreaded(path decPath, path filePath, DWORD hash) {
    std::ifstream inputFile(decPath, std::ios::in | std::ios::binary);
    if (!inputFile.is_open()) {
        if (debug) {
            std::cout << "COULD NOT OPEN DEC PATH " << decPath << std::endl;
        }
        return;
    }

    // Read the file into a vector
    std::vector<char> input(std::istreambuf_iterator<char>(inputFile), {});

    // Compress the data
    std::vector<char> compressedData;
    Compress(input, compressedData);

    // Save the compressed data back to the file
    xu->save(filePath.string(), compressedData);

    inputFile.close();
}

bool XPAC::Compress(std::vector<char>& input, std::vector<char>& compressedData) {
    z_stream zlibStream;
    zlibStream.zalloc = Z_NULL;
    zlibStream.zfree = Z_NULL;
    zlibStream.opaque = Z_NULL;

    if (deflateInit(&zlibStream, Z_BEST_COMPRESSION) != Z_OK) {
        return false;
    }

    zlibStream.next_in = reinterpret_cast<Bytef*>(input.data());
    zlibStream.avail_in = static_cast<uInt>(input.size());

    int chunkSize = 4096;
    std::vector<char> tempBuffer(chunkSize);

    do {
        zlibStream.next_out = reinterpret_cast<Bytef*>(&tempBuffer[0]);
        zlibStream.avail_out = chunkSize;

        if (deflate(&zlibStream, Z_FINISH) == Z_STREAM_ERROR) {
            deflateEnd(&zlibStream);
            return false;
        }

        compressedData.insert(compressedData.end(), tempBuffer.begin(), tempBuffer.begin() + (chunkSize - zlibStream.avail_out));
    } while (zlibStream.avail_out == 0);

    deflateEnd(&zlibStream);

    return true;
}

bool XPAC::Decompress(Bytef* compressedData, size_t compressedSize, std::vector<char>& output) const {
    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    stream.avail_in = static_cast<uInt>(compressedSize);
    stream.next_in = compressedData;

    if (inflateInit(&stream) != Z_OK) {
        return false;
    }

    const size_t bufferSize = 1024;
    char buffer[bufferSize];

    do {
        stream.avail_out = bufferSize;
        stream.next_out = reinterpret_cast<Bytef*>(buffer);

        int ret = inflate(&stream, Z_NO_FLUSH);

        switch (ret) {
        case Z_STREAM_ERROR:
            inflateEnd(&stream);
            return false;
        case -3:
            inflateEnd(&stream);
            return false;
        }

        output.insert(output.end(), buffer, buffer + (bufferSize - stream.avail_out));
    } while (stream.avail_out == 0);

    inflateEnd(&stream);
    return true;
}

void XPAC::FindDDS(const std::vector<char>& data, std::vector<std::vector<char>>& ddsArray, const DWORD hash) {

    size_t offset = 0;
    size_t prevoffset = 0;
    bool first = 1;
    std::vector<size_t> tracker;
    while (offset < data.size()) {
        if (offset + headerSize <= data.size() && std::memcmp(&data[offset], &DDS_HEADER, headerSize) == 0) {
            if (first) {
                prevoffset = offset;
                tracker.push_back(offset);
                first = 0;
            } else {
                ddsArray.emplace_back(data.begin() + prevoffset, data.begin() + offset);
                prevoffset = offset;
                tracker.push_back(offset);
            }

        }

        offset += 4;
    }

    if (!first) {

        ddsArray.emplace_back(data.begin() + prevoffset, data.end());

        size_t sizet = ddsArray.size();
        utextureOffsets[hash] = new uint32_t[sizet+1];

        for (int i = 0; i < sizet; i++) {
            utextureOffsets[hash][i] = tracker[i];
        }
        utextureOffsets[hash][sizet] = data.size();
    }
}


void XPAC::DecompThreaded(Bytef* cData, const XPACEntry entry, std::string name) {
    std::vector<char> decomp;
    if (Decompress(cData, entry.dwDecompressedSize, decomp)) {
        uisCompressed[entry.dwHash] = true;

        std::vector<std::vector<char>> holdDDS;
        FindDDS(decomp, holdDDS, entry.dwHash);

        if (!holdDDS.empty()) {
            uhasTextures[entry.dwHash] = true;
            utextures[entry.dwHash] = holdDDS.size();

            std::string filename = "";
            if (name != "") {
                filename = path(name).filename().string();
                filename.erase(filename.end() - 4, filename.end());
                name.erase(name.begin(), name.begin() + 2);
            } else {
                std::stringstream ss;
                ss << "Unknown/" << cFile << std::hex << entry.dwHash << ".bin" << std::dec;
                name = ss.str();
            }

            xu->FixFolder(name);
            name = xu->CreateFolders(name);
            name.erase(name.end() - 4, name.end());
            name += "_Textures";
            xu->CreateFolder(name);
            name += "\\";
            int j = 0;
            for (auto it = holdDDS.begin(); it < holdDDS.end(); it++) {
                std::string outputFileName = name + std::to_string(j) + ".dds";
                CSaveThreaded(*it, outputFileName);
                j++;
            }
            xu->save(name + filename + "_dec.zig", decomp);
            if (upCompressedFiles) {
                std::ofstream of(name + filename + "_c.zig", std::ios::binary);
                of.write(reinterpret_cast<const char*>(cData), entry.dwDecompressedSize);
                of.close();
            }
            
        } else {
            uhasTextures[entry.dwHash] = false;
            utextures[entry.dwHash] = false;
            utextureOffsets[entry.dwHash] = nullptr;
            if (name != "") {
                name.erase(name.begin(), name.begin() + 2);
                size_t nlen = name.length();
                if (upCompressedFiles) {
                    name.insert(nlen - 4, 1, 'c');
                    name.insert(nlen - 4, 1, '_');
                    std::ofstream of(name, std::ios::binary);
                    of.write(reinterpret_cast<const char*>(cData), entry.dwDecompressedSize);
                    of.close();
                    name.insert(nlen - 5, 1, 'e');
                    name.insert(nlen - 5, 1, 'd');
                } else {
                    name.insert(nlen - 4, 1, 'c');
                    name.insert(nlen - 4, 1, 'e');
                    name.insert(nlen - 4, 1, 'd');
                    name.insert(nlen - 4, 1, '_');
                }
            } else {
                std::stringstream ss;
                ss << "Unknown/" << cFile << std::hex << entry.dwHash << ".bin" << std::dec;
                name = ss.str();
            }
            xu->FixFolder(name);
            name = xu->CreateFolders(name);
            xu->save(name, decomp);
        }
        delete cData;
    } else {
        uisCompressed[entry.dwHash] = false;
        uhasTextures[entry.dwHash] = false;
        utextures[entry.dwHash] = false;
        utextureOffsets[entry.dwHash] = nullptr;
        if (name != "") {
            name.erase(name.begin(), name.begin() + 2);
        } else {
            std::stringstream ss;
            ss << "Unknown/" << cFile << std::hex << entry.dwHash << ".bin" << std::dec;
            name = ss.str();
        }
        xu->FixFolder(name);
        name = xu->CreateFolders(name);
        std::ofstream of(name, std::ios::binary);
        of.write(reinterpret_cast<const char*>(cData), entry.dwDecompressedSize);
        of.close();
        delete cData;
    }
    {
        std::lock_guard<std::mutex> lock(mx);
        --activeThreads;
    }
}

void XPAC::CSaveThreaded(const std::vector<char>& data, std::string ddsFile) const {
    std::ofstream of(ddsFile, std::ios::binary);
    of.write(data.data(), data.size());
    of.close();
}

int XPAC::Hash(const char* pString) {
    int dwHash = 0;
    char pStringByte;

    for (unsigned int i = strlen(pString) - 1; (i & 0x80000000u) == 0; --i) {
        pStringByte = toupper(pString[i]);
        if (pStringByte == 47)
            pStringByte = 92;
        dwHash = pStringByte + 131 * dwHash;
    }
    return dwHash;
}

HMODULE GCM() {
    HMODULE hm = NULL;
    GetModuleHandleEx(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
        (LPCTSTR)GCM,
        &hm
    );
    return hm;
}

std::string XPAC::repackString(const DWORD hash) {
    std::string dname = unhashmap[std::to_string(hash)];
    if (dname[0] == '.') {
        dname = dname.substr(2);;
        dname.insert(0, "_");

        size_t pos = 0;
        pos = dname.find('/', pos);
        while (pos < dname.size()) {
            dname.insert(++pos, 1, '_');
            pos = dname.find('/', ++pos);
            if (dname.find('/', ++pos) == std::string::npos) {
                break;
            } else {
                --pos;
            }
        }

    }

    if (unhashmap[std::to_string(hash)].empty()) {
        std::stringstream ss;
        ss << std::hex << hash;
        return std::string("_Unknown/_" + cFile + ss.str() + ".bin").c_str();
    } else {
        return dname;
    }
}

void XPAC::loadUnHasher() {

    std::string mapfile;
    HRSRC hr = FindResource(GCM(), MAKEINTRESOURCE(GCTEXT), MAKEINTRESOURCE(TEXTFILE));
    HGLOBAL hd = LoadResource(GCM(), hr);
    DWORD hs = SizeofResource(GCM(), hr);
    char* hf = (char*)LockResource(hd);
    mapfile.assign(hf, hs);

    std::string f = "", s = "";

    std::string* c = &f;

    unhashmap;


    for (auto it = mapfile.begin(); it < mapfile.end(); it++) {
        if (*it == '\n') {
            continue;
        }
        if (*it == ':') {
            c = &s;
        } else if (*it == ';') {
            unhashmap[f] = s;
            f = "", s = "";
            c = &f;
        } else {
            c->push_back(*it);
        }
    }
}