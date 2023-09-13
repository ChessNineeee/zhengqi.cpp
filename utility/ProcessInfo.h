//
// Created by 70903 on 2023/9/7.
//

#ifndef ZHENGQI_CPP_PROCESSINFO_H
#define ZHENGQI_CPP_PROCESSINFO_H

#include "utility/StringPiece.h"
#include "utility/Timestamp.h"
#include "utility/Types.h"
#include <sys/types.h>
#include <vector>

namespace zhengqi {
namespace utility {
namespace ProcessInfo {
pid_t pid();
string pidString();
uid_t uid();
string username();
uid_t euid();
Timestamp startTime();
int clockTicksPerSecond();
int pageSize();
bool isDebugBuild(); // constexpr

string hostname();
string procname();
StringPiece procname(const string &stat);

/// read /proc/self/status
string procStatus();

/// read /proc/self/stat
string procStat();

/// read /proc/self/task/tid/stat
string threadStat();

/// readlink /proc/self/exe
string exePath();

int openedFiles();
int maxOpenFiles();

struct CpuTime {
  double userSeconds;
  double systemSeconds;

  CpuTime() : userSeconds(0.0), systemSeconds(0.0) {}

  double total() const { return userSeconds + systemSeconds; }
};
CpuTime cpuTime();

int numThreads();
std::vector<pid_t> threads();
} // namespace ProcessInfo
} // namespace utility
} // namespace zhengqi

#endif // ZHENGQI_CPP_PROCESSINFO_H
