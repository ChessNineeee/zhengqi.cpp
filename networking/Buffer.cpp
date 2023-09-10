#include "networking/Buffer.h"
#include "networking/SocketsOps.h"
#include "utility/Types.h"

#include <sys/uio.h>

using namespace zhengqi::utility;
using namespace zhengqi::networking;

const char Buffer::kCRLF[] = "\r\n";

const size_t Buffer::kCheapPrepend;
const size_t Buffer::kInitialSize;

ssize_t Buffer::readFd(int fd, int *savedErrno) {
  char extraBuf[65536];
  struct iovec vec[2];
  const size_t writable = writableBytes();
  vec[0].iov_base = begin() + writerIndex_;
  vec[0].iov_len = writable;
  vec[1].iov_base = extraBuf;
  vec[1].iov_len = sizeof extraBuf;

  // readv 操作的数组数目
  const int iovcnt = (writable < sizeof extraBuf) ? 2 : 1;
  const ssize_t n = sockets::readv(fd, vec, iovcnt);

  if (n < 0) {
    *savedErrno = errno;
  }

  else if (implicit_cast<size_t>(n) <= writable) {
    writerIndex_ += n;
  } else {
    writerIndex_ = buffer_.size();
    append(extraBuf, n - writable);
  }

  return n;
}
