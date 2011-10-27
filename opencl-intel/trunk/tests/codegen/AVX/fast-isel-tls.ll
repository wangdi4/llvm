; RUN: llc -mcpu=sandybridge -march=x86 -relocation-model=pic -mtriple=i686-unknown-linux-gnu -fast-isel < %p/../../llvm/CodeGen/X86/fast-isel-tls.ll | grep __tls_get_addr
