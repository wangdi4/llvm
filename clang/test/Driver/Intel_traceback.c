// Check that -traceback is disabled by default.
// RUN: %clang -### -S -target x86_64 %s 2>&1 | FileCheck %s --check-prefixes=NOTRACEBACK
// RUN: %clang -### -S -g -target x86_64 %s 2>&1 | FileCheck %s --check-prefixes=NOTRACEBACK
// NOTRACEBACK-NOT: -traceback

// Check that -traceback is passed to the correct compiler's options.
// RUN: %clang -### -S -traceback -target x86_64-linux-gnu %s 2> %t1
// RUN: FileCheck < %t1 %s --check-prefixes=TRACEBACK
// RUN: %clang -### -S -traceback -target x86_64-windows-msvc %s 2> %t2
// RUN: FileCheck < %t2 %s --check-prefixes=TRACEBACK
// RUN: %clang_cl -### -S -traceback -target x86_64-windows-msvc %s 2> %t3
// RUN: FileCheck < %t3 %s --check-prefixes=TRACEBACK
// TRACEBACK: -traceback
// TRACEBACK: -debug-info-kind=line-directives-only

// Check that -traceback is independent on dwarf or codeview.
// RUN: FileCheck < %t1 %s --check-prefixes=NODWARF
// RUN: FileCheck < %t2 %s --check-prefixes=NOCODEVIEW
// NODWARF-NOT: -dwarf-version=
// NOCODEVIEW-NOT: -gcodeview

// Check that -traceback can be used along with -g. The default debug info kind
// for -g is limited, we need to check the debug info kind is not downgraded
// due to -traceback.
// RUN: %clang -### -S -traceback -g -target x86_64 %s 2>&1 | FileCheck %s --check-prefixes=TRACEBACKWITHG
// TRACEBACKWITHG: -traceback
// TRACEBACKWITHG: -debug-info-kind=constructor

/// Unsupported on other targets.
// RUN: not %clang -### -S -traceback -target aarch64  %s 2>&1 | FileCheck --check-prefix=TARGET %s
// TARGET: unsupported option '-traceback' for target 'aarch64'

// -traceback requires lgcc_s
// RUN: %clang -### -traceback -target x86_64-unknown-linux-gnu  %s 2>&1 \
// RUN:  | FileCheck -check-prefix=LIBGCC_S %s
// LIBGCC_S: "-lgcc_s"
// LIBGCC_S-NOT: "--as-needed" "-lgcc_s" "--no-as-needed"

// -traceback for Windows requires -incremental:no
// RUN: %clang_cl -### -traceback -target x86_64-windows-msvc %s 2>&1 \
// RUN:  | FileCheck --check-prefix TRACEBACK_LINK %s
// TRACEBACK_LINK: "-incremental:no"
