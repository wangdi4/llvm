// RUN: llvm-mc -triple i386-pc-windows-msvc -x86-asm-syntax=intel -output-asm-variant=1 < %s | FileCheck %s

arpl BYTE PTR test_mem,ax
//CHECK: arpl word ptr [test_mem], ax
arpl WORD PTR test_mem,ax
//CHECK: arpl word ptr [test_mem], ax