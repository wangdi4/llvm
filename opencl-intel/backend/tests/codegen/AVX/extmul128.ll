; RUN: llc -mcpu=sandybridge -march=x86-64 < %p/../../llvm/CodeGen/X86/extmul128.ll | grep mul | count 2
