#ifndef ZHENGQI_CPP_NETWORKING_INSPECT_SYSTEMINSPECTOR_H
#define ZHENGQI_CPP_NETWORKING_INSPECT_SYSTEMINSPECTOR_H

#include "networking/inspect/Inspector.h"

namespace zhengqi {
namespace networking {

class SystemInspector : utility::noncopyable {
public:
  void registerCommands(Inspector *ins);

  static std::string overview(HttpRequest::Method, const Inspector::ArgList &);
  static std::string loadavg(HttpRequest::Method, const Inspector::ArgList &);
  static std::string version(HttpRequest::Method, const Inspector::ArgList &);
  static std::string cpuinfo(HttpRequest::Method, const Inspector::ArgList &);
  static std::string meminfo(HttpRequest::Method, const Inspector::ArgList &);
  static std::string stat(HttpRequest::Method, const Inspector::ArgList &);
};
} // namespace networking
} // namespace zhengqi

#endif // !ZHENGQI_CPP_NETWORKING_INSPECT_SYSTEMINSPECTOR_H
