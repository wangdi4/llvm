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

#include <fpga_mc_dispatcher.h>
extern void __kmpc_omp_init();

int main(void) {
  __kmpc_omp_init();
  return fpga_mc_main();
}
