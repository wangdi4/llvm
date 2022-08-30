#include "non_cached_kernel_lock.hpp"

namespace sycl {
__SYCL_INLINE_VER_NAMESPACE(_V1) {
namespace detail {

NonCachedKernelLock::~NonCachedKernelLock() {}

Locked<RT::PiKernel> NonCachedKernelLock::lockKernel(RT::PiKernel &K) {
  std::unique_lock<std::mutex> MapLock{MapMtx};
  std::mutex &Mtx = Map[K];
  MapLock.unlock();

  return {K, Mtx};
}

} // namespace detail
} // __SYCL_INLINE_VER_NAMESPACE(_V1)
} // namespace sycl
