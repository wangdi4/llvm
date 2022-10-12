// REQUIRES: x86-registered-target
<<<<<<< HEAD
// INTEL_CUSTOMIZATION
// RUN: %clang -Xclang -opaque-pointers -S -emit-llvm -ffp-model=fast -emit-llvm %s -o - \
// RUN: | FileCheck %s --check-prefixes=CHECK,CHECK-FAST

// RUN: %clang -Xclang -opaque-pointers -S -emit-llvm -ffp-model=precise %s -o - \
// RUN: | FileCheck %s --check-prefixes=CHECK,CHECK-PRECISE

// RUN: %clang -Xclang -opaque-pointers -S -emit-llvm -ffp-model=strict %s -o - \
// RUN: -target x86_64 | FileCheck %s --check-prefixes=CHECK,CHECK-STRICT

// RUN: %clang -Xclang -opaque-pointers -S -emit-llvm -ffp-model=strict -ffast-math \
// RUN: -target x86_64 %s -o - | FileCheck %s \
// RUN: --check-prefixes CHECK,CHECK-STRICT-FAST

// RUN: %clang -Xclang -opaque-pointers -S -emit-llvm -ffp-model=precise -ffast-math \
// RUN: %s -o - | FileCheck %s --check-prefixes CHECK,CHECK-FAST1
// end INTEL_CUSTOMIZATION
=======
// INTEL RUN: %clang -Xclang -opaque-pointers -S -emit-llvm -ffp-model=fast -emit-llvm %s -o - \
// INTEL RUN: | FileCheck %s --check-prefixes=CHECK,CHECK-FAST

// INTEL RUN: %clang -Xclang -opaque-pointers -S -emit-llvm -ffp-model=precise %s -o - \
// INTEL RUN: | FileCheck %s --check-prefixes=CHECK,CHECK-PRECISE

// INTEL RUN: %clang -Xclang -opaque-pointers -S -emit-llvm -ffp-model=strict %s -o - \
// INTEL RUN: -target x86_64 | FileCheck %s --check-prefixes=CHECK,CHECK-STRICT

// INTEL RUN: %clang -Xclang -opaque-pointers -S -emit-llvm -ffp-model=strict -ffast-math \
// INTEL RUN: -target x86_64 %s -o - | FileCheck %s \
// INTEL RUN: --check-prefixes CHECK,CHECK-STRICT-FAST

// INTEL RUN: %clang -Xclang -opaque-pointers -S -emit-llvm -ffp-model=precise -ffast-math \
// INTEL RUN: %s -o - | FileCheck %s --check-prefixes CHECK,CHECK-FAST1
>>>>>>> 4b49e7e9598858a7bd0f2bd4bdb0ad17e2413ecd

float mymuladd(float x, float y, float z) {
  // CHECK: define{{.*}} float @mymuladd
  return x * y + z;

  // CHECK-FAST: fmul fast float
  // CHECK-FAST: load float, ptr
  // CHECK-FAST: fadd fast float

  // CHECK-PRECISE: load float, ptr
  // CHECK-PRECISE: load float, ptr
  // CHECK-PRECISE: load float, ptr
  // CHECK-PRECISE: call float @llvm.fmuladd.f32(float {{.*}}, float {{.*}}, float {{.*}})

  // CHECK-STRICT: load float, ptr
  // CHECK-STRICT: load float, ptr
  // CHECK-STRICT: call float @llvm.experimental.constrained.fmul.f32(float {{.*}}, float {{.*}}, {{.*}})
  // CHECK-STRICT: load float, ptr
  // CHECK-STRICT: call float @llvm.experimental.constrained.fadd.f32(float {{.*}}, float {{.*}}, {{.*}})

  // CHECK-STRICT-FAST: load float, ptr
  // CHECK-STRICT-FAST: load float, ptr
  // CHECK-STRICT-FAST: call fast float @llvm.experimental.constrained.fmul.f32(float {{.*}}, float {{.*}}, {{.*}})
  // CHECK-STRICT-FAST: load float, ptr
  // CHECK-STRICT-FAST: call fast float @llvm.experimental.constrained.fadd.f32(float {{.*}}, float {{.*}}, {{.*}}

  // CHECK-FAST1: load float, ptr
  // CHECK-FAST1: load float, ptr
  // CHECK-FAST1: fmul fast float {{.*}}, {{.*}}
  // CHECK-FAST1: load float, ptr {{.*}}
  // CHECK-FAST1: fadd fast float {{.*}}, {{.*}}
}
