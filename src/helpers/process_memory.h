#pragma once

#include <iostream>

#ifdef __linux__
#include <fstream>
#include <unistd.h>

void getMemoryUsage(size_t& vm_usage, size_t& resident_set) {
    vm_usage = 0;
    resident_set = 0;

    std::ifstream statm("/proc/self/statm");
    if (statm) {
        size_t size, resident, share, text, lib, data, dt;
        statm >> size >> resident >> share >> text >> lib >> data >> dt;

        long page_size = sysconf(_SC_PAGESIZE); // in bytes
        vm_usage = size * page_size;
        resident_set = resident * page_size;
    } else {
        std::cerr << "Failed to open /proc/self/statm" << std::endl;
    }
}

#elif _WIN32
#include <windows.h>
#include <psapi.h>

void getMemoryUsage(size_t& vm_usage, size_t& resident_set) {
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        vm_usage = pmc.WorkingSetSize;
        resident_set = pmc.PagefileUsage;
    } else {
        std::cerr << "Failed to get process memory info" << std::endl;
    }
}

#elif __APPLE__
#include <mach/mach.h>

void getMemoryUsage(size_t& vm_usage, size_t& resident_set) {
    vm_usage = 0;
    resident_set = 0;

    task_t task = MACH_PORT_NULL;
    struct task_basic_info t_info;
    mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;

    if (task_for_pid(current_task(), getpid(), &task) != KERN_SUCCESS) {
        std::cerr << "Failed to get task for process" << std::endl;
        return;
    }

    if (task_info(task, TASK_BASIC_INFO, (task_info_t)&t_info, &t_info_count) != KERN_SUCCESS) {
        std::cerr << "Failed to get task info" << std::endl;
        return;
    }

    vm_usage = t_info.virtual_size;      // Virtual memory size
    resident_set = t_info.resident_size; // Resident set size
}

#else
#error "Platform not supported"
#endif
