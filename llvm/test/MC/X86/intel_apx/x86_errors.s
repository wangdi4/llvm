// RUN: not llvm-mc -triple i386-unknown-unknown %s 2> %t.err
// RUN: FileCheck --check-prefix=32 < %t.err %s

// NOTE: This is a smoke test. No need to check all the r16-r31
// b/c we will check REX2 is used for apx extended register.

// TODO: Merge to x86_errors.s after disclose.

// 32: error: register %r16d is only available in 64-bit mode
movl %eax, %r16d
