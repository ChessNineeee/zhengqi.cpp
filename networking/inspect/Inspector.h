#ifndef ZHENGQI_CPP_NETWORKING_INSPECT_INSPECTOR_H
#define ZHENGQI_CPP_NETWORKING_INSPECT_INSPECTOR_H

#include "networking/http/HttpRequest.h"
#include "networking/http/HttpServer.h"
#include "utility/MutexLock.h"
#include "utility/noncopyable.h"

#include <functional>
#include <map>
#include <memory>
#include <vector>

namespace zhengqi {
namespace networking {

class ProcessInspector;
class PerformanceInspector;
class SystemInspector;

class Inspector : utility::noncopyable {
public:
  typedef std::vector<std::string> ArgList;
  typedef std::function<std::string(HttpRequest::Method, const ArgList &args)>
      Callback;
  Inspector(EventLoop *loop, const InetAddress &httpAddr,
            const std::string &name);
  ~Inspector();

  void add(const std::string &module, const std::string &command,
           const Callback &cb, const std::string &help);
  void remove(const std::string &module, const std::string &command);

private:
  typedef std::map<std::string, Callback> CommandList;
  typedef std::map<std::string, std::string> HelpList;

  void start();
  void onRequest(const HttpRequest &req, HttpResponse *resp);

  HttpServer server_;
  std::unique_ptr<ProcessInspector> processInspector_;
  std::unique_ptr<PerformanceInspector> performanceInspector_;
  std::unique_ptr<SystemInspector> systemInspector_;
  utility::MutexLock mutex_;
  std::map<std::string, CommandList> modules_;
  std::map<std::string, HelpList> helps_;
};
} // namespace networking
} // namespace zhengqi

#endif // !DEBUG
