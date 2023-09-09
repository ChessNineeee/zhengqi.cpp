//
// Created by 70903 on 2023/9/9.
//

#ifndef ZHENGQI_CPP_BRPC_IOBUF_H
#define ZHENGQI_CPP_BRPC_IOBUF_H

#include <cstdint> // uint32_t
#include <string>
#include <sys/types.h>
#include <sys/uio.h> // iovec

extern "C" {
struct const_iovec {
  const void *iov_base;
  size_t iov_len;
};

#ifndef USE_MESALINK
struct ssl_st;
#else
#define ssl_st MESALINK_SSL
#endif // !USE_MESALINK
}

namespace brpc {
namespace butil {
class IOBuf {
public:
  static const size_t DEFAULT_BLOCK_SIZE = 8192;
  static const size_t INITIAL_CAL = 32;
  struct Block;

  struct BlockRef {
    uint32_t offset;
    uint32_t length;
    Block *block;
  };

  struct SmallView {
    BlockRef refs[2];
  };

  struct BigView {
    int32_t magic;
    uint32_t start;
    BlockRef *refs;
    uint32_t nref;
    uint32_t cap_mask;
    size_t nbytes;

    const BlockRef &ref_at(uint32_t i) const {
      return refs[(start + i) & cap_mask];
    }

    BlockRef &ref_at(uint32_t i) { return refs[(start + i) & cap_mask]; }

    uint32_t capacity() const { return cap_mask + 1; }
  };

  struct Movable {
    explicit Movable(IOBuf &v) : _v(&v) {}
    IOBuf &value() const { return *_v; }

  private:
    IOBuf *_v;
  };

  typedef uint64_t Area;
  static const Area INVALID_AREA = 0;

  IOBuf();
  IOBuf(const IOBuf &);
  IOBuf(const Movable &);
  ~IOBuf() { clear(); }
  void operator=(const IOBuf &);
  void operator=(const Movable &);
  void operator=(const char *);
  void operator=(const std::string &);

  void swap(IOBuf &);

  size_t pop_front(size_t n);

  size_t pop_back(size_t n);

  size_t cutn(IOBuf *out, size_t n);
  size_t cutn(void *out, size_t n);
  size_t cutn(std::string *out, size_t n);

  bool cut1(void *c);

  int cut_until(IOBuf *out, char const *delim);

  int cut_until(IOBuf *out, const std::string &delim);

  ssize_t cut_into_writer(IWriter *writer, size_t size_hint = 1024 * 1024);

  ssize_t cut_into_file_descriptor(int fd, size_t size_hint = 1024 * 1024);

  ssize_t pcut_into_file_descriptor(int fd, off_t offset,
                                    size_t size_hint = 1024 * 1024);

  ssize_t cut_into_SSL_channel(struct ssl_st *ssl, int *ssl_error);

  static ssize_t cut_multiple_into_writer(IWriter *writer, IOBuf *const *pieces,
                                          size_t count);

  static ssize_t cut_multiple_into_file_descriptor(int fd, IOBuf *const *pieces,
                                                   size_t count);

  static ssize_t pcut_multiple_into_file_descriptor(int fd, off_t offset,
                                                    IOBuf *const *pieces,
                                                    size_t count);

  static ssize_t cut_multiple_into_SSL_channel(struct ssl_st *ssl,
                                               IOBuf *const *pieces,
                                               size_t count, int *ssl_error);

  void append(const IOBuf &other);

  void append(const Movable &other);

  int push_back(char c);

  int append(void const *data, size_t count);

  int appendv(const const_iovec vec[], size_t n);
  int appendv(const iovec *vec, size_t n) {
    return appendv(reinterpret_cast<const const_iovec *>(vec), n);
  }

  int append_user_data(void *data, size_t size, void (*deleter)(void *));

  int append_user_data_with_meta(void *data, size_t size,
                                 void (*deleter)(void *), uint64_t meta);

  uint64_t get_first_data_meta();

  int resize(size_t n) { return resize(n, '\0'); }

  int resize(size_t n, char c);

  Area reserve(size_t n);

  int unsafe_assign(Area area, const void *data);

  size_t append_to(IOBuf *buf, size_t n = static_cast<size_t>(-1L),
                   size_t pos = 0) const;

  size_t copy_to(IOBuf *buf, size_t n = static_cast<size_t>(-1L),
                 size_t pos = 0) const
#if defined(__GNUC__) &&                                                       \
    (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8))
      __attribute__((error("Call append_to(IOBuf*) instead")))
#endif
      ;

  size_t copy_to(void *buf, size_t n = static_cast<size_t>(-1L),
                 size_t pos = 0) const;
  size_t copy_to(std::string *buf, size_t n = static_cast<size_t>(-1L),
                 size_t pos = 0) const;

  size_t append_to(std::string *s, size_t n = static_cast<size_t>(-1L),
                   size_t pos = 0) const;

  size_t copy_to_cstr(char *cstr, size_t n = static_cast<size_t>(-1L),
                      size_t pos = 0) const;

  std::string to_string() const;

  const void *fetch(void *aux_buffer, size_t n) const;

  const void *fetch1() const;

  void clear();

  bool empty() const;

  size_t length() const;
  size_t size() const { return length(); }

  static size_t block_count();
  static size_t block_memory();
  static size_t new_bigview_count();
  static size_t block_count_hit_tls_threshold();

  bool equals(const StringPiece &) const;
  bool equals(const IOBuf &other) const;

  size_t backing_block_num() const { return _ref_num(); }

  StringPiece backing_block(size_t i) const;

  Movable movable() { return Movable(*this); }

protected:
  int _cut_by_char(IOBuf *out, char);
  int _cut_by_delim(IOBuf *out, char const *dbegin, size_t ndelim);

  bool _small() const;

  template <bool MOVE>
  void _push_or_move_back_ref_to_smallview(const BlockRef &);

  template <bool MOVE> void _push_or_move_back_ref_to_bigview(const BlockRef &);

  void _push_back_ref(const BlockRef &);

  void _move_back_ref(const BlockRef &);

  int _pop_front_ref() { return _pop_or_moveout_front_ref<false>(); };

  int _moveout_front_ref() { return _pop_or_moveout_front_ref<true>(); }

  template <bool MOVEOUT> int _pop_or_moveout_front_ref();

  int _pop_back_ref();

  size_t _ref_num() const;

  BlockRef &_front_ref();
  const BlockRef &_front_ref() const;
  BlockRef &_back_ref();
  const BlockRef &_back_ref() const;

  BlockRef &_ref_at(size_t i);
  const BlockRef &_ref_at(size_t i) const;

  const BlockRef &_pref_at(size_t i) const;

private:
  union {
    BigView _bv;
    SmallView _sv;
  };
};
} // namespace butil
} // namespace brpc

#endif // ZHENGQI_CPP_IOBUF_H
