#include "CpuMonitor.h"
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <cmath>
#include <algorithm>

CpuMonitor::CpuMonitor() {
    update();
}

bool CpuMonitor::readCpuStats(std::vector<unsigned long long>& totalTimes, std::vector<unsigned long long>& idleTimes) {
    std::ifstream file("/proc/stat");
    std::string line;
    totalTimes.clear();
    idleTimes.clear();

    while (std::getline(file, line)) {
        if (line.substr(0, 3) != "cpu")
            break;

        std::istringstream iss(line);
        std::string label;
        unsigned long long user, nice, system, idle, iowait, irq, softirq, steal = 0;
        iss >> label >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;

        unsigned long long total = user + nice + system + idle + iowait + irq + softirq + steal;
        totalTimes.push_back(total);
        idleTimes.push_back(idle + iowait);
    }

    return !totalTimes.empty();
}

float CpuMonitor::calculateCpuUsage(int index, unsigned long long total, unsigned long long idle) {
    unsigned long long deltaTotal = total - prevTotal[index];
    unsigned long long deltaIdle = idle - prevIdle[index];

    prevTotal[index] = total;
    prevIdle[index] = idle;

    if (deltaTotal == 0) return 0.0f;

    return 100.0f * (1.0f - static_cast<float>(deltaIdle) / deltaTotal);
}

bool CpuMonitor::update() {
    std::vector<unsigned long long> totalTimes, idleTimes;

    if (!readCpuStats(totalTimes, idleTimes))
        return false;

    if (cpu.usagePerCore.empty()) {
        cpu.nbrCPU = totalTimes.size() - 1;
        cpu.usagePerCore.resize(cpu.nbrCPU);
        prevTotal.resize(cpu.nbrCPU + 1);
        prevIdle.resize(cpu.nbrCPU + 1);
    }

    for (size_t i = 1; i < totalTimes.size(); ++i) {
        cpu.usagePerCore[i - 1] = calculateCpuUsage(i, totalTimes[i], idleTimes[i]);
    }

    // Calcul fréquence actuelle
    std::ifstream freqFile("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq");
    if (freqFile.is_open()) {
        float freqKHz;
        freqFile >> freqKHz;
        cpu.frequency = freqKHz / 1000.0f; // Convertir en MHz
    }

    // Calcul fréquence max
    std::ifstream maxFreqFile("/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq");
    if (maxFreqFile.is_open()) {
        float maxFreqKHz;
        maxFreqFile >> maxFreqKHz;
        cpu.frequencyMax = maxFreqKHz / 1000.0f;
    }

    return true;
}

float CpuMonitor::getCpuUsage() {
    if (cpu.usagePerCore.empty()) return 0.0f;

    float sum = 0.0f;
    for (float usage : cpu.usagePerCore) {
        sum += usage;
    }
    return sum / cpu.usagePerCore.size();
}

float CpuMonitor::getCpuFreq() {
    return cpu.frequency;
}

std::string CpuMonitor::getCpuInfo() {
    std::ifstream file("/proc/cpuinfo");
    std::string line, info;

    while (std::getline(file, line)) {
        if (line.find("model name") != std::string::npos ||
            line.find("cpu cores") != std::string::npos ||
            line.find("vendor_id") != std::string::npos) {
            info += line + "\n";
        }
    }

    return info;
}

const std::vector<float>& CpuMonitor::getUsagePerCore() const {
    return cpu.usagePerCore;
}

float CpuMonitor::getMaxFrequency() const {
    return cpu.frequencyMax;
}

short CpuMonitor::getCpuCount() const {
    return cpu.nbrCPU;
}
