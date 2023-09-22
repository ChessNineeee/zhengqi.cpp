#include "networking-examples/curl/Curl.h"
#include "networking/EventLoop.h"
#include "utility/Logging.h"
#include <sstream>
#include <stdio.h>
#include <vector>

using namespace zhengqi::utility;
using namespace zhengqi::networking;
using namespace zhengqi::curl;

typedef std::shared_ptr<FILE> FilePtr;

template <int N> bool startWith(const string &str, const char (&prefix)[N]) {
  return str.size() >= N - 1 && std::equal(prefix, prefix + N - 1, str.begin());
}

class Piece : noncopyable {
public:
  Piece(const RequestPtr &req, const FilePtr &out, const string &range,
        std::function<void()> done)
      : req_(req), out_(out), range_(range), doneCb_(std::move(done)) {
    LOG_INFO << "range: " << range;
    req->setRange(range);
    req->setDataCallback(std::bind(&Piece::onData, this, _1, _2));
    req->setDoneCallback(std::bind(&Piece::onDone, this, _1, _2));
  }

private:
  void onData(const char *data, int len) {
    ::fwrite(data, 1, static_cast<size_t>(len), get_pointer(out_));
  }

  void onDone(Request *c, int code) {
    LOG_INFO << "[" << range_ << "] is done";
    req_.reset();
    out_.reset();
    doneCb_();
  }

  RequestPtr req_;
  FilePtr out_;
  string range_;
  std::function<void()> doneCb_;
};

class Downloader : noncopyable {
public:
  Downloader(EventLoop *loop, const string &url)
      : loop_(loop), curl_(loop_), url_(url), req_(curl_.getUrl(url_)),
        found_(false), acceptRanges_(false), length_(0), pieces_(kConcurrent),
        concurrent_(0) {}

private:
  void onHeader(const char *data, int len) {

    string line(data, static_cast<size_t>(len));
    if (startWith(line, "HTTP/1.1 200") || startWith(line, "HTTP/1.0 200")) {
      found_ = true;
    }
    if (line == "Accept-Ranges: bytes\r\n") {
      acceptRanges_ = true;
      LOG_DEBUG << "Accept-Ranges";
    } else if (startWith(line, "Content-Length:")) {
      length_ = atoll(line.c_str() + strlen("Content-Length:"));
      LOG_INFO << "Content-Length: " << length_;
    }
  }

  void onHeaderDone(Request *c, int code) {
    LOG_DEBUG << code;

    if (acceptRanges_ && length_ >= kConcurrent * 4096) {
      LOG_INFO << "Downloading with " << kConcurrent << " connections";
      concurrent_ = kConcurrent;
      concurrentDownload();
    }
  }
  EventLoop *loop_;
  Curl curl_;
  string url_;
  RequestPtr req_;
  RequestPtr req2_;
  bool found_;
  bool acceptRanges_;
  int64_t length_;
  FilePtr out_;
  std::vector<std::unique_ptr<Piece>> pieces_;
  int concurrent_;

  const static int kConcurrent = 4;
};
