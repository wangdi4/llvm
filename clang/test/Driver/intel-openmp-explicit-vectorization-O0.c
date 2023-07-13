// REQUIRES: x86-registered-target

// RUN: %clang -O0 -fiopenmp -c %s -### 2>&1 | FileCheck %s --check-prefixes=CHECK-VEC,CHECK-NOVEC-LTO
// RUN: %clang -O0 -fiopenmp-simd -c %s -### 2>&1 | FileCheck %s --check-prefixes=CHECK-VEC,CHECK-NOVEC-LTO
// RUN: %clang -O0 -fiopenmp -fiopenmp-vec-at-O0 -c %s -### 2>&1 | FileCheck %s --check-prefixes=CHECK-VEC,CHECK-NOVEC-LTO
// RUN: %clang -O0 -fiopenmp-simd -fiopenmp-vec-at-O0 -c %s -### 2>&1 | FileCheck %s --check-prefixes=CHECK-VEC,CHECK-NOVEC-LTO
// RUN: %clang -O0 -fiopenmp -fno-iopenmp-vec-at-O0 -c %s -### 2>&1 | FileCheck %s --check-prefixes=CHECK-NOVEC,CHECK-NOVEC-LTO
// RUN: %clang -O0 -fiopenmp-simd -fno-iopenmp-vec-at-O0 -c %s -### 2>&1 | FileCheck %s --check-prefixes=CHECK-NOVEC,CHECK-NOVEC-LTO
// RUN: %clang -O2 -fiopenmp -c %s -### 2>&1 | FileCheck %s --check-prefixes=CHECK-NOVEC,CHECK-NOVEC-LTO
// RUN: %clang -O2 -fiopenmp-simd -c %s -### 2>&1 | FileCheck %s --check-prefixes=CHECK-NOVEC,CHECK-NOVEC-LTO
// RUN: %clang -O2 -fiopenmp -fiopenmp-vec-at-O0 -c %s -### 2>&1 | FileCheck %s --check-prefixes=CHECK-NOVEC,CHECK-NOVEC-LTO
// RUN: %clang -O2 -fiopenmp-simd -fiopenmp-vec-at-O0 -c %s -### 2>&1 | FileCheck %s --check-prefixes=CHECK-NOVEC,CHECK-NOVEC-LTO
// RUN: %clang -O2 -fiopenmp -fno-iopenmp-vec-at-O0 -c %s -### 2>&1 | FileCheck %s --check-prefixes=CHECK-NOVEC,CHECK-NOVEC-LTO
// RUN: %clang -O2 -fiopenmp-simd -fno-iopenmp-vec-at-O0 -c %s -### 2>&1 | FileCheck %s --check-prefixes=CHECK-NOVEC,CHECK-NOVEC-LTO

// RUN: %clang -target x86_64-unknown-linux -flto --intel -O0 -fiopenmp %s -### 2>&1 | FileCheck %s --check-prefixes=CHECK-VEC,CHECK-VEC-LTO-LIN
// RUN: %clang -target x86_64-unknown-linux -flto --intel -O0 -fiopenmp-simd %s -### 2>&1 | FileCheck %s --check-prefixes=CHECK-VEC,CHECK-VEC-LTO-LIN
// RUN: %clang -target x86_64-unknown-linux -flto --intel -O0 -fiopenmp -fiopenmp-vec-at-O0 %s -### 2>&1 | FileCheck %s --check-prefixes=CHECK-VEC,CHECK-VEC-LTO-LIN
// RUN: %clang -target x86_64-unknown-linux -flto --intel -O0 -fiopenmp-simd -fiopenmp-vec-at-O0 %s -### 2>&1 | FileCheck %s --check-prefixes=CHECK-VEC,CHECK-VEC-LTO-LIN
// RUN: %clang -target x86_64-unknown-linux -flto --intel -O0 -fiopenmp -fno-iopenmp-vec-at-O0 %s -### 2>&1 | FileCheck %s --check-prefixes=CHECK-NOVEC,CHECK-NOVEC-LTO
// RUN: %clang -target x86_64-unknown-linux -flto --intel -O0 -fiopenmp-simd -fno-iopenmp-vec-at-O0 %s -### 2>&1 | FileCheck %s --check-prefixes=CHECK-NOVEC,CHECK-NOVEC-LTO
// RUN: %clang -target x86_64-unknown-linux -flto --intel -O2 -fiopenmp %s -### 2>&1 | FileCheck %s --check-prefixes=CHECK-NOVEC,CHECK-NOVEC-LTO
// RUN: %clang -target x86_64-unknown-linux -flto --intel -O2 -fiopenmp-simd %s -### 2>&1 | FileCheck %s --check-prefixes=CHECK-NOVEC,CHECK-NOVEC-LTO
// RUN: %clang -target x86_64-unknown-linux -flto --intel -O2 -fiopenmp -fiopenmp-vec-at-O0 %s -### 2>&1 | FileCheck %s --check-prefixes=CHECK-NOVEC,CHECK-NOVEC-LTO
// RUN: %clang -target x86_64-unknown-linux -flto --intel -O2 -fiopenmp-simd -fiopenmp-vec-at-O0 %s -### 2>&1 | FileCheck %s --check-prefixes=CHECK-NOVEC,CHECK-NOVEC-LTO
// RUN: %clang -target x86_64-unknown-linux -flto --intel -O2 -fiopenmp -fno-iopenmp-vec-at-O0 %s -### 2>&1 | FileCheck %s --check-prefixes=CHECK-NOVEC,CHECK-NOVEC-LTO
// RUN: %clang -target x86_64-unknown-linux -flto --intel -O2 -fiopenmp-simd -fno-iopenmp-vec-at-O0 %s -### 2>&1 | FileCheck %s --check-prefixes=CHECK-NOVEC,CHECK-NOVEC-LTO

