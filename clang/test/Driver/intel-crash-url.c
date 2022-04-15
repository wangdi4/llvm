// RUN: not %clang -fsyntax-only %s 2>&1 | FileCheck -check-prefixes=CHECK_OPTIONS_LINUX,CHECK_MSG %s
// RUN: not %clang_cl -fsyntax-only %s 2>&1 | FileCheck -check-prefixes=CHECK_OPTIONS_WIN,CHECK_MSG %s

#pragma clang __debug parser_crash

// CHECK_MSG-NOT: bugs.llvm.org
// CHECK_MSG: PLEASE append the compiler options
// CHECK_OPTIONS_LINUX: "-save-temps -v"
// CHECK_OPTIONS_WIN: "/Qsave-temps -v"
// CHECK_MSG: rebuild the application to to get the full command which is failing and submit a bug report to https://software.intel.com/en-us/support/priority-support which includes the failing command, input files for the command and the crash backtrace (if any).
// CHECK_MSG-NOT: bugs.llvm.org
