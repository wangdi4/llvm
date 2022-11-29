// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fintel-compatibility -Wno-implicit-function-declaration -w -S -o - -emit-llvm              %s | FileCheck %s --check-prefix=NO_ERRNO
// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fintel-compatibility -Wno-implicit-function-declaration -w -S -o - -emit-llvm -fmath-errno %s | FileCheck %s --check-prefix=HAS_ERRNO
// RUN: %clang_cc1 -triple x86_64-unknown-windows-msvc -fintel-compatibility -Wno-implicit-function-declaration -w -S -o - -emit-llvm              %s | FileCheck %s --check-prefix=NO_ERRNO_WIN
// RUN: %clang_cc1 -triple x86_64-unknown-windows-msvc -fintel-compatibility -Wno-implicit-function-declaration -w -S -o - -emit-llvm -fmath-errno %s | FileCheck %s --check-prefix=HAS_ERRNO_WIN

// Test attributes and builtin codegen of math library calls.

void foo(float f) {
    ldexp(f,f);    ldexpf(f,f);   ldexpl(f,f);

// NO_ERRNO: declare double @llvm.ldexp.f64(double, i32) [[READNONE_INTRINSIC:#[0-9]+]]
// NO_ERRNO: declare float @llvm.ldexp.f32(float, i32) [[READNONE_INTRINSIC]]
// NO_ERRNO: declare x86_fp80 @llvm.ldexp.f80(x86_fp80, i32) [[READNONE_INTRINSIC]]
// HAS_ERRNO: declare double @ldexp(double noundef, i32 noundef) [[NOUNWIND:#[0-9]+]]
// HAS_ERRNO: declare float @ldexpf(float noundef, i32 noundef) [[NOUNWIND]]
// HAS_ERRNO: declare x86_fp80 @ldexpl(x86_fp80 noundef, i32 noundef) [[NOUNWIND]]
// NO_ERRNO_WIN: declare double @llvm.ldexp.f64(double, i32) [[READNONE_INTR_WIN:#[0-9]+]]
// NO_ERRNO_WIN: declare float @llvm.ldexp.f32(float, i32) [[READNONE_INTR_WIN]]
// HAS_ERRNO_WIN: declare {{.*}} double @ldexp(double noundef, i32 noundef) [[NOUNWIND_WIN:#[0-9]+]]
// HAS_ERRNO_WIN: declare {{.*}} float @ldexpf(float noundef, i32 noundef) [[NOUNWIND_WIN]]
// HAS_ERRNO_WIN: declare {{.*}} double @ldexpl(double noundef, i32 noundef) [[NOUNWIND_WIN]]
};

// NO_ERRNO: attributes [[READNONE_INTRINSIC]] = { {{.*}}memory(none){{.*}} }
// HAS_ERRNO: attributes [[NOUNWIND]] = { nounwind {{.*}} }
// NO_ERRNO_WIN: attributes [[READNONE_INTR_WIN]] = { {{.*}}memory(none){{.*}} }
// HAS_ERRNO_WIN: attributes [[NOUNWIND_WIN]] = { nounwind {{.*}} }
