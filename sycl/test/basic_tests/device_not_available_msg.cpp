// RUN: %clangxx -fsycl %s -o %t.out
// RUN: %t.out

//==---------------- device_not_available_msg.cpp --------------------------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include <CL/sycl.hpp>

#include <string>

using namespace cl;

// The test checks the message which is thrown when device is not available
int main() {
  class RejectEverythingDeviceSelector : public sycl::device_selector {
  public:
    int operator()(const sycl::device &Device) const final { return -1; }
  };

  try {
    sycl::queue Q(RejectEverythingDeviceSelector{});
  } catch (sycl::runtime_error &E) {
    const std::string ExpectedMessage(
        "No device of requested type available. Please check "
        "https://software.intel.com/en-us/articles/"
        "intel-oneapi-dpcpp-compiler-system-requirements-beta");
    const std::string GotMessage(E.what());
    const bool Success = std::string::npos != GotMessage.find(ExpectedMessage);
    return Success ? 0 : 1;
  }

  return 2;
}
