#ifndef CPUMONITOR_H
#define CPUMONITOR_H

#include <string>
#include <vector>

struct CPU {
    float frequency;
    float frequencyMax;
    short nbrCPU;
    std::vector<float> usagePerCore;
};

class CpuMonitor {
private:
    CPU cpu;
    std::vector<unsigned long long> prevTotal, prevIdle;

    bool readCpuStats(std::vector<unsigned long long>& totalTimes, std::vector<unsigned long long>& idleTimes);
    float calculateCpuUsage(int index, unsigned long long total, unsigned long long idle);

public:
    CpuMonitor();

    bool update();

    float getCpuUsage();                     // Moyenne globale
    float getCpuFreq();                      // Fréquence actuelle
    std::string getCpuInfo();                // Infos détaillées

    const std::vector<float>& getUsagePerCore() const;
    float getMaxFrequency() const;
    short getCpuCount() const;
};

#endif // CPUMONITOR_H
