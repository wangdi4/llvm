; RUN: llc -mcpu=sandybridge -mtriple=x86_64-apple-darwin10 -disable-fp-elim -o /dev/null < %p/../../llvm/CodeGen/X86/2010-06-24-g-constraint-crash.ll 
