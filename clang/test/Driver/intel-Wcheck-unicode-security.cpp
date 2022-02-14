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
// CHECK_SECURITY-SAME: "--checks=misc-misleading-bidirectional,misc-misleading-identifier,misc-homoglyph"
// CHECK_SECURITY-SAME: "--"
// CHECK_SECURITY-SAME: "-D FOO" "-c"
