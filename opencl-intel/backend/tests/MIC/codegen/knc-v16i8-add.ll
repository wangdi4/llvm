; XFAIL: *
; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

define void @func16(<16 x i1> %mask, <16 x i8> *%p) nounwind alwaysinline {
FirstBB:
;KNF:  vloadd    (%rdi){uint8i}, [[R0:%v[0-9]+]]
;KNF:  vaddpi    [[R0]], [[R0]], [[R1:%v[0-9]+]]
;KNF:  vxorpi    (%rdi){uint8}, [[R1]], [[R2:%v[0-9]+]]
;KNF:  vstored   [[R2]]{uint8i}, (%rdi)
  %v = load <16 x i8> *%p
  %t1 = add <16 x i8> %v, %v
  %v1 = xor <16 x i8> %t1, %v
  store <16 x i8> %v1, <16 x i8>* %p, align 16
  ret void
}

define void @func8(<8 x i1> %mask, <8 x i8> *%p) nounwind alwaysinline {
FirstBB:
;KNF: vadd
;KNF: vxor
  %v = load <8 x i8> *%p
  %t1 = add <8 x i8> %v, %v
  %v1 = xor <8 x i8> %t1, %v
  store <8 x i8> %v1, <8 x i8>* %p, align 8
  ret void
}

define void @func4(<4 x i1> %mask, <4 x i8> *%p) nounwind alwaysinline {
FirstBB:
;KNF: movb
;KNF: addb
;KNF: xorb
;KNF: movb
  %v = load <4 x i8> *%p
  %t1 = add <4 x i8> %v, %v
  %v1 = xor <4 x i8> %t1, %v
  store <4 x i8> %v1, <4 x i8>* %p, align 8
  ret void
}

define void @func16_1(<16 x i1> %mask, <16 x i8> *%p) nounwind alwaysinline {
FirstBB:
;KNF:  vloadd    (%rdi){uint8i}, [[R0:%v[0-9]+]]
;KNF:  vaddpi    [[R0]], [[R0]], [[R1:%v[0-9]+]]
;KNF:  vxorpi    (%rdi){uint8}, [[R1]], [[R2:%v[0-9]+]]
;KNF-not:  vstored   [[R2]]{uint8i}, (%rdi)
;KNF:  ret
  %v = load <16 x i8> *%p
  %t1 = add <16 x i8> %v, %v
  %v1 = xor <16 x i8> %t1, %v
  store <16 x i8> %v1, <16 x i8>* %p, align 1
  ret void
}

