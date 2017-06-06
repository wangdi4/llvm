//======== deviceRTLs/nios2/main.c - Target RTLs implementation -*- C -*-=====//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.txt for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Device part of the Plugin RTL for Nios(R) II.
///
//===----------------------------------------------------------------------===//

#include <tt_device_dispatcher.h>

int main(void) {
  return tt_device_main();
}
