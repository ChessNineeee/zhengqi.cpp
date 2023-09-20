#include "networking/inspect/SystemInspector.h"
#include "utility/FileUtil.h"

#include <sys/utsname.h>

using namespace zhengqi::utility;
using namespace zhengqi::networking;

namespace zhengqi {
namespace inspect {
std::string uptime(Timestamp now, Timestamp start, bool showMicroseconds);
long getLong(const string &content, const char *key);
int stringPrintf(string *out, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));
} // namespace inspect
} // namespace zhengqi

using namespace zhengqi::inspect;

void SystemInspector::registerCommands(Inspector *ins) {
  ins->add("sys", "overview", SystemInspector::overview,
           "print system overview");
  ins->add("sys", "loadavg", SystemInspector::loadavg, "print /proc/loadavg");
  ins->add("sys", "version", SystemInspector::version, "print /proc/version");
  ins->add("sys", "cpuinfo", SystemInspector::cpuinfo, "print /proc/cpuinfo");
  ins->add("sys", "meminfo", SystemInspector::meminfo, "print /proc/meminfo");
  ins->add("sys", "stat", SystemInspector::stat, "print /proc/stat");
}

std::string SystemInspector::loadavg(HttpRequest::Method,
                                     const Inspector::ArgList &) {
  std::string loadavg;
  FileUtil::readFile("/proc/loadavg", 65536, &loadavg);
  return loadavg;
}

std::string SystemInspector::version(HttpRequest::Method,
                                     const Inspector::ArgList &) {
  std::string version;
  FileUtil::readFile("/proc/version", 65536, &version);
  return version;
}

std::string SystemInspector::cpuinfo(HttpRequest::Method,
                                     const Inspector::ArgList &) {
  std::string cpuinfo;
  FileUtil::readFile("/proc/cpuinfo", 65536, &cpuinfo);
  return cpuinfo;
}

std::string SystemInspector::meminfo(HttpRequest::Method,
                                     const Inspector::ArgList &) {
  std::string meminfo;
  FileUtil::readFile("/proc/meminfo", 65536, &meminfo);
  return meminfo;
}

std::string SystemInspector::stat(HttpRequest::Method,
                                  const Inspector::ArgList &) {
  std::string stat;
  FileUtil::readFile("/proc/stat", 65536, &stat);
  return stat;
}

std::string SystemInspector::overview(HttpRequest::Method,
                                      const Inspector::ArgList &) {
  std::string result;
  result.reserve(1024);

  Timestamp now = Timestamp::now();
  result += "Page generated at ";
  result += now.toFormattedString();
  result += " (UTC)\n";

  {
    struct utsname un;
    if (::uname(&un) == 0) {
      stringPrintf(&result, "Hostname: %s\n", un.nodename);
      stringPrintf(&result, "Machine: %s\n", un.machine);
      stringPrintf(&result, "OS: %s %s %s\n", un.sysname, un.release,
                   un.version);
    }
  }

  std::string stat;
  FileUtil::readFile("/proc/stat", 65536, &stat);

  Timestamp bootTime(Timestamp::kMicroSecondsPerSecond *
                     getLong(stat, "btime "));
  result += "Boot time: ";
  result += bootTime.toFormattedString(false /* show microseconds */);
  result += " (UTC)\n";
  result += "Up time: ";
  result += uptime(now, bootTime, false /* show microseconds */);
  result += "\n";

  // CPU load
  {
    std::string loadavg;
    FileUtil::readFile("/proc/loadavg", 65536, &loadavg);
    stringPrintf(&result, "Processes created: %ld\n",
                 getLong(stat, "processes "));
    stringPrintf(&result, "Loadavg: %s\n", loadavg.c_str());
  }

  // Memory
  {
    std::string meminfo;
    FileUtil::readFile("/proc/meminfo", 65536, &meminfo);
    long total_kb = getLong(meminfo, "MemTotal:");
    long free_kb = getLong(meminfo, "MemFree:");
    long buffers_kb = getLong(meminfo, "Buffers:");
    long cached_kb = getLong(meminfo, "Cached:");
    stringPrintf(&result, "Total Memory: %6ld MiB\n", total_kb / 1024);
    stringPrintf(&result, "Free Memory:  %6ld MiB\n", free_kb / 1024);
    stringPrintf(&result, "Buffers:      %6ld MiB\n", buffers_kb / 1024);
    stringPrintf(&result, "Cached:       %6ld MiB\n", cached_kb / 1024);
    stringPrintf(&result, "Real Used:    %6ld MiB\n",
                 (total_kb - free_kb - buffers_kb - cached_kb) / 1024);
    stringPrintf(&result, "Real Free:    %6ld MiB\n",
                 (free_kb + buffers_kb + cached_kb) / 1024);
  }
  // Swap
  // Disk
  // Network
  return result;
}
