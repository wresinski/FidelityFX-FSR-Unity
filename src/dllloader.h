#pragma once

#include <string>

#include <windows.h>


class DllLoader
{
public:
    static DllLoader& Instance(const wchar_t* path);

public:
    ~DllLoader() { Release(); }
    FARPROC GetProcAddress(LPCSTR procName);

protected:
    DllLoader() {}

private:
    DllLoader(const DllLoader&) = delete;
    DllLoader& operator=(const DllLoader&) = delete;
    DllLoader(const DllLoader&&) = delete;
    DllLoader& operator=(const DllLoader&&) = delete;

private:
    bool LoadDll(const wchar_t* path);
    uint32_t Release();

private:
    std::wstring m_Path = {};
    HMODULE m_DLL = NULL;
};