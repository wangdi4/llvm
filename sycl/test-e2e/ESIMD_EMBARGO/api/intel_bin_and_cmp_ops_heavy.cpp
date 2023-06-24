// INTEL_FEATURE_ESIMD_EMBARGO
//==------- intel_bin_and_cmp_ops_heavy.cpp  - DPC++ ESIMD on-device test -==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// REQUIRES: intel_feature_esimd_embargo
// TODO: Actually, the test requires PVC emulator working as Xe3P/FalconShores
// device, or actual Xe3P/FalconShores device. Need to a) add
// 'gpu-intel-xe3p'(or marketing name if there is such) or b) add
// 'gpu-intel-xe3p-emulator' instead. REQUIRES: gpu-intel-pvc

// RUN: %clangxx -fsycl -fsycl-device-code-split=per_kernel %s -o %t.out
// TODO: The test can only be runned manually now because of lack of Rialto
// target support in TC/QA-scripts.
// RUNx: %GPU_RUN_PLACEHOLDER %t.out

// Tests various binary operations applied to simd objects.

// TODO
// Arithmetic operations behaviour depends on Gen's control register's rounding
// mode, which is RTNE by default:
//    cr0.5:4 is 00b = Round to Nearest or Even (RTNE)
// For half and tfloat32 this leads to divergence between Gen and host
// (emulated) results larger than certain threshold. Might need to tune the cr0
// once this feature is available in ESIMD.
//
//
// Info note: The guide on usage of PVC emulator emulating Rialto GPU.
// 1)	Proceed to  goto.intel.com/fulsim
// 2) Right mouse button click on the device e.g. “PVC Release”,
//    and “Open Link in New Tab”. Due to some glitch a regular left mouse click
//    does not work.
// 3)	Copy the address of needed zip file to clipboard. (e.g.
// https://gfx-assets.fm.intel.com/artifactory/gfx-cobalt-assets-fm/Cobalt/Linux/PVC/65927/PVC-65927-Linux.zip)
// 4)	On Linux: wget --user $USER --ask-password
// https://gfx-assets.fm.intel.com/artifactory/gfx-cobalt-assets-fm/Cobalt/Linux/PVC/66223/PVC-66223-Linux.zip
//    wget --user $USER --ask-password
//    https://gfx-assets.fm.intel.com/artifactory/gfx-cobalt-assets-fm/Cobalt/Linux/PVC/66405/PVC-66405-Linux.zip
// 5)	Unzip to folder, for example: /iusers/$USER/fulsim/PVC
// 6)	Open 2 linux command-shells (or tmux panels): I’ll call them: _test_ and
//    _pvc_ .
// 7)	In _pvc_ panel:
//    "COUNT=200; CUR=1; while [ "$CUR" -ne `expr $COUNT + 1` ]; \
//     do /iusers/$USER/fulsim/PVC/AubLoad -device rlt.8x10x8.a0 -socket \
//     tcp -swsbcheck fatal -msglevel terse ; CUR=`expr $CUR + 1`; done;"
// 8)	In _test_ panel:
// cat > igdrcl.config
// SetCommandStreamReceiver = 2
// ProductFamilyOverride = rlt
// ForceDeviceId = 0x0b63
// PrintDebugSettings = 1
//
// After following steps (1) to (7), just proceed to _test_ panel and run the
// test in the directory with 'igdrcl.config' file as usual. E.g. "./a.out"

#define SKIP_MAIN
#include "../../ESIMD/api/bin_and_cmp_ops_heavy.cpp"

using hf8 = sycl::ext::intel::experimental::esimd::hf8;
using bf8 = sycl::ext::intel::experimental::esimd::bf8;
namespace esimd_test {
TID(sycl::ext::intel::experimental::esimd::hf8)
TID(sycl::ext::intel::experimental::esimd::bf8)
} // namespace esimd_test

