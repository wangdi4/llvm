//==------ device_selector.cpp - SYCL device selector ----------------------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include <CL/sycl/backend_types.hpp>
#include <CL/sycl/device.hpp>
#include <CL/sycl/device_selector.hpp>
#include <CL/sycl/exception.hpp>
#include <CL/sycl/stl.hpp>
#include <detail/device_impl.hpp>
#include <detail/force_device.hpp>
// 4.6.1 Device selection class

__SYCL_INLINE_NAMESPACE(cl) {
namespace sycl {

<<<<<<< HEAD
#ifdef INTEL_CUSTOMIZATION
// Utility function to check if device is of the preferred SYCL_BE.
// TODO: this should be changed to just quering the backend of the
// device's plugin.
//
static bool isDeviceOfPreferredSyclBe(const device &Device) {
  // Taking the version information from the platform gives us more useful
  // information than the driver_version of the device.
  const platform Platform = Device.get_info<info::device::platform>();
  const std::string PlatformVersion =
      Platform.get_info<info::platform::version>();

  detail::pi::Backend PreferredBE = detail::pi::getPreferredBE();

  if (PreferredBE == RT::Backend::SYCL_BE_PI_LEVEL0) {
    if (PlatformVersion.find("Level-Zero") != std::string::npos) {
      return true;
    }
  } else if (PreferredBE == RT::Backend::SYCL_BE_PI_OPENCL) {
    if (PlatformVersion.find("OpenCL") != std::string::npos) {
      return true;
    }
  } else if (PreferredBE == RT::Backend::SYCL_BE_PI_CUDA) {
    if (PlatformVersion.find("CUDA") != std::string::npos) {
      return true;
    }
  }
  return false;
}
#endif // INTEL_CUSTOMIZATION
=======
// Utility function to check if device is of the preferred backend.
// Currently preference is given to the opencl backend.
static bool isDeviceOfPreferredSyclBe(const device &Device) {
  if (Device.is_host())
    return false;

  return detail::getSyclObjImpl(Device)->getPlugin().getBackend() ==
         backend::opencl;
}
>>>>>>> 937fec14aeac2607af98450ddf71252321db5573

device device_selector::select_device() const {
  vector_class<device> devices = device::get_devices();
  int score = -1;
  const device *res = nullptr;
  for (const auto &dev : devices) {
<<<<<<< HEAD
    int dev_score = operator()(dev);
    if (detail::pi::trace(detail::pi::TraceLevel::PI_TRACE_ALL)) { // INTEL
      auto platform_name = dev.get_info<info::device::platform>().
          get_info<info::platform::version>();
      auto device_name = dev.get_info<info::device::name>() ;
      std::cout
        << "SYCL_PI_TRACE[1]: select_device(): -> score = " << dev_score << std::endl
        << "SYCL_PI_TRACE[1]:   platform: " << platform_name << std::endl
        << "SYCL_PI_TRACE[1]:   device: " << device_name << std::endl;
    }

#ifdef INTEL_CUSTOMIZATION
    //
=======
    int dev_score = (*this)(dev);
    if (detail::pi::trace(detail::pi::TraceLevel::PI_TRACE_ALL)) {
      string_class PlatformVersion = dev.get_info<info::device::platform>()
                                         .get_info<info::platform::version>();
      string_class DeviceName = dev.get_info<info::device::name>();
      std::cout << "SYCL_PI_TRACE[all]: "
                << "select_device(): -> score = " << score << std::endl
                << "SYCL_PI_TRACE[all]: "
                << "  platform: " << PlatformVersion << std::endl
                << "SYCL_PI_TRACE[all]: "
                << "  device: " << DeviceName << std::endl;
    }

>>>>>>> 937fec14aeac2607af98450ddf71252321db5573
    // SYCL spec says: "If more than one device receives the high score then
    // one of those tied devices will be returned, but which of the devices
    // from the tied set is to be returned is not defined". Here we give a
    // preference to the device of the preferred BE.
    //
    if (score < dev_score ||
<<<<<<< HEAD
        (score == dev_score && dev_score != -1 &&
         isDeviceOfPreferredSyclBe(dev))) {
=======
        (score == dev_score && isDeviceOfPreferredSyclBe(dev))) {
>>>>>>> 937fec14aeac2607af98450ddf71252321db5573
      res = &dev;
      score = dev_score;
    }
  }

  if (res != nullptr) {
    if (detail::pi::trace(detail::pi::TraceLevel::PI_TRACE_BASIC)) {
<<<<<<< HEAD
      auto platform_name = res->get_info<info::device::platform>().
          get_info<info::platform::version>();
      auto device_name = res->get_info<info::device::name>() ;
      std::cout
        << "SYCL_PI_TRACE[1]: select_device(): ->" << std::endl
        << "SYCL_PI_TRACE[1]:   platform: " << platform_name << std::endl
        << "SYCL_PI_TRACE[1]:   device: " << device_name << std::endl;
=======
      string_class PlatformVersion = res->get_info<info::device::platform>()
                                         .get_info<info::platform::version>();
      string_class DeviceName = res->get_info<info::device::name>();
      std::cout << "SYCL_PI_TRACE[all]: "
                << "Selected device ->" << std::endl
                << "SYCL_PI_TRACE[all]: "
                << "  platform: " << PlatformVersion << std::endl
                << "SYCL_PI_TRACE[all]: "
                << "  device: " << DeviceName << std::endl;
>>>>>>> 937fec14aeac2607af98450ddf71252321db5573
    }
    return *res;
  }

  throw cl::sycl::runtime_error(
      "No device of requested type available. Please check "
      "https://software.intel.com/en-us/articles/"
      "intel-oneapi-dpcpp-compiler-system-requirements-beta",
      PI_DEVICE_NOT_FOUND);
#endif // INTEL_CUSTOMIZATION
}

int default_selector::operator()(const device &dev) const {

<<<<<<< HEAD
#ifdef INTEL_CUSTOMIZATION
#if 0
  // Take note of the SYCL_BE environment variable when doing default selection
  const char *SYCL_BE = std::getenv("SYCL_BE");
  if (SYCL_BE) {
    std::string backend = (SYCL_BE ? SYCL_BE : "");
    // Taking the version information from the platform gives us more useful
    // information than the driver_version of the device.
    const platform platform = dev.get_info<info::device::platform>();
    const std::string platformVersion =
        platform.get_info<info::platform::version>();;
    // If using PI_CUDA, don't accept a non-CUDA device
    if (platformVersion.find("CUDA") == std::string::npos &&
        backend == "PI_CUDA") {
      return -1;
    }
    // If using PI_OPENCL, don't accept a non-OpenCL device
    if (platformVersion.find("OpenCL") == std::string::npos &&
        backend == "PI_OPENCL") {
      return -1;
    }
#endif

=======
>>>>>>> 937fec14aeac2607af98450ddf71252321db5573
  int Score = -1;

  // Give preference to device of SYCL BE.
  if (isDeviceOfPreferredSyclBe(dev))
    Score = 50;

  // override always wins
<<<<<<< HEAD
  if (dev.get_info<info::device::device_type>() == detail::get_forced_type()) {
    Score += 1000;
    return Score;
  }
=======
  if (dev.get_info<info::device::device_type>() == detail::get_forced_type())
    Score += 1000;
>>>>>>> 937fec14aeac2607af98450ddf71252321db5573

  if (dev.is_gpu())
    Score += 500;

  if (dev.is_cpu())
    Score += 300;

  if (dev.is_host())
    Score += 100;

  return Score;
}

int gpu_selector::operator()(const device &dev) const {
  int Score = -1;
  if (dev.is_gpu()) {
    Score = 1000;
    // Give preference to device of SYCL BE.
    if (isDeviceOfPreferredSyclBe(dev))
      Score += 50;
  }
  return Score;
}

int cpu_selector::operator()(const device &dev) const {
  int Score = -1;
  if (dev.is_cpu()) {
    Score = 1000;
    // Give preference to device of SYCL BE.
    if (isDeviceOfPreferredSyclBe(dev))
      Score += 50;
  }
  return Score;
}

int accelerator_selector::operator()(const device &dev) const {
  int Score = -1;
  if (dev.is_accelerator()) {
    Score = 1000;
    // Give preference to device of SYCL BE.
    if (isDeviceOfPreferredSyclBe(dev))
      Score += 50;
  }
  return Score;
}

int host_selector::operator()(const device &dev) const {
  int Score = -1;
  if (dev.is_host()) {
    Score = 1000;
    // Give preference to device of SYCL BE.
    if (isDeviceOfPreferredSyclBe(dev))
      Score += 50;
  }
  return Score;
}
#endif // INTEL_CUSTOMIZATION

} // namespace sycl
} // __SYCL_INLINE_NAMESPACE(cl)
