#include "dllloader.h"

#include <memory>
#include <mutex>
#include <unordered_map>

#include "fsrunityplugin.h"


DllLoader& DllLoader::Instance(const wchar_t* path)
{
    static std::unordered_map<std::wstring, std::unique_ptr<DllLoader>> dllMap;
    static std::mutex criticalSection;
    std::lock_guard<std::mutex> lock(criticalSection);
    if (dllMap.find(path) == dllMap.end()) {
        dllMap[path].reset(new DllLoader);
        bool res = dllMap[path]->LoadDll(path);
        if (!res) {
            FSR_ERROR("Failed to load dll");
        }
    }
    return *dllMap[path];
}

FARPROC DllLoader::GetProcAddress(LPCSTR procName)
{
    return ::GetProcAddress(m_DLL, procName);
}

inline HMODULE SearchAndLoadDll(const std::wstring& dir, const std::wstring& dllName)
{
    HMODULE hModule = NULL;
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW((dir + L"\\*").c_str(), &findData);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (wcscmp(findData.cFileName, L".") == 0 || wcscmp(findData.cFileName, L"..") == 0)
                continue;
            std::wstring path = dir + L"\\" + findData.cFileName;
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                hModule = SearchAndLoadDll(path, dllName);
            } else if (_wcsicmp(findData.cFileName, dllName.c_str()) == 0) {
                hModule = LoadLibraryExW(path.c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
                if (!hModule) {
                    //DWORD error = GetLastError();
                    FSR_ERROR("LoadLibraryExW failed");
                }
            }
            if (hModule) {
                break;
            }
        } while (FindNextFileW(hFind, &findData));
        FindClose(hFind);
    }
    return hModule;
}

bool DllLoader::LoadDll(const wchar_t* path)
{
    if (path != nullptr && wcscmp(m_Path.c_str(), path) != 0) {
        Release();
        m_Path = path;
        m_DLL = SearchAndLoadDll(L".\\", m_Path);
        return m_DLL != NULL;
    }
    return false;
}

uint32_t DllLoader::Release()
{
    BOOL result = -1;
    if (m_DLL) {
        result = FreeLibrary(m_DLL);
        if (!result) {
            DWORD error = GetLastError();
            FSR_ERROR("DLL Release failed");
            return error;
        }
    }
    return result;
}