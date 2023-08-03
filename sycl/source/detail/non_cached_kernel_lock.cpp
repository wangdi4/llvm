#include "non_cached_kernel_lock.hpp"

namespace sycl {
inline namespace _V1 {
namespace detail {

NonCachedKernelLock::~NonCachedKernelLock() {}

Locked<sycl::detail::pi::PiKernel> NonCachedKernelLock::lockKernel(sycl::detail::pi::PiKernel &K) {
  std::unique_lock<std::mutex> MapLock{MapMtx};
  std::mutex &Mtx = Map[K];
  MapLock.unlock();

  return {K, Mtx};
}

} // namespace detail
} // namespace _V1
} // namespace sycl
