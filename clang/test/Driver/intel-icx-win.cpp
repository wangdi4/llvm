// Verify that the use of icx on Windows accepts both Windows and Linux
// type options.  When a Windows type option is encounted, an additional
// diagnostic should be emitted.

// UNSUPPORTED: system-linux

// RUN: %clang --intel -Fodummy -fp-model=strict -### -c %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=USE_ICX_CL
// USE_ICX_CL: MSVC specific option '-Fodummy' encountered. Use 'icx-cl' to compile with MSVC command line compatibility
// USE_ICX_CL-NOT: unknown argument

// RUN: %clang --intel -Fodummy -### -c -Wno-invalid-command-line-argument %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=WARNING
// RUN: %clang_cl --intel -Fodummy -### -c %s 2>&1 \
// RUN:   | FileCheck %s --check-prefix=WARNING
// WARNING-NOT: MSVC specific option
