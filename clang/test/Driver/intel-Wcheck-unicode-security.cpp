/// Test behavior of -Wcheck-unicode-security, which uses clang-tidy

// -Wno-check-unicode-security
// RUN: %clangxx -Wno-check-unicode-security -c -### %s 2>&1 \
// RUN:   | FileCheck --check-prefix=NO_CHECK_SECURITY %s
// RUN: %clang_cl /Wno-check-unicode-security -c -### %s 2>&1 \
// RUN:   | FileCheck --check-prefix=NO_CHECK_SECURITY %s
// RUN: %clangxx -c -### %s 2>&1 \
// RUN:   | FileCheck --check-prefix=NO_CHECK_SECURITY %s
// NO_CHECK_SECURITY-NOT: clang-tidy{{.*}}

// -Wcheck-unicode-security
// RUN: %clangxx -Wcheck-unicode-security -DFOO -c -### %s 2>&1 \
// RUN:   | FileCheck --check-prefix=CHECK_SECURITY %s
// RUN: %clang_cl /Wcheck-unicode-security -DFOO -c -### %s 2>&1 \
// RUN:   | FileCheck --check-prefix=CHECK_SECURITY %s
// CHECK_SECURITY: clang-tidy
// CHECK_SECURITY-SAME: "--checks=misc-misleading-bidirectional,misc-misleading-identifier,misc-confusable-identifiers"
// CHECK_SECURITY-SAME: "--"
// CHECK_SECURITY-SAME: "-D FOO" "-c"

// -Wcheck-unicode-security should not be passed to clang-tidy
// RUN: %clangxx -Wcheck-unicode-security -### %s 2>&1 \
// RUN:   | FileCheck --check-prefix=NO_SECURITY_OPT %s
// RUN: %clang_cl /Wcheck-unicode-security -### %s 2>&1 \
// RUN:   | FileCheck --check-prefix=NO_SECURITY_OPT %s
// NO_SECURITY_OPT-NOT: clang-tidy{{.*}} .*Wcheck-unicode-security

// /Tp should not be passed for MSVC
// RUN: %clang_cl /Wcheck-unicode-security -### /Tp%s 2>&1 \
// RUN:   | FileCheck --check-prefix=NO_TP_SECURITY_OPT %s
// NO_TP_SECURITY_OPT-NOT: clang-tidy{{.*}} .*Tp

// /Tc should not be passed for MSVC
// RUN: %clang_cl /Wcheck-unicode-security -### /Tc%s 2>&1 \
// RUN:   | FileCheck --check-prefix=NO_TC_SECURITY_OPT %s
// NO_TC_SECURITY_OPT-NOT: clang-tidy{{.*}} .*Tc

// -fveclib should not be passed for MSVC
// RUN: %clang_cl /Wcheck-unicode-security -### %s 2>&1 \
// RUN:   | FileCheck --check-prefix=NO_VECLIB_SECURITY_OPT %s
// NO_VECLIB_SECURITY_OPT-NOT: clang-tidy{{.*}} .*fveclib

// -fsycl needs to be passed to clang-tidy for DPC++
// RUN: %clangxx -Wcheck-unicode-security -fsycl -### %s 2>&1 \
// RUN:   | FileCheck --check-prefix=SYCL %s
// RUN: %clang_cl /Wcheck-unicode-security -fsycl -### %s 2>&1 \
// RUN:   | FileCheck --check-prefix=SYCL %s
// SYCL: clang-tidy{{.*}} "-fsycl"
