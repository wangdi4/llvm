// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fintel-compatibility -w -S -o - -emit-llvm              %s | FileCheck %s --check-prefix=NO_ERRNO
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fintel-compatibility -w -S -o - -emit-llvm -fmath-errno %s | FileCheck %s --check-prefix=HAS_ERRNO
// RUN: %clang_cc1 -triple x86_64-unknown-windows-msvc -fintel-compatibility -w -S -o - -emit-llvm              %s | FileCheck %s --check-prefix=NO_ERRNO_WIN
// RUN: %clang_cc1 -triple x86_64-unknown-windows-msvc -fintel-compatibility -w -S -o - -emit-llvm -fmath-errno %s | FileCheck %s --check-prefix=HAS_ERRNO_WIN

// Test attributes and builtin codegen of math library calls.

void foo(float f) {
    ldexp(f,f);    ldexpf(f,f);   ldexpl(f,f);

// NO_ERRNO: declare double @llvm.ldexp.f64(double, i32) [[READNONE_INTRINSIC:#[0-9]+]]
// NO_ERRNO: declare float @llvm.ldexp.f32(float, i32) [[READNONE_INTRINSIC]]
// NO_ERRNO: declare x86_fp80 @llvm.ldexp.f80(x86_fp80, i32) [[READNONE_INTRINSIC]]
// HAS_ERRNO: declare double @ldexp(double, i32) [[NOUNWIND:#[0-9]+]]
// HAS_ERRNO: declare float @ldexpf(float, i32) [[NOUNWIND]]
// HAS_ERRNO: declare x86_fp80 @ldexpl(x86_fp80, i32) [[NOUNWIND]]
// NO_ERRNO_WIN: declare double @llvm.ldexp.f64(double, i32) [[READNONE_INTR_WIN:#[0-9]+]]
// NO_ERRNO_WIN: declare float @llvm.ldexp.f32(float, i32) [[READNONE_INTR_WIN]]
// HAS_ERRNO_WIN: declare {{.*}} double @ldexp(double, i32) [[NOUNWIND_WIN:#[0-9]+]]
// HAS_ERRNO_WIN: declare {{.*}} float @ldexpf(float, i32) [[NOUNWIND_WIN]]
// HAS_ERRNO_WIN: declare {{.*}} double @ldexpl(double, i32) [[NOUNWIND_WIN]]
};

// NO_ERRNO: attributes [[READNONE_INTRINSIC]] = { {{.*}}readnone{{.*}} }
// HAS_ERRNO: attributes [[NOUNWIND]] = { nounwind {{.*}} }
// NO_ERRNO_WIN: attributes [[READNONE_INTR_WIN]] = { {{.*}}readnone{{.*}} }
// HAS_ERRNO_WIN: attributes [[NOUNWIND_WIN]] = { nounwind {{.*}} }
