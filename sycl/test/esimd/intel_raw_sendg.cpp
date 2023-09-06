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
  // CHECK: = call <8 x i32> asm "($11) raw_sendg.$1 (M1, $2) $0.0/$3 $4.0/$5 $6.0/$7 $8 $9 $10", "=^rw,n,n,n,^rw,n,^rw,n,r,r,n,^cr"
  // CHECK-SAME: i32 3, i8 8, i32 32
  // CHECK-SAME: <8 x i32>  <i32 50, i32 50, i32 50, i32 50, i32 50, i32 50, i32 50, i32 50>
  // CHECK-SAME: i32 32
  // CHECK-SAME: <8 x i32>  <i32 51, i32 51, i32 51, i32 51, i32 51, i32 51, i32 51, i32 51>
  // CHECK-SAME: i32 32, i64 6, i64 9, i64 500
  // CHECK-SAME: <8 x i16> <i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1>
  auto z = sycl::ext::intel::experimental::esimd::raw_sendg<int, 8, 8, 3, 500>(
      x, y, 6, 9);
  // CHECK: = call <8 x i32> asm "($10) raw_sendg.$1 (M1, $2) $0.0/$3 $4.0/$5 $6.0/$7 $8 %null.0/0 $9", "=^rw,n,n,n,^rw,n,^rw,n,r,n,^cr"
  // CHECK-SAME: i32 3, i8 8, i32 32
  // CHECK-SAME: <8 x i32>  <i32 50, i32 50, i32 50, i32 50, i32 50, i32 50, i32 50, i32 50>
  // CHECK-SAME: i32 32
  // CHECK-SAME: <8 x i32>  <i32 51, i32 51, i32 51, i32 51, i32 51, i32 51, i32 51, i32 51>
  // CHECK-SAME: i32 32, i64 6, i64 500
  // CHECK-SAME: <8 x i16> <i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1>
  auto z1 = sycl::ext::intel::experimental::esimd::raw_sendg<int, 8, 8, 3, 500>(
      x, y, 6);
  // CHECK: = call <8 x i32> asm "($9) raw_sendg.$1 (M1, $2) $0.0/$3 $4.0/$5 $6.0/$7 %null.0/0 %null.0/0 $8", "=^rw,n,n,n,^rw,n,^rw,n,n,^cr"
  // CHECK-SAME: i32 3, i8 8, i32 32
  // CHECK-SAME: <8 x i32>  <i32 50, i32 50, i32 50, i32 50, i32 50, i32 50, i32 50, i32 50>
  // CHECK-SAME: i32 32
  // CHECK-SAME: <8 x i32>  <i32 51, i32 51, i32 51, i32 51, i32 51, i32 51, i32 51, i32 51>
  // CHECK-SAME: i32 32, i64 500
  // CHECK-SAME: <8 x i16> <i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1>
  auto z2 =
      sycl::ext::intel::experimental::esimd::raw_sendg<int, 8, 8, 3, 500>(x, y);
}

SYCL_EXTERNAL void func1() SYCL_ESIMD_FUNCTION {
  simd<int, 8> x = 60;

  // CHECK: = call <8 x i32> asm "($9) raw_sendg.$1 (M1, $2) $0.0/$3 $4.0/$5 %null.0/0 $6 $7 $8", "=^rw,n,n,n,^rw,n,r,r,n,^cr"
  // CHECK-SAME: i32 5, i8 8, i32 32
  // CHECK-SAME: <8 x i32>  <i32 60, i32 60, i32 60, i32 60, i32 60, i32 60, i32 60, i32 60>
  // CHECK-SAME: i32 32, i64 4, i64 2, i64 199
  // CHECK-SAME: <8 x i16> <i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1>
  auto y = sycl::ext::intel::experimental::esimd::raw_sendg<int, 8, 8, 5, 199>(
      x, 4, 2);
  // CHECK: = call <8 x i32> asm "($8) raw_sendg.$1 (M1, $2) $0.0/$3 $4.0/$5 %null.0/0 $6 %null.0/0 $7", "=^rw,n,n,n,^rw,n,r,n,^cr"
  // CHECK-SAME: i32 5, i8 8, i32 32
  // CHECK-SAME: <8 x i32>  <i32 60, i32 60, i32 60, i32 60, i32 60, i32 60, i32 60, i32 60>
  // CHECK-SAME: i32 32, i64 4, i64 199
  // CHECK-SAME: <8 x i16> <i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1>
  auto y1 =
      sycl::ext::intel::experimental::esimd::raw_sendg<int, 8, 8, 5, 199>(x, 4);
  // CHECK: = call <8 x i32> asm "($7) raw_sendg.$1 (M1, $2) $0.0/$3 $4.0/$5 %null.0/0 %null.0/0 %null.0/0 $6", "=^rw,n,n,n,^rw,n,n,^cr"
  // CHECK-SAME: i32 5, i8 8, i32 32
  // CHECK-SAME: <8 x i32>  <i32 60, i32 60, i32 60, i32 60, i32 60, i32 60, i32 60, i32 60>
  // CHECK-SAME: i32 32, i64 199
  // CHECK-SAME: <8 x i16> <i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1>
  auto y2 =
      sycl::ext::intel::experimental::esimd::raw_sendg<int, 8, 8, 5, 199>(x);
}

