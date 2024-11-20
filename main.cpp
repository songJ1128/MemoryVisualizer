#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <vector>
#include <psapi.h>
#define TH32CS_PROCESS 0x00000002


struct MemoryRegion {
    PVOID baseAddress;
    SIZE_T regionSize;
    DWORD state;
};

struct ProcessInfo {
    std::string processName;
    DWORD processID;
    SIZE_T workingSetSize;
    std::vector<MemoryRegion> memoryRegions;
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
                PROCESS_MEMORY_COUNTERS_EX pmc;
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

void getMemoryRegions(ProcessInfo& processInfo, HANDLE hProcess) {
    MEMORY_BASIC_INFORMATION mbi;
    BYTE* address = nullptr;

    while (VirtualQueryEx(hProcess, address, &mbi, sizeof(mbi)) == sizeof(mbi)) {
        MemoryRegion region;
        region.baseAddress = mbi.BaseAddress;
        region.regionSize = mbi.RegionSize;
        region.state = mbi.State;
        processInfo.memoryRegions.push_back(region);
        address += mbi.RegionSize;
    }
}

int main() {
    std::vector<ProcessInfo> processes = getProcessList();

    for (const auto& process : processes) {
        std::cout << "Process Name: " << process.processName << ", Process ID: " << process.processID << ", Working Set Size: " << process.workingSetSize << " bytes" << std::endl;
        for (const auto& region : process.memoryRegions) {
            std::cout << "  Base Address: " << region.baseAddress << ", Region Size: " << region.regionSize << " bytes, State: " << region.state << std::endl;
        }
    }

    return 0;
}
