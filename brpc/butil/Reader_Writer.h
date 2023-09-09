#ifndef ZHENGQI_CPP_BRPC_READER_WRITER_H
#define ZHENGQI_CPP_BRPC_READER_WRITER_H

#include <sys/uio.h>

namespace brpc {
namespace butil {

class IReader {
public:
  virtual ~IReader() {}

  virtual ssize_t ReadV(const iovec *iov, int iovcnt) = 0;
};

class IWriter {

public:
  virtual ~IWriter() {}

  virtual ssize_t WriteV(const iovec *iov, int iovcnt) = 0;
};
} // namespace butil
} // namespace brpc

#endif // !ZHENGQI_CPP_BRPC_READER_WRITER_H
