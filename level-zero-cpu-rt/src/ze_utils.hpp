// INTEL_COPYRIGHT_BEGIN
// Copyright (c) 2020, Intel Corporation. All rightrs reserved.
// INTEL_COPYRIGHT_END

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string.h>

namespace __zert__ {
namespace util {

inline int strncpy_s(char *dst, size_t numberOfElements, const char *src,
                     size_t count) {
  if ((dst == nullptr) || (src == nullptr)) {
    return -EINVAL;
  }
  if (numberOfElements < count) {
    return -ERANGE;
  }

  size_t length = strlen(src);
  if (length > count) {
    length = count;
  }
  memcpy(dst, src, length);

  if (length < numberOfElements) {
    numberOfElements = length;
  }
  dst[numberOfElements] = '\0';

  return 0;
}

inline int memcpy_s(void *dst, size_t destSize, const void *src, size_t count) {
  if ((dst == nullptr) || (src == nullptr)) {
    return -EINVAL;
  }
  if (destSize < count) {
    return -ERANGE;
  }

  memcpy(dst, src, count);
  return 0;
}

inline std::string get_envvar(std::string var) {
#ifdef _WIN32
  char *buf = nullptr;
  size_t sz;
  _dupenv_s(&buf, &sz, var.c_str());
  if (buf != nullptr) {
    std::string res(buf);
    delete buf;
    return res;
  } else {
    return {};
  }
#elif defined(__unix__) or defined(__QNX__) or defined(__APPLE__)
  char *val = std::getenv(var.c_str());
  if (val != nullptr) {
    return std::string(val);
  } else {
    return {};
  }
#else
#error "unsupported platform"
#endif
}

struct Logger final {
private:
  std::string prefix_;
  std::stringstream ss_;
  std::ostream &os_;

public:
  Logger(std::string prefix, std::ostream &os) : prefix_(prefix), os_(os) {
    ss_ << prefix << ":";
  }
  ~Logger() { os_ << ss_.str() << std::endl; }
  template <class T> Logger &operator<<(T t) {
    ss_ << t;
    return *this;
  }
};

} // namespace util
} // namespace __zert__

#define ZESIMOUT                                                               \
  __zert__::util::Logger(std::string("ZESIMOUT:") + std::string(__FILE__) +    \
                             std::string(":") + std::to_string(__LINE__),      \
                         std::cout)
#define ZESIMERR                                                               \
  __zert__::util::Logger(std::string("ZESIMERR:") + std::string(__FILE__) +    \
                             std::string(":") + std::to_string(__LINE__),      \
                         std::cerr)