// RUN: %clang_cl --target=x86_64-pc-windows-msvc -flto -fuse-ld=lld --intel -Od -Qiopenmp %s -### 2>&1 | FileCheck %s --check-prefixes=CHECK-VEC,CHECK-VEC-LTO-WIN
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -flto -fuse-ld=lld --intel -Od -Qiopenmp-simd %s -### 2>&1 | FileCheck %s --check-prefixes=CHECK-VEC,CHECK-VEC-LTO-WIN
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -flto -fuse-ld=lld --intel -Od -Qiopenmp -Qiopenmp-vec-at-O0 %s -### 2>&1 | FileCheck %s --check-prefixes=CHECK-VEC,CHECK-VEC-LTO-WIN
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -flto -fuse-ld=lld --intel -Od -Qiopenmp-simd -Qiopenmp-vec-at-O0 %s -### 2>&1 | FileCheck %s --check-prefixes=CHECK-VEC,CHECK-VEC-LTO-WIN
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -flto -fuse-ld=lld --intel -Od -Qiopenmp -Qno-iopenmp-vec-at-O0 %s -### 2>&1 | FileCheck %s --check-prefixes=CHECK-NOVEC,CHECK-NOVEC-LTO
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -flto -fuse-ld=lld --intel -Od -Qiopenmp-simd -Qno-iopenmp-vec-at-O0 %s -### 2>&1 | FileCheck %s --check-prefixes=CHECK-NOVEC,CHECK-NOVEC-LTO
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -flto -fuse-ld=lld --intel -O2 -Qiopenmp %s -### 2>&1 | FileCheck %s --check-prefixes=CHECK-NOVEC,CHECK-NOVEC-LTO
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -flto -fuse-ld=lld --intel -O2 -Qiopenmp-simd %s -### 2>&1 | FileCheck %s --check-prefixes=CHECK-NOVEC,CHECK-NOVEC-LTO
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -flto -fuse-ld=lld --intel -O2 -Qiopenmp -Qiopenmp-vec-at-O0 %s -### 2>&1 | FileCheck %s --check-prefixes=CHECK-NOVEC,CHECK-NOVEC-LTO
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -flto -fuse-ld=lld --intel -O2 -Qiopenmp-simd -Qiopenmp-vec-at-O0 %s -### 2>&1 | FileCheck %s --check-prefixes=CHECK-NOVEC,CHECK-NOVEC-LTO
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -flto -fuse-ld=lld --intel -O2 -Qiopenmp -Qno-iopenmp-vec-at-O0 %s -### 2>&1 | FileCheck %s --check-prefixes=CHECK-NOVEC,CHECK-NOVEC-LTO
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -flto -fuse-ld=lld --intel -O2 -Qiopenmp-simd -Qno-iopenmp-vec-at-O0 %s -### 2>&1 | FileCheck %s --check-prefixes=CHECK-NOVEC,CHECK-NOVEC-LTO

// CHECK-VEC: "-mllvm" "-vecopt=true" "-mllvm" "-enable-vec-clone=true" "-mllvm" "-enable-o0-vectorization=true"
// CHECK-NOVEC-NOT: {{-vecopt=|-enable-vec-clone|-enable-o0-vectorization}}
// CHECK-NOVEC-LTO-NOT: ld{{.*}}{{-vecopt=|-enable-vec-clone|-enable-o0-vectorization}}
// CHECK-VEC-LTO-LIN: ld{{.*}} "-plugin-opt=-vecopt=true" "-plugin-opt=-enable-vec-clone=true" "-plugin-opt=-enable-o0-vectorization=true"
// CHECK-VEC-LTO-WIN: lld-link{{.*}} "-mllvm:-vecopt=true" "-mllvm:-enable-vec-clone=true" "-mllvm:-enable-o0-vectorization=true"
