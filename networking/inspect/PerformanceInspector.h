#ifndef ZHENGQI_CPP_NETWORKING_INSPECT_PERFORMANCEINSPECTOR_H
#define ZHENGQI_CPP_NETWORKING_INSPECT_PERFORMANCEINSPECTOR_H

#include "networking/inspect/Inspector.h"
#include "utility/noncopyable.h"

namespace zhengqi {
namespace networking {
class PerformanceInspector : utility::noncopyable {
public:
  void registerCommands(Inspector *ins);
  static std::string heap(HttpRequest::Method, const Inspector::ArgList &);
  static std::string growth(HttpRequest::Method, const Inspector::ArgList &);
  static std::string profile(HttpRequest::Method, const Inspector::ArgList &);
  static std::string cmdline(HttpRequest::Method, const Inspector::ArgList &);
  static std::string memstats(HttpRequest::Method, const Inspector::ArgList &);
  static std::string memhistogram(HttpRequest::Method,
                                  const Inspector::ArgList &);
  static std::string releaseFreeMemory(HttpRequest::Method,
                                       const Inspector::ArgList &);
  static std::string symbol(HttpRequest::Method, const Inspector::ArgList &);
};
} // namespace networking
} // namespace zhengqi

#endif // !ZHENGQI_CPP_NETWORKING_INSPECT_PERFORMANCEINSPECTOR_H
