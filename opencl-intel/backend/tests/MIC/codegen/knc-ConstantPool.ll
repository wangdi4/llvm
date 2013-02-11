; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s
;
;

target datalayout = "e-p:64:64"

@gb = common global <16 x i32> zeroinitializer, align 64
@pgb = common global <16 x i32>* null, align 8

define <16 x i32> @test1(<16 x i32> %a, <16 x i32> %b) nounwind readnone ssp {
entry:
; CHECK: _const_0:
; CHECK: .long 42
; CHECK-NOT: .long
; CHECK: test1
; CHECK: vbroadcasts{{[ds]}} _const_0(%rip), %zmm0
  ret <16 x i32> <i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42>
}

define <16 x float> @test2(<16 x float> %a, <16 x float> %b) nounwind readnone ssp {
entry:
; CHECK: _const_1:
; CHECK: .long 0x42280000
; CHECK-NOT: .long
; CHECK: test2
; CHECK: vbroadcasts{{[ds]}} _const_1(%rip), %zmm0
  ret <16 x float> <float 42.0, float 42.0, float 42.0, float 42.0, float 42.0, float 42.0, float 42.0, float 42.0, float 42.0, float 42.0, float 42.0, float 42.0, float 42.0, float 42.0, float 42.0, float 42.0>

}

define <1 x float> @test3(<16 x float> %a, <16 x float> %b) nounwind readnone ssp {
entry:
; CHECK: _const_2:
; CHECK: .long 0x42280000
; CHECK-NOT: .long
; CHECK: test3
; CHECK: vbroadcastss _const_2(%rip), %zmm0
  ret <1 x float> <float 42.0>
}