int main(void) {
  queue q(esimd_test::ESIMDSelector, esimd_test::createExceptionHandler());

  auto dev = q.get_device();
  std::cout << "Running on " << dev.get_info<sycl::info::device::name>()
            << "\n";
  bool passed = true;
  using BinOp = esimd_test::BinaryOp;

  bool SupportsDouble = dev.has(aspect::fp64);
  bool SupportsHalf = dev.has(aspect::fp16);

  auto arith_ops = esimd_test::ArithBinaryOpsNoDiv;

  passed &= test<hf8, float, 32, BinOp, VEfa, IDf>(arith_ops, q, 0.001f);
  passed &= test<tfloat32, hf8, 32, BinOp, VEfa, IDf>(arith_ops, q, 0.001f);
  if (SupportsHalf)
    passed &= test<hf8, half, 32, BinOp, VEfa, IDf>(arith_ops, q, 0.001f);
  passed &= test<bf8, hf8, 32, BinOp, VEfa, IDf>(arith_ops, q, 0.001f);
  passed &= test<hf8, bf8, 32, BinOp, VEfa, IDf>(arith_ops, q, 0.001f);
  passed &= test<bf8, float, 32, BinOp, VEfa, IDf>(arith_ops, q, 0.001f);
  passed &= test<tfloat32, bf8, 32, BinOp, VEfa, IDf>(arith_ops, q, 0.001f);
  if (SupportsHalf)
    passed &= test<bf8, half, 32, BinOp, VEfa, IDf>(arith_ops, q, 0.001f);

  // Test division separately, as error probability is higher.
  auto div_op = esimd_test::BinaryOpSeq<BinOp::div>{};

  passed &= test<hf8, float, 32, BinOp, VEf, IDf>(div_op, q, 0.001f);
  if (SupportsHalf)
    passed &= test<hf8, half, 32, BinOp, VEf, IDf>(div_op, q, 0.001f);

  passed &= test<hf8, bf8, 32, BinOp, VEf, IDf>(div_op, q, 0.01f);
  passed &= test<bf8, hf8, 32, BinOp, VEf, IDf>(div_op, q, 0.01f);
  passed &= test<bf8, float, 32, BinOp, VEf, IDf>(div_op, q, 0.001f);
  passed &= test<bf8, bf8, 32, BinOp, VEf, IDf>(div_op, q, 1);

  if (SupportsHalf)
    passed &= test<bf8, half, 32, BinOp, VEf, IDf>(div_op, q, 0.001f);

  using CmpOp = esimd_test::CmpOp;
  auto cmp_ops = esimd_test::CmpOps;

  passed &= test<hf8, float, 32, CmpOp, VSf, IDf>(cmp_ops, q);
  passed &= test<hf8, hf8, 32, CmpOp, VSf, IDf>(cmp_ops, q);
  passed &= test<char, hf8, 32, CmpOp, VSf, IDf>(cmp_ops, q);
  if (SupportsHalf)
    passed &= test<hf8, half, 32, CmpOp, VSf, IDf>(cmp_ops, q);
  passed &= test<bf8, hf8, 32, CmpOp, VSf, IDf>(cmp_ops, q);
  passed &= test<hf8, bf8, 32, CmpOp, VSf, IDf>(cmp_ops, q);
  passed &= test<bf8, float, 32, CmpOp, VSf, IDf>(cmp_ops, q);
  passed &= test<bf8, bf8, 32, CmpOp, VSf, IDf>(cmp_ops, q);
  passed &= test<char, bf8, 32, CmpOp, VSf, IDf>(cmp_ops, q);
  if (SupportsHalf)
    passed &= test<bf8, half, 32, CmpOp, VSf, IDf>(cmp_ops, q);

  std::cout << (passed ? "Test PASSED\n" : "Test FAILED\n");
  return passed ? 0 : 1;
}
// end INTEL_FEATURE_ESIMD_EMBARGO
