// Test to be sure that the proper options are passed to the link step when
// LTO is enabled.
// REQUIRES: x86-registered-target

// RUN: %clang -target x86_64-unknown-linux -flto --intel -### -mllvm -dummy-option %s 2>&1 \
// RUN:  | FileCheck -check-prefix=CHECK_LTO %s
// CHECK_LTO: ld{{.*}} "-plugin-opt=mcpu=x86-64"
// CHECK_LTO: "-plugin-opt=O2"
// CHECK_LTO: "-plugin-opt=-vector-library=SVML"
// CHECK_LTO: "-plugin-opt=-disable-hir-generate-mkl-call"
// CHECK_LTO: "-plugin-opt=-intel-libirc-allowed"
// CHECK_LTO: "-plugin-opt=-dummy-option"

// RUN: %clang_cl --target=x86_64-pc-windows-msvc -flto -fuse-ld=lld --intel -mllvm -dummy-option -### %s 2>&1 \
// RUN: %clang_cl --target=x86_64-pc-windows-msvc -Qipo --intel -mllvm -dummy-option -### %s 2>&1 \
// RUN:  | FileCheck -check-prefix=CHECK_LTO_WIN %s
// CHECK_LTO_WIN: lld-link{{.*}} "-mllvm:-mcpu=x86-64"
// CHECK_LTO_WIN: "-opt:lldlto=2"
// CHECK_LTO_WIN: "-mllvm:-vector-library=SVML"
// CHECK_LTO_WIN: "-mllvm:-disable-hir-generate-mkl-call"
// CHECK_LTO_WIN: "-mllvm:-intel-libirc-allowed"
// CHECK_LTO_WIN: "-mllvm:-dummy-option"
