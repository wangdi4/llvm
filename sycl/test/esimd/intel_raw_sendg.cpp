// INTEL_FEATURE_ESIMD_EMBARGO
// The test verifies generalized raw send on Xe3+.
// REQUIRES: intel_feature_esimd_embargo

// RUN: %clangxx -fsycl -fsycl-device-only -fsycl-esimd-force-stateless-mem -S %s -o %t.ll
// RUN: sycl-post-link -lower-esimd -split-esimd -S %t.ll -o %t.table
// RUN: FileCheck %s -input-file=%t_esimd_0.ll

#include <sycl/ext/intel/esimd.hpp>

using namespace sycl::ext::intel::esimd;
using namespace sycl;

SYCL_EXTERNAL void func0() SYCL_ESIMD_FUNCTION {
  simd<int, 8> x = 50;
  simd<int, 8> y = 51;
  // CHECK: = call <8 x i32> @llvm.genx.raw.sendg.v8i32.v8i1.v8i32.v8i32(i16 32, i1 false, i1 false, i8 3,
  // CHECK-SAME: <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>,
  // CHECK-SAME: <8 x i32> <i32 50, i32 50, i32 50, i32 50, i32 50, i32 50, i32 50, i32 50>, i16 32,
  // CHECK-SAME:  <8 x i32> <i32 51, i32 51, i32 51, i32 51, i32 51, i32 51, i32 51, i32 51>, i16 32,
  // CHECK-SAME: i64 6, i64 9, i64 500, <8 x i32> zeroinitializer)
  auto z = sycl::ext::intel::experimental::esimd::raw_sendg<int, 8, 8, 3, 500>(
      x, y, 6, 9);

  // CHECK: = call <8 x i32> @llvm.genx.raw.sendg.v8i32.v8i1.v8i32.v8i32(i16 32, i1 false, i1 false, i8 3,
  // CHECK-SAME: <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>,
  // CHECK-SAME: <8 x i32> <i32 50, i32 50, i32 50, i32 50, i32 50, i32 50, i32 50, i32 50>, i16 32,
  // CHECK-SAME: <8 x i32> <i32 51, i32 51, i32 51, i32 51, i32 51, i32 51, i32 51, i32 51>, i16 32,
  // CHECK-SAME: i64 6, i64 undef, i64 500, <8 x i32> zeroinitializer)
  auto z1 = sycl::ext::intel::experimental::esimd::raw_sendg<int, 8, 8, 3, 500>(
      x, y, 6);
  // CHECK: = call <8 x i32> @llvm.genx.raw.sendg.v8i32.v8i1.v8i32.v8i32(i16 32, i1 false, i1 false, i8 3,
  // CHECK-SAME: <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>,
  // CHECK-SAME: <8 x i32> <i32 50, i32 50, i32 50, i32 50, i32 50, i32 50, i32 50, i32 50>, i16 32,
  // CHECK-SAME: <8 x i32> <i32 51, i32 51, i32 51, i32 51, i32 51, i32 51, i32 51, i32 51>, i16 32,
  // CHECK-SAME: i64 undef, i64 undef, i64 500, <8 x i32> zeroinitializer)
  auto z2 =
      sycl::ext::intel::experimental::esimd::raw_sendg<int, 8, 8, 3, 500>(x, y);
}

SYCL_EXTERNAL void func1() SYCL_ESIMD_FUNCTION {
  simd<int, 8> x = 60;
  // CHECK: = call <8 x i32> @llvm.genx.raw.sendg.v8i32.v8i1.v8i32.v16i32(i16 32, i1 false, i1 false, i8 5,
  // CHECK-SAME: <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>,
  // CHECK-SAME: <8 x i32> <i32 60, i32 60, i32 60, i32 60, i32 60, i32 60, i32 60, i32 60>, i16 32,
  // CHECK-SAME: <16 x i32> undef, i16 0,
  // CHECK-SAME: i64 4, i64 2, i64 199, <8 x i32> zeroinitializer)
  auto y = sycl::ext::intel::experimental::esimd::raw_sendg<int, 8, 8, 5, 199>(
      x, 4, 2);
  // CHECK: = call <8 x i32> @llvm.genx.raw.sendg.v8i32.v8i1.v8i32.v16i32(i16 32, i1 false, i1 false, i8 5,
  // CHECK-SAME: <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>,
  // CHECK-SAME: <8 x i32> <i32 60, i32 60, i32 60, i32 60, i32 60, i32 60, i32 60, i32 60>, i16 32,
  // CHECK-SAME: <16 x i32> undef, i16 0,
  // CHECK-SAME: i64 4, i64 undef, i64 199, <8 x i32> zeroinitializer)
  auto y1 =
      sycl::ext::intel::experimental::esimd::raw_sendg<int, 8, 8, 5, 199>(x, 4);

  // CHECK: = call <8 x i32> @llvm.genx.raw.sendg.v8i32.v8i1.v8i32.v16i32(i16 32, i1 false, i1 false, i8 5,
  // CHECK-SAME: <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>,
  // CHECK-SAME: <8 x i32> <i32 60, i32 60, i32 60, i32 60, i32 60, i32 60, i32 60, i32 60>, i16 32,
  // CHECK-SAME <16 x i32> undef, i16 0,
  // CHECK-SAME: i64 undef, i64 undef, i64 199, <8 x i32> zeroinitializer)
  auto y2 =
      sycl::ext::intel::experimental::esimd::raw_sendg<int, 8, 8, 5, 199>(x);
}