SYCL_EXTERNAL void func2() SYCL_ESIMD_FUNCTION {
  simd<int, 8> x = 80;
  simd<int, 8> y = 82;
  // CHECK: call void asm sideeffect "($9) raw_sendgc_eot.$0 (M1, $1) %null.0/0 $2.0/$3 $4.0/$5 $6 $7 $8", "n,n,^rw,n,^rw,n,r,r,n,^cr"
  // CHECK-SAME: i32 7, i8 8
  // CHECK-SAME: <8 x i32> <i32 80, i32 80, i32 80, i32 80, i32 80, i32 80, i32 80, i32 80>
  // CHECK-SAME: i32 32
  // CHECK-SAME: <8 x i32> <i32 82, i32 82, i32 82, i32 82, i32 82, i32 82, i32 82, i32 82>
  // CHECK-SAME: i32 32, i64 33, i64 34, i64 87
  // CHECK-SAME: <8 x i16> <i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1>
  sycl::ext::intel::experimental::esimd::raw_sendg<
      8, 7, 87, sycl::ext::intel::experimental::esimd::raw_send_eot::eot,
      sycl::ext::intel::experimental::esimd::raw_send_sendc::sendc>(x, y, 33,
                                                                    34);
  // CHECK: call void asm sideeffect "($8) raw_sendgc_eot.$0 (M1, $1) %null.0/0 $2.0/$3 $4.0/$5 $6 %null.0/0 $7", "n,n,^rw,n,^rw,n,r,n,^cr"
  // CHECK-SAME: i32 7, i8 8
  // CHECK-SAME: <8 x i32> <i32 80, i32 80, i32 80, i32 80, i32 80, i32 80, i32 80, i32 80>
  // CHECK-SAME: i32 32
  // CHECK-SAME: <8 x i32> <i32 82, i32 82, i32 82, i32 82, i32 82, i32 82, i32 82, i32 82>
  // CHECK-SAME: i32 32, i64 33, i64 87
  // CHECK-SAME: <8 x i16> <i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1>
  sycl::ext::intel::experimental::esimd::raw_sendg<
      8, 7, 87, sycl::ext::intel::experimental::esimd::raw_send_eot::eot,
      sycl::ext::intel::experimental::esimd::raw_send_sendc::sendc>(x, y, 33);
  // CHECK: call void asm sideeffect "($7) raw_sendgc_eot.$0 (M1, $1) %null.0/0 $2.0/$3 $4.0/$5 %null.0/0 %null.0/0 $6", "n,n,^rw,n,^rw,n,n,^cr"
  // CHECK-SAME: i32 7, i8 8
  // CHECK-SAME: <8 x i32> <i32 80, i32 80, i32 80, i32 80, i32 80, i32 80, i32 80, i32 80>
  // CHECK-SAME: i32 32
  // CHECK-SAME: <8 x i32> <i32 82, i32 82, i32 82, i32 82, i32 82, i32 82, i32 82, i32 82>
  // CHECK-SAME: i32 32, i64 87
  // CHECK-SAME: <8 x i16> <i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1>
  sycl::ext::intel::experimental::esimd::raw_sendg<
      8, 7, 87, sycl::ext::intel::experimental::esimd::raw_send_eot::eot,
      sycl::ext::intel::experimental::esimd::raw_send_sendc::sendc>(x, y);
}

SYCL_EXTERNAL void func3() SYCL_ESIMD_FUNCTION {
  simd<int, 8> x = 70;

  // CHECK: call void asm sideeffect "($7) raw_sendg.$0 (M1, $1) %null.0/0 $2.0/$3 %null.0/0 $4 $5 $6", "n,n,^rw,n,r,r,n,^cr"
  // CHECK-SAME: i32 5, i8 8
  // CHECK-SAME: <8 x i32>  <i32 70, i32 70, i32 70, i32 70, i32 70, i32 70, i32 70, i32 70>
  // CHECK-SAME: i32 32, i64 12, i64 14, i64 600
  // CHECK-SAME: <8 x i16> <i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1>
  sycl::ext::intel::experimental::esimd::raw_sendg<8, 5, 600>(x, 12, 14);

  // CHECK: call void asm sideeffect "($6) raw_sendg.$0 (M1, $1) %null.0/0 $2.0/$3 %null.0/0 $4 %null.0/0 $5", "n,n,^rw,n,r,n,^cr"
  // CHECK-SAME: i32 5, i8 8
  // CHECK-SAME: <8 x i32>  <i32 70, i32 70, i32 70, i32 70, i32 70, i32 70, i32 70, i32 70>
  // CHECK-SAME: i32 32, i64 12, i64 600
  // CHECK-SAME: <8 x i16> <i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1>
  sycl::ext::intel::experimental::esimd::raw_sendg<8, 5, 600>(x, 12);

  // CHECK: call void asm sideeffect "($5) raw_sendg.$0 (M1, $1) %null.0/0 $2.0/$3 %null.0/0 %null.0/0 %null.0/0 $4", "n,n,^rw,n,n,^cr"
  // CHECK-SAME: i32 5, i8 8
  // CHECK-SAME: <8 x i32>  <i32 70, i32 70, i32 70, i32 70, i32 70, i32 70, i32 70, i32 70>
  // CHECK-SAME: i32 32, i64 600
  // CHECK-SAME: <8 x i16> <i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1>
  sycl::ext::intel::experimental::esimd::raw_sendg<8, 5, 600>(x);
}

// end INTEL_FEATURE_ESIMD_EMBARGO
