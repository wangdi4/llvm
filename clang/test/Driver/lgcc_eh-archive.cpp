// Test that gcc_eh is encapsulated with --whole-archive and --no-whole-archive
// when -static-libgcc is used.

// RUN: %clang -### --target=x86_64-unknown-linux-gnu -static-libgcc %s \
// RUN:     2>&1 | FileCheck --check-prefix ENABLE-ARCHIVE %s

// RUN: %clang -### --target=x86_64-unknown-linux-gnu -static %s \
// RUN:     2>&1 | FileCheck --check-prefix ENABLE-ARCHIVE %s

// ENABLE-ARCHIVE: "--whole-archive" "-lgcc_eh" "--no-whole-archive"

