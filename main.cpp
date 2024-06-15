#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <iostream>
#include <iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")

DWORD GetProcessIdByName(const std::wstring &processName) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to create snapshot of processes." << std::endl;
        return 0;
    }
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(hSnapshot, &pe32)) {
        CloseHandle(hSnapshot);
        std::cerr << "Failed to retrieve information about the first process." << std::endl;
        return 0;
    }
    do {
        std::wstring exeFileName = std::wstring(pe32.szExeFile, pe32.szExeFile + strlen(pe32.szExeFile));
        if (exeFileName == processName) {
            CloseHandle(hSnapshot);
            return pe32.th32ProcessID;
        }
    } while (Process32Next(hSnapshot, &pe32));
    CloseHandle(hSnapshot);
    return 0;
}

void SuspendProcess(DWORD processId, DWORD networkThreadId) {
    HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, networkThreadId);
    if (hThread != NULL) {
        SuspendThread(hThread);
        CloseHandle(hThread);
        std::cout << "Network thread suspended: " << networkThreadId << std::endl;
    } else {
        std::cerr << "Failed to suspend network thread: " << networkThreadId << std::endl;
    }
}

void ResumeProcess(DWORD processId, DWORD networkThreadId) {
    HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, networkThreadId);
    if (hThread != NULL) {
        ResumeThread(hThread);
        CloseHandle(hThread);
        std::cout << "Network thread resumed: " << networkThreadId << std::endl;
    } else {
        std::cerr << "Failed to resume network thread: " << networkThreadId << std::endl;
    }
}



int main() {
    std::wstring processName = L"javaw.exe";
    DWORD PID = GetProcessIdByName(processName);
    if (PID == 0) {
        std::wcerr << "Failed to find process: " << processName << std::endl;
        return 1;
    } else {
        std::wcout << "PID is:  " << processName << ": " << PID << std::endl;
    }
    DWORD networkThreadId = 25724; // Network TID
    bool isSuspended = false;

    while (true) {
        if (GetAsyncKeyState(VK_DELETE) & 0x8000) { // VK_DELETE -  DELETE
            if (!isSuspended) {
                SuspendProcess(PID, networkThreadId);
                isSuspended = true;
            }
        } else {
            if (isSuspended) {
                ResumeProcess(PID, networkThreadId);
                isSuspended = false;
            }
        }
        Sleep(100);
    }
   return 0;
}