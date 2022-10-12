<<<<<<< HEAD
// INTEL_CUSTOMIZATION
// RUN: %clang -target x86_64-linux-gnu -S -emit-llvm -o - -O0 \
// RUN:     -Xclang -opaque-pointers -disable-llvm-passes %s | FileCheck %s -check-prefix=CHECK-O0
// RUN: %clang -target x86_64-linux-gnu -S -emit-llvm -o - -O0 \
// RUN:     -fsanitize=address -fsanitize-address-use-after-scope \
// RUN:     -Xclang -opaque-pointers -disable-llvm-passes %s | FileCheck %s -check-prefix=LIFETIME
// RUN: %clang -Xclang -opaque-pointers -target x86_64-linux-gnu -S -emit-llvm -o - -O0 \
// RUN:     -fsanitize=memory -Xclang -disable-llvm-passes %s | \
// RUN:     FileCheck %s -check-prefix=LIFETIME
// RUN: %clang -Xclang -opaque-pointers -target aarch64-linux-gnu -S -emit-llvm -o - -O0 \
// RUN:     -fsanitize=hwaddress -Xclang -disable-llvm-passes %s | \
// RUN:     FileCheck %s -check-prefix=LIFETIME
// end INTEL_CUSTOMIZATION
=======
// INTEL RUN: %clang -target x86_64-linux-gnu -S -emit-llvm -o - -O0 \
// INTEL RUN:     -Xclang -opaque-pointers -disable-llvm-passes %s | FileCheck %s -check-prefix=CHECK-O0
// INTEL RUN: %clang -target x86_64-linux-gnu -S -emit-llvm -o - -O0 \
// INTEL RUN:     -fsanitize=address -fsanitize-address-use-after-scope \
// INTEL RUN:     -Xclang -opaque-pointers -disable-llvm-passes %s | FileCheck %s -check-prefix=LIFETIME
// INTEL RUN: %clang -Xclang -opaque-pointers -target x86_64-linux-gnu -S -emit-llvm -o - -O0 \
// INTEL RUN:     -fsanitize=memory -Xclang -disable-llvm-passes %s | \
// INTEL RUN:     FileCheck %s -check-prefix=LIFETIME
// INTEL RUN: %clang -Xclang -opaque-pointers -target aarch64-linux-gnu -S -emit-llvm -o - -O0 \
// INTEL RUN:     -fsanitize=hwaddress -Xclang -disable-llvm-passes %s | \
// INTEL RUN:     FileCheck %s -check-prefix=LIFETIME
>>>>>>> 4b49e7e9598858a7bd0f2bd4bdb0ad17e2413ecd

extern int bar(char *A, int n);

// CHECK-O0-NOT: @llvm.lifetime.start
int foo(int n) {
  if (n) {
    // LIFETIME: @llvm.lifetime.start.p0(i64 10, ptr {{.*}})
    char A[10];
    return bar(A, 1);
    // LIFETIME: @llvm.lifetime.end.p0(i64 10, ptr {{.*}})
  } else {
    // LIFETIME: @llvm.lifetime.start.p0(i64 20, ptr {{.*}})
    char A[20];
    return bar(A, 2);
    // LIFETIME: @llvm.lifetime.end.p0(i64 20, ptr {{.*}})
  }
}
