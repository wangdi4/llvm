#pragma once

#include <sycl/detail/common.hpp>
#include <sycl/detail/locked.hpp>
#include <sycl/detail/pi.hpp>

#include <map>
#include <mutex>

namespace sycl {
inline namespace _V1 {
namespace detail {

class NonCachedKernelLock {
public:
  ~NonCachedKernelLock();

  Locked<sycl::detail::pi::PiKernel> lockKernel(sycl::detail::pi::PiKernel &K);

private:
  using KernelLockMapT = std::map<sycl::detail::pi::PiKernel, std::mutex>;

  std::mutex MapMtx;
  KernelLockMapT Map{std::less<sycl::detail::pi::PiKernel>{}};
};

} // namespace detail
} // namespace _V1
} // namespace sycl
