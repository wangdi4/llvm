; RUN: llc -mcpu=sandybridge -mtriple=i386-pc-linux-gnu < %p/../../llvm/CodeGen/X86/2009-08-06-inlineasm.ll 
; PR4668
; XFAIL: *
; FIXME: If the coalescer happens to coalesce %level.1 with the copy to EAX
; (for ret) then this will fail to compile. The fundamental problem is
; once the coalescer fixes a virtual register to physical register we can't
; evict it.
