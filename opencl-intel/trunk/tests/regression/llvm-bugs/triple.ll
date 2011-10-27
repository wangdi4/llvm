; RUN: llvm-as %s -o %t.bc
; RUN: llc %t.bc -o %t1.ll
; ModuleID = 'bugpoint-reduced-simplified.bc'
target triple = "x86_64-pc-win32"
; CHECK: ret
define void @test() nounwind {
entry:
  %mem16 = alloca [3 x <16 x i32>], align 64
  %0 = ptrtoint [3 x <16 x i32>]* %mem16 to i64
  store i64 %0, i64 addrspace(1)* undef
  ret void
}