SYCL_EXTERNAL void func2() SYCL_ESIMD_FUNCTION {
  simd<int, 8> x = 80;
  simd<int, 8> y = 82;
  // CHECK: = call <16 x i32> @llvm.genx.raw.sendg.v16i32.v8i1.v8i32.v8i32(i16 0, i1 true, i1 true, i8 7,
  // CHECK-SAME: <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>,
  // CHECK-SAME: <8 x i32> <i32 80, i32 80, i32 80, i32 80, i32 80, i32 80, i32 80, i32 80>, i16 32,
  // CHECK-SAME: <8 x i32> <i32 82, i32 82, i32 82, i32 82, i32 82, i32 82, i32 82, i32 82>, i16 32,
  // CHECK-SAME: i64 33, i64 34, i64 87, <16 x i32> undef)
  sycl::ext::intel::experimental::esimd::raw_sendg<
      8, 7, 87, sycl::ext::intel::experimental::esimd::raw_send_eot::eot,
      sycl::ext::intel::experimental::esimd::raw_send_sendc::sendc>(x, y, 33,
                                                                    34);
  // CHECK: = call <16 x i32> @llvm.genx.raw.sendg.v16i32.v8i1.v8i32.v8i32(i16 0, i1 true, i1 true, i8 7,
  // CHECK-SAME: <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>,
  // CHECK-SAME: <8 x i32> <i32 80, i32 80, i32 80, i32 80, i32 80, i32 80, i32 80, i32 80>, i16 32,
  // CHECK-SAME: <8 x i32> <i32 82, i32 82, i32 82, i32 82, i32 82, i32 82, i32 82, i32 82>, i16 32,
  // CHECK-SAME: i64 33, i64 undef, i64 87, <16 x i32> undef)
  sycl::ext::intel::experimental::esimd::raw_sendg<
      8, 7, 87, sycl::ext::intel::experimental::esimd::raw_send_eot::eot,
      sycl::ext::intel::experimental::esimd::raw_send_sendc::sendc>(x, y, 33);
  // CHECK: = call <16 x i32> @llvm.genx.raw.sendg.v16i32.v8i1.v8i32.v8i32(i16 0, i1 true, i1 true, i8 7,
  // CHECK-SAME: <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>,
  // CHECK-SAME: <8 x i32> <i32 80, i32 80, i32 80, i32 80, i32 80, i32 80, i32 80, i32 80>, i16 32,
  // CHECK-SAME: <8 x i32> <i32 82, i32 82, i32 82, i32 82, i32 82, i32 82, i32 82, i32 82>, i16 32
  // CHECK-SAME: i64 undef, i64 undef, i64 87, <16 x i32> undef)
  sycl::ext::intel::experimental::esimd::raw_sendg<
      8, 7, 87, sycl::ext::intel::experimental::esimd::raw_send_eot::eot,
      sycl::ext::intel::experimental::esimd::raw_send_sendc::sendc>(x, y);
}

SYCL_EXTERNAL void func3() SYCL_ESIMD_FUNCTION {
  simd<int, 8> x = 70;
  // CHECK: = call <16 x i32> @llvm.genx.raw.sendg.v16i32.v8i1.v8i32.v16i32(i16 0, i1 false, i1 false, i8 5,
  // CHECK-SAME: <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>,
  // CHECK-SAME: <8 x i32> <i32 70, i32 70, i32 70, i32 70, i32 70, i32 70, i32 70, i32 70>, i16 32,
  // CHECK-SAME: <16 x i32> undef, i16 0, i64 12, i64 14, i64 600, <16 x i32> undef)
  sycl::ext::intel::experimental::esimd::raw_sendg<8, 5, 600>(x, 12, 14);

  // CHECK: = call <16 x i32> @llvm.genx.raw.sendg.v16i32.v8i1.v8i32.v16i32(i16 0, i1 false, i1 false, i8 5,
  // CHECK-SAME: <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>,
  // CHECK-SAME: <8 x i32> <i32 70, i32 70, i32 70, i32 70, i32 70, i32 70, i32 70, i32 70>, i16 32,
  // CHECK-SAME: <16 x i32> undef, i16 0, i64 12, i64 undef, i64 600, <16 x i32> undef)
  sycl::ext::intel::experimental::esimd::raw_sendg<8, 5, 600>(x, 12);

  // CHECK: = call <16 x i32> @llvm.genx.raw.sendg.v16i32.v8i1.v8i32.v16i32(i16 0, i1 false, i1 false, i8 5,
  // CHECK-SAME: <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>,
  // CHECK-SAME: <8 x i32> <i32 70, i32 70, i32 70, i32 70, i32 70, i32 70, i32 70, i32 70>, i16 32,
  // CHECK-SAME: <16 x i32> undef, i16 0, i64 undef, i64 undef, i64 600, <16 x i32> undef)
  sycl::ext::intel::experimental::esimd::raw_sendg<8, 5, 600>(x);
}

// end INTEL_FEATURE_ESIMD_EMBARGO
