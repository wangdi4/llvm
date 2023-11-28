// Test that gcc_eh is forced to link library modules that define _Unwind_ForcedUnwind
// when -static-libgcc is used.

// RUN: %clang -### --target=x86_64-unknown-linux-gnu -static-libgcc %s \
// RUN:     2>&1 | FileCheck --check-prefix FORCE-UNWIND %s

// RUN: %clang -### --target=x86_64-unknown-linux-gnu -static %s \
// RUN:     2>&1 | FileCheck --check-prefix FORCE-UNWIND %s

// FORCE-UNWIND: "-u" "_Unwind_ForcedUnwind" "-lgcc"
