// INTEL_FEATURE_ISA_BF16_BASE
// REQUIRES: intel_feature_isa_bf16_base
// RUN: %clang_cc1 -triple x86_64-linux -emit-llvm  -target-feature +avx512fp16 < %s | FileCheck %s --check-prefixes=CHECK
struct bh {
  __bf16 a;
  _Float16 b;
};

struct bh bh(__bf16 a, _Float16 b) {
  // CHECK: define{{.*}}<2 x bfloat> @
  struct bh x;
  x.a = a;
  x.b = b;
  return x;
}

struct hb {
  _Float16 a;
  __bf16 b;
};

struct hb hb(_Float16 a, __bf16 b) {
  // CHECK: define{{.*}}<2 x half> @
  struct hb x;
  x.a = a;
  x.b = b;
  return x;
}
// end INTEL_FEATURE_ISA_BF16_BASE
