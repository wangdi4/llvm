; REQUIRES: intel_feature_isa_amx_lnc
; RUN: llc < %s -O0 -mtriple=x86_64-unknown-unknown -mattr=+avx512f,+amx-tile,+amx-bf16,+amx-int8,+amx-transpose,+amx-avx512,+amx-memory | FileCheck %s

; CHECK-LABEL: test_amx:
; CHECK:       # %bb.0:
; amx-avx512
; CHECK:    tilemovrow $9, %tmm7, %zmm{{.*}}
; CHECK:    tilemovrow %{{.*}}, %tmm7, %zmm{{.*}}
; CHECK:    tilemovrow %xmm{{.*}}, %tmm7, %zmm{{.*}}
; amx-transpose
; CHECK:    t2rpntlvw       %{{.*}}, (%{{.*}},%{{.*}}), %tmm0
; CHECK:    t2rpntlvwt1     %{{.*}}, (%{{.*}},%{{.*}}), %tmm2
; CHECK:    t2rpntlvwz0     (%{{.*}},%{{.*}}), %tmm0
; CHECK:    t2rpntlvwz0t1   (%{{.*}},%{{.*}}), %tmm2
; CHECK:    t2rpntlvwz1     (%{{.*}},%{{.*}}), %tmm0
; CHECK:    t2rpntlvwz1t1   (%{{.*}},%{{.*}}), %tmm2
; CHECK:    ttdpbf16ps      %tmm3, %tmm2, %tmm1
; CHECK:    ttdpfp16ps      %tmm6, %tmm5, %tmm4
; CHECK:    ttransposed     %tmm3, %tmm1

define void @test_amx(i32 %rv32, i64 %stride, i64 %rvalue, i8* %addr1, <4 x float> %xmm) {
; amx-avx512
call <16 x float> @llvm.x86.tilemovei(i8 7, i8 9)
call <16 x float> @llvm.x86.tilemovee(i8 7, i32 %rv32)
call <16 x float> @llvm.x86.tilemovex(i8 7, <4 x float> %xmm)
; amx-transpose
call void @llvm.x86.t2rpntlvw  (i8 1, i8* %addr1, i64 %stride, i64 %rvalue)
call void @llvm.x86.t2rpntlvwt1   (i8 2, i8* %addr1, i64 %stride, i64 %rvalue)
call void @llvm.x86.t2rpntlvwz0(i8 1, i8* %addr1, i64 %stride)
call void @llvm.x86.t2rpntlvwz0t1(i8 2, i8* %addr1, i64 %stride)
call void @llvm.x86.t2rpntlvwz1(i8 1, i8* %addr1, i64 %stride)
call void @llvm.x86.t2rpntlvwz1t1(i8 2, i8* %addr1, i64 %stride)
call void @llvm.x86.ttdpbf16ps(i8 1, i8 2, i8 3)
call void @llvm.x86.ttdpfp16ps(i8 4, i8 5, i8 6)
call void @llvm.x86.ttransposed(i8 1, i8 3)

ret void
}
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; amx-avx512
declare <16 x float> @llvm.x86.tilemovei(i8 %tile0, i8 %tile1)
declare <16 x float> @llvm.x86.tilemovee(i8 %tile0, i32 %rv32)
declare <16 x float> @llvm.x86.tilemovex(i8 %tile0, <4 x float> %xmm)
; amx-transpose
declare void @llvm.x86.t2rpntlvw     (i8 %tile1, i8* %addr1, i64 %stride, i64 %rvalue)
declare void @llvm.x86.t2rpntlvwt1   (i8 %tile1, i8* %addr1, i64 %stride, i64 %rvalue)
declare void @llvm.x86.t2rpntlvwz0(i8 %tile1, i8* %addr1, i64 %stride)
declare void @llvm.x86.t2rpntlvwz0t1(i8 %tile1, i8* %addr1, i64 %stride)
declare void @llvm.x86.t2rpntlvwz1(i8 %tile1, i8* %addr1, i64 %stride)
declare void @llvm.x86.t2rpntlvwz1t1(i8 %tile1, i8* %addr1, i64 %stride)
declare void @llvm.x86.ttdpbf16ps(i8 %tile0, i8 %tile1, i8 %tile2)
declare void @llvm.x86.ttdpfp16ps(i8 %tile0, i8 %tile1, i8 %tile2)
declare void @llvm.x86.ttransposed(i8 %tile0, i8 %tile1)
