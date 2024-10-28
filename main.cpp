#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <vector>

#define TH32CS_PROCESS 0x00000002

struct ProcessInfo {
    std::string processName;
    DWORD processID;
    SIZE_T workingSetSize;
};

std::vector<ProcessInfo> getProcessList() {
    std::vector<ProcessInfo> processes;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_PROCESS, 0);

    if (hSnapshot == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to create process snapshot." << std::endl;
        return processes;
    }

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(pe);

    if (Process32First(hSnapshot, &pe)) {
        do {
            ProcessInfo info;
            info.processName = pe.szExeFile;
            info.processID = pe.th32ProcessID;

            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, info.processID);
            if (hProcess) {
                PROCESS_MEMORY_COUNTERS pmc;
                if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
                    info.workingSetSize = pmc.WorkingSetSize;
                    processes.push_back(info);
                }
                CloseHandle(hProcess);
            }
        } while (Process32Next(hSnapshot, &pe));
    }

    CloseHandle(hSnapshot);
    return processes;
}