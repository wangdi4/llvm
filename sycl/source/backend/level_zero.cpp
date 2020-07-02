//==--------- level_zero.cpp - SYCL Level-Zero backend ---------------------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include <CL/sycl.hpp>
<<<<<<< HEAD
#include <detail/plugin.hpp>
#include <detail/platform_impl.hpp>
=======
#include <detail/platform_impl.hpp>
#include <detail/plugin.hpp>
>>>>>>> a51c3334b96efa9a3ffaaa77ed5628ab4c1dd07c
#include <detail/program_impl.hpp>
#include <detail/queue_impl.hpp>

__SYCL_INLINE_NAMESPACE(cl) {
namespace sycl {
namespace level0 {
using namespace detail;

<<<<<<< HEAD
// Get the L0 plugin.
static const plugin& getPlugin() {
  static const plugin *L0Plugin = nullptr;
  if (L0Plugin) return *L0Plugin;

  const vector_class<plugin> &Plugins = pi::initialize();
  for (const auto &Plugin : Plugins)
    if (Plugin.getBackend() == backend::level0) {
      L0Plugin = &Plugin;
      break;
    }
  if (!L0Plugin) {
    throw runtime_error("sycl::level0 - no Level-Zero plugin",
                        PI_INVALID_OPERATION);
  }
  return *L0Plugin;
}

//----------------------------------------------------------------------------
// Implementation of level0::make<platform>
__SYCL_EXPORT platform make_platform(pi_native_handle NativeHandle) {
  const auto &Plugin = getPlugin();
  // Create PI platform first.
  pi::PiPlatform PiPlatform;
  Plugin.call<PiApiKind::piextPlatformCreateWithNativeHandle>(NativeHandle, &PiPlatform);
=======
//----------------------------------------------------------------------------
// Implementation of level0::make<platform>
__SYCL_EXPORT platform make_platform(pi_native_handle NativeHandle) {
  const auto &Plugin = pi::getPlugin<backend::level0>();
  // Create PI platform first.
  pi::PiPlatform PiPlatform;
  Plugin.call<PiApiKind::piextPlatformCreateWithNativeHandle>(NativeHandle,
                                                              &PiPlatform);
>>>>>>> a51c3334b96efa9a3ffaaa77ed5628ab4c1dd07c

  // Construct the SYCL platform from PI platfrom.
  return detail::createSyclObjFromImpl<platform>(
      std::make_shared<platform_impl>(PiPlatform, Plugin));
}

//----------------------------------------------------------------------------
// Implementation of level0::make<device>
<<<<<<< HEAD
__SYCL_EXPORT device make_device(const platform &Platform, pi_native_handle NativeHandle) {
  const auto &Plugin = getPlugin();
=======
__SYCL_EXPORT device make_device(const platform &Platform,
                                 pi_native_handle NativeHandle) {
  const auto &Plugin = pi::getPlugin<backend::level0>();
>>>>>>> a51c3334b96efa9a3ffaaa77ed5628ab4c1dd07c
  const auto &PlatformImpl = getSyclObjImpl(Platform);
  // Create PI device first.
  pi::PiDevice PiDevice;
  Plugin.call<PiApiKind::piextDeviceCreateWithNativeHandle>(
      NativeHandle, PlatformImpl->getHandleRef(), &PiDevice);
  // Construct the SYCL device from PI device.
  return detail::createSyclObjFromImpl<device>(
      std::make_shared<device_impl>(PiDevice, PlatformImpl));
}

//----------------------------------------------------------------------------
// Implementation of level0::make<program>
<<<<<<< HEAD
__SYCL_EXPORT program make_program(const context &Context, pi_native_handle NativeHandle) {
=======
__SYCL_EXPORT program make_program(const context &Context,
                                   pi_native_handle NativeHandle) {
>>>>>>> a51c3334b96efa9a3ffaaa77ed5628ab4c1dd07c
  // Construct the SYCL program from native program.
  // TODO: move here the code that creates PI program, and remove the
  // native interop constructor.
  return detail::createSyclObjFromImpl<program>(
      std::make_shared<program_impl>(getSyclObjImpl(Context), NativeHandle));
}

//----------------------------------------------------------------------------
// Implementation of level0::make<queue>
<<<<<<< HEAD
__SYCL_EXPORT queue make_queue(const context &Context, pi_native_handle NativeHandle) {
  const auto &Plugin = getPlugin();
=======
__SYCL_EXPORT queue make_queue(const context &Context,
                               pi_native_handle NativeHandle) {
  const auto &Plugin = pi::getPlugin<backend::level0>();
>>>>>>> a51c3334b96efa9a3ffaaa77ed5628ab4c1dd07c
  const auto &ContextImpl = getSyclObjImpl(Context);
  // Create PI queue first.
  pi::PiQueue PiQueue;
  Plugin.call<PiApiKind::piextQueueCreateWithNativeHandle>(
      NativeHandle, ContextImpl->getHandleRef(), &PiQueue);
  // Construct the SYCL queue from PI queue.
<<<<<<< HEAD
  return detail::createSyclObjFromImpl<queue>(
      std::make_shared<queue_impl>(
          PiQueue, ContextImpl, ContextImpl->get_async_handler()));
=======
  return detail::createSyclObjFromImpl<queue>(std::make_shared<queue_impl>(
      PiQueue, ContextImpl, ContextImpl->get_async_handler()));
>>>>>>> a51c3334b96efa9a3ffaaa77ed5628ab4c1dd07c
}

} // namespace level0
} // namespace sycl
} // __SYCL_INLINE_NAMESPACE(cl)
