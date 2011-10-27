; http://llvm.org/bugs/show_bug.cgi?id=9158
; ModuleID = 'bugpoint-reduced-simplified.bc'
; RUN: llvm-as %s -o %t.bc
; RUN: llc %t.bc -o %t1.ll
target triple = "x86_64-unknown-linux-gnu"

define fastcc void @m_387() nounwind {
entry:
  %0 = load <4 x float>* undef
  %1 = load <4 x float>* null
  %merge68 = select <4 x i1> undef, <4 x float> %0, <4 x float> %1
  store <4 x float> %merge68, <4 x float> addrspace(1)* undef
  ret void
}
