#pragma once

#include <sycl/detail/common.hpp>
#include <sycl/detail/locked.hpp>
#include <sycl/detail/pi.hpp>

#include <map>
#include <mutex>

namespace sycl {
__SYCL_INLINE_VER_NAMESPACE(_V1) {
namespace detail {

class NonCachedKernelLock {
public:
  ~NonCachedKernelLock();

  Locked<RT::PiKernel> lockKernel(RT::PiKernel &K);

private:
  using KernelLockMapT = std::map<RT::PiKernel, std::mutex>;

  std::mutex MapMtx;
  KernelLockMapT Map{std::less<RT::PiKernel>{}};
};

} // namespace detail
} // __SYCL_INLINE_VER_NAMESPACE(_V1)
} // namespace sycl
