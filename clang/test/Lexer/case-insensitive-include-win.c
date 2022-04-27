// Most Microsoft-specific testing should go in case-insensitive-include-ms.c
// This file should only include code that really needs a Windows host OS to
// run.
// INTEL_CUSTOMIZATION
// This will fail for us until community starts handling CPATH containing
// network mounted drives on Windows. The issue has been reported to the
// submitter of the test.
// UNSUPPORTED: system-windows
// end INTEL_CUSTOMIZATION

// REQUIRES: system-windows
// RUN: mkdir -p %t.dir
// RUN: touch %t.dir/foo.h
// RUN: not %clang_cl /FI\\?\%t.dir\FOO.h /WX -fsyntax-only %s 2>&1 | FileCheck %s

// CHECK: non-portable path to file '"\\?\
