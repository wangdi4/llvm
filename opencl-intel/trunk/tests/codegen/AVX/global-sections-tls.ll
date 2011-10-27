; RUN: llc -mcpu=sandybridge  -mtriple=i386-unknown-linux-gnu  < %p/../../llvm/CodeGen/X86/global-sections-tls.ll | FileCheck %p/../../llvm/CodeGen/X86/global-sections-tls.ll -check-prefix=LINUX
