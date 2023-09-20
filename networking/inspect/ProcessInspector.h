#ifndef ZHENGQI_CPP_NETWORKING_INSPECT_PROCESSINSPECTOR_H
#define ZHENGQI_CPP_NETWORKING_INSPECT_PROCESSINSPECTOR_H

#include "networking/inspect/Inspector.h"
#include "utility/noncopyable.h"

namespace zhengqi {
namespace networking {
class ProcessInspector : utility::noncopyable {

public:
  void registerCommands(Inspector *ins);
  static std::string overview(HttpRequest::Method, const Inspector::ArgList &);
  static std::string pid(HttpRequest::Method, const Inspector::ArgList &);
  static std::string procStatus(HttpRequest::Method,
                                const Inspector::ArgList &);
  static std::string openedFiles(HttpRequest::Method,
                                 const Inspector::ArgList &);
  static std::string threads(HttpRequest::Method, const Inspector::ArgList &);

  static std::string username_;
};
} // namespace networking
} // namespace zhengqi

#endif // !ZHENGQI_CPP_NETWORKING_INSPECT_PROCESSINSPECTOR_H
