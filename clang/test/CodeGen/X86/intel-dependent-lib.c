// RUN: %clang_cc1 %s --dependent-lib=libcmt --dependent-lib=libmmt -triple i686-pc-win32 -fms-extensions -emit-llvm -o - | FileCheck -check-prefix WINDOWS %s
// RUN: %clang_cc1 %s --dependent-lib=libcmt --dependent-lib=libmmt -triple x86_64-pc-win32 -fms-extensions -emit-llvm -o - | FileCheck -check-prefix WINDOWS  %s
// RUN: %clang_cc1 %s --dependent-lib=libcmt --dependent-lib=libmmt -triple i686-pc-linux -emit-llvm -o - | FileCheck -check-prefix LINUX %s
// RUN: %clang_cc1 %s --dependent-lib=libcmt --dependent-lib=libmmt -triple x86_64-pc-linux -emit-llvm -o - | FileCheck -check-prefix LINUX %s

// WINDOWS: !llvm.linker.options = !{!0, !1, !2, !3, !4}
// WINDOWS: !0 = !{!"/DEFAULTLIB:libcpmt.lib"}
// WINDOWS: !1 = !{!"/DEFAULTLIB:kernel32.lib"}
// WINDOWS: !2 = !{!"/DEFAULTLIB:USER32.LIB"}
// WINDOWS: !3 = !{!"/DEFAULTLIB:libcmt.lib"}
// WINDOWS: !4 = !{!"/DEFAULTLIB:libmmt.lib"}

// LINUX: !llvm.dependent-libraries = !{!0, !1, !2, !3, !4}
// LINUX: !0 = !{!"libcmt"}
// LINUX: !1 = !{!"libmmt"}
// LINUX: !2 = !{!"libcpmt.lib"}
// LINUX: !3 = !{!"kernel32"}
// LINUX: !4 = !{!"USER32.LIB"}

#pragma comment(lib, "libcpmt.lib")
#pragma comment(lib, "kernel32")
#pragma comment(lib, "USER32.LIB")
