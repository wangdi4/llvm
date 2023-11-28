// REQUIRES: intel_feature_sw_dtrans
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-linux-gnu -emit-dtrans-info -fintel-compatibility -emit-llvm  %s -o - | FileCheck %s --check-prefixes=SPLIT
// RUN: %clang_cc1 -disable-llvm-passes -O2 -triple x86_64-windows -emit-dtrans-info -fintel-compatibility -emit-llvm  %s -o - | FileCheck %s --check-prefixes=AS_STRUCT
// __int128_t not available on 32 bit platforms, so no abillity to test.

extern "C" {
__int128_t int128_param(__int128_t);
__uint128_t uint128_param(__uint128_t);
}

void use() {
  int128_param(__int128_t{});
  uint128_param(__uint128_t{});
}

// SPLIT: declare { i64, i64 } @int128_param(i64 noundef, i64 noundef)
// AS_STRUCT: declare !intel.dtrans.func.type ![[I128_MD:[0-9]+]] dso_local <2 x i64> @int128_param(ptr noundef "intel_dtrans_func_index"="1")
// SPLIT: declare { i64, i64 } @uint128_param(i64 noundef, i64 noundef)
// AS_STRUCT: declare !intel.dtrans.func.type ![[U128_MD:[0-9]+]] dso_local <2 x i64> @uint128_param(ptr noundef "intel_dtrans_func_index"="1")

// AS_STRUCT: ![[I128_MD]] = distinct !{![[I128_PTR:[0-9]+]]}
// AS_STRUCT: ![[I128_PTR]] = !{i128 0, i32 1}
// AS_STRUCT: ![[U128_MD]] = distinct !{![[I128_PTR]]}
