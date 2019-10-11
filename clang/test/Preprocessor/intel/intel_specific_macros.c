// The same test for IL0 backend is in the Preprocessor/intel/il0-backend subfolder
// CQ#366613
// RUN: %clang_cc1 -E -dM < %s | FileCheck -check-prefix CHECK1 %s
// CHECK1-NOT: #define __LONG_DOUBLE_SIZE__

// RUN: %clang_cc1 -E -dM -fintel-compatibility < %s | FileCheck -check-prefix CHECK2 %s
// CHECK2: #define __LONG_DOUBLE_SIZE__ {{.+}}

// Linux
// RUN: %clang_cc1 -E -dM -fintel-compatibility -triple i386-none-linux < %s | FileCheck -check-prefix X86_LINUX %s
// X86_LINUX: #define __LONG_DOUBLE_SIZE__ 80
// RUN: %clang_cc1 -E -dM -fintel-compatibility -triple x86_64-none-linux < %s | FileCheck -check-prefix X86_64_LINUX %s
// X86_64_LINUX: #define __LONG_DOUBLE_SIZE__ 80

// Win32
// RUN: %clang_cc1 -E -dM -fintel-compatibility -triple i386-none-win32 < %s | FileCheck -check-prefix X86_WIN32 %s
// X86_WIN32: #define __LONG_DOUBLE_SIZE__ 64
// RUN: %clang_cc1 -E -dM -fintel-compatibility -triple x86_64-none-win32 < %s | FileCheck -check-prefix X86_64_WIN32 %s
// X86_64_WIN32: #define __LONG_DOUBLE_SIZE__ 64

// Cygwin
// RUN: %clang_cc1 -E -dM -fintel-compatibility -triple i386-none-cygwin < %s | FileCheck -check-prefix X86_CYGWIN %s
// X86_CYGWIN: #define __LONG_DOUBLE_SIZE__ 80
// RUN: %clang_cc1 -E -dM -fintel-compatibility -triple x86_64-none-cygwin < %s | FileCheck -check-prefix X86_64_CYGWIN %s
// X86_64_CYGWIN: #define __LONG_DOUBLE_SIZE__ 80


// CQ#369662
// RUN: %clang_cc1 -fgnuc-version=4.2.1 -x c++ -E -dM < %s | FileCheck -check-prefix CHECK_GNUG_1 %s
// RUN: %clang_cc1 -fgnuc-version=4.2.1 -x c++ -E -dM -fintel-compatibility < %s | FileCheck -check-prefix CHECK_GNUG_1 %s
// CHECK_GNUG_1: #define __GNUG__ 4

// CQ#373889
// RUN: %clang_cc1 -E -dM -fintel-compatibility -triple x86_64-none-linux -x c++ < %s | FileCheck -check-prefix CHECK_BOOL %s
// CHECK_BOOL: #define _BOOL 1
