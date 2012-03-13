; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

define <16 x i32> @funcV16I32(<16 x i32> %v1, <16 x i32> %v2, <16 x i32> %v3, <16 x i32> %v4) nounwind {
; KNF:  vcmppi    {eq}, %v1, %v0, %k1
; KNF:  vorpi     %v2, %v2, %v3{%k1}
  %t0 = icmp eq <16 x i32> %v1, %v2
  %t1 = select <16 x i1> %t0, <16 x i32> %v3, <16 x i32> %v4
  ret <16 x i32> %t1
}

define <16 x i8> @funcV16I8(<16 x i8> %v1, <16 x i8> %v2, <16 x i8> %v3, <16 x i8> %v4) nounwind {
;KNF:  vandpi    _const_0(%rip), %v0, [[R0:%v[0-9]+]]
;KNF:  vandpi    _const_0(%rip), %v1, [[R1:%v[0-9]+]]
;KNF:  vcmppu    {nlt}, [[R1]], [[R0]], %k1
;KNF:  vorpi     %v2, %v2, %v3{%k1}
;KNF:  vorpi     %v3, %v3, %v0
  %t0 = icmp uge <16 x i8> %v1, %v2
  %t1 = select <16 x i1> %t0, <16 x i8> %v3, <16 x i8> %v4
  ret <16 x i8> %t1
}

define void @funcV16I8_2(<16 x i1> %mask, <16 x i8> *%p) nounwind alwaysinline {
FirstBB:
;KNF:  vxorpi    %v0, %v0, %v0
;KNF:  vloadd    (%rdi){uint8}, %v1
;KNF:  vorpi     %v0, %v0, %v1{%k1}
;KNF:  vstored   %v1{uint8}, (%rdi)
  %v = load <16 x i8> *%p
  %t1 = select <16 x i1> %mask, <16 x i8> zeroinitializer, <16 x i8> %v
  store <16 x i8> %t1, <16 x i8>* %p, align 16
  ret void
}
