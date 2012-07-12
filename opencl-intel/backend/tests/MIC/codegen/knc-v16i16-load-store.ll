; XFAIL: win32
; XFAIL: *
; Fails since PCG is missing an optimization for _mm512_extloaduel_epi32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC
;
target datalayout = "e-p:64:64"

define void @A(<16 x i16>* %a, <16 x i16>* %b) nounwind {
; KNF: vandpi    (%{{[a-z]*}}){uint16}, %v{{[0-9]*}}, %v{{[0-9]*}}
; KNF: vstored    %v{{[0-9]*}}{uint16i}, (%{{[a-z]+}})
; KNF: ret
;
; KNC:  vbroadcastss _const_0(%rip), [[V2:%v[0-9]+]]
; KNC:  vpandd    (%rdi){uint16}, [[V2]], [[V3:%v[0-9]+]]
; KNC:  vmovdqa32 [[V3]]{uint16}, (%rsi)
; KNC:  ret
;
  %1 = load <16 x i16>* %a
  store <16 x i16> %1, <16 x i16>* %b
  ret void
}

define void @B(<16 x i16>* %a, <16 x i16>* %b) nounwind {
; KNF: movq %rdi, [[R1:%[a-z]+]]
; KNF: vloadunpackld (%rdi){uint16i}, [[V0:%v[0-9]+]]
; KNF: andq $63, [[R1]]
; KNF: cmpq $32, [[R1]]
; KNF: jle
; KNF: vandpi _const_{{[0-9]}}(%rip){1to16}, [[V0]], [[V1:%v[0-9]+]]
; KNF: movq %rsi, [[R2:%[a-z]+]]
; KNF: vpackstoreld [[V1]]{uint16i}, (%rsi)
; KNF: andq $63, [[R1]]
; KNF: cmpq $32, [[R1]]
; KNF: jle
; KNF: vpackstorehd [[V1]]{uint16i}, 64(%rsi)
; KNF: ret
;
; KNC: movq %rdi, [[R1:%[a-z]+]]
; KNC: vloadunpackld (%rdi){uint16}, [[V0:%v[0-9]+]]
; KNC: andq $63, [[R1]]
; KNC: cmpq $32, [[R1]]
; KNC: jle
; KNC: vandd _const_{{[0-9]}}(%rip), [[V0]], [[V1:%v[0-9]+]]
; KNC: movq %rsi, [[R2:%[a-z]+]]
; KNC: vpackstoreld [[V1]]{uint16}, (%rsi)
; KNC: andq $63, [[R1]]
; KNC: cmpq $32, [[R1]]
; KNC: jle
; KNC: vpackstorehd [[V1]]{uint16}, 64(%rsi)
; KNC: ret
;
  %1 = load <16 x i16>* %a, align 2
  store <16 x i16> %1, <16 x i16>* %b, align 2
  ret void
}
