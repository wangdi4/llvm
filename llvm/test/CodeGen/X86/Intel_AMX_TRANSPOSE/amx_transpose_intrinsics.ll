; REQUIRES: intel_feature_isa_amx_transpose
; RUN: llc < %s -mtriple=x86_64-unknown-unknown -mattr=+avx512f,+amx-tile,+amx-bf16,+amx-int8,+amx-transpose,+amx-avx512,+amx-memory | FileCheck %s

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

define void @test_tconjtcmmimfp16ps() {
; CHECK-LABEL: test_tconjtcmmimfp16ps:
; CHECK:       # %bb.0:
; CHECK-NEXT:    tconjtcmmimfp16ps %tmm3, %tmm2, %tmm1
; CHECK-NEXT:    retq
  call void @llvm.x86.tconjtcmmimfp16ps(i8 1, i8 2, i8 3)
  ret void
}
declare void @llvm.x86.tconjtcmmimfp16ps(i8 %A, i8 %B, i8 %C)

define void @test_tconjtfp16() {
; CHECK-LABEL: test_tconjtfp16:
; CHECK:       # %bb.0:
; CHECK-NEXT:    tconjtfp16 %tmm2, %tmm1
; CHECK-NEXT:    retq
  call void @llvm.x86.tconjtfp16(i8 1, i8 2)
  ret void
}
declare void @llvm.x86.tconjtfp16(i8 %A, i8 %B)

define void @test_ttcmmimfp16ps() {
; CHECK-LABEL: test_ttcmmimfp16ps:
; CHECK:       # %bb.0:
; CHECK-NEXT:    ttcmmimfp16ps %tmm3, %tmm2, %tmm1
; CHECK-NEXT:    retq
  call void @llvm.x86.ttcmmimfp16ps(i8 1, i8 2, i8 3)
  ret void
}
declare void @llvm.x86.ttcmmimfp16ps(i8 %A, i8 %B, i8 %C)

define void @test_ttcmmrlfp16ps() {
; CHECK-LABEL: test_ttcmmrlfp16ps:
; CHECK:       # %bb.0:
; CHECK-NEXT:    ttcmmrlfp16ps %tmm3, %tmm2, %tmm1
; CHECK-NEXT:    retq
  call void @llvm.x86.ttcmmrlfp16ps(i8 1, i8 2, i8 3)
  ret void
}

declare void @llvm.x86.ttcmmrlfp16ps(i8 %A, i8 %B, i8 %C)

define void @test_amx2(i8* %pointer, i8* %base, i64 %stride) {
; CHECK-LABEL: test_amx2:
; CHECK:       # %bb.0:
; CHECK-NEXT:    vxorps %xmm0, %xmm0, %xmm0
; CHECK-NEXT:    vmovups %zmm0, -{{[0-9]+}}(%rsp)
; CHECK-NEXT:    movb $1, -{{[0-9]+}}(%rsp)
; CHECK-NEXT:    movb $8, -{{[0-9]+}}(%rsp)
; CHECK-NEXT:    movw $8, -{{[0-9]+}}(%rsp)
; CHECK-NEXT:    movb $8, -{{[0-9]+}}(%rsp)
; CHECK-NEXT:    movw $8, -{{[0-9]+}}(%rsp)
; CHECK-NEXT:    movb $8, -{{[0-9]+}}(%rsp)
; CHECK-NEXT:    movw $8, -{{[0-9]+}}(%rsp)
; CHECK-NEXT:    ldtilecfg -{{[0-9]+}}(%rsp)
; CHECK-NEXT:    movw $8, %ax
; CHECK-NEXT:    tileloadd (%rsi,%rdx), %tmm0
; CHECK-NEXT:    tilezero %tmm1
; CHECK-NEXT:    tilezero %tmm2
; CHECK-NEXT:    ttcmmimfp16ps %tmm1, %tmm0, %tmm2
; CHECK-NEXT:    ttcmmrlfp16ps %tmm1, %tmm0, %tmm2
; CHECK-NEXT:    tconjtcmmimfp16ps %tmm1, %tmm0, %tmm2
; CHECK-NEXT:    tconjtfp16 %tmm2, %tmm0
; CHECK-NEXT:    tilestored %tmm0, (%rdi,%rdx)
; CHECK-NEXT:    tilerelease
; CHECK-NEXT:    vzeroupper
; CHECK-NEXT:    retq

  %a = call x86_amx @llvm.x86.tileloadd64.internal(i16 8, i16 8, i8* %base, i64 %stride)
  %b = call x86_amx @llvm.x86.tilezero.internal(i16 8, i16 8)
  %c = call x86_amx @llvm.x86.tilezero.internal(i16 8, i16 8)

  %c1 = call x86_amx @llvm.x86.ttcmmimfp16ps.internal(i16 8, i16 8, i16 8, x86_amx %c, x86_amx %a, x86_amx %b)
  %c2 = call x86_amx @llvm.x86.ttcmmrlfp16ps.internal(i16 8, i16 8, i16 8, x86_amx %c1, x86_amx %a, x86_amx %b)
  %c3 = call x86_amx @llvm.x86.tconjtcmmimfp16ps.internal(i16 8, i16 8, i16 8, x86_amx %c2, x86_amx %a, x86_amx %b)
  %c4 = call x86_amx @llvm.x86.tconjtfp16.internal(i16 8, i16 8, x86_amx %c3)

  call void @llvm.x86.tilestored64.internal(i16 8, i16 8, i8* %pointer, i64 %stride, x86_amx %c4)
  ret void
}

declare x86_amx @llvm.x86.tilezero.internal(i16, i16)
declare x86_amx @llvm.x86.tileloadd64.internal(i16, i16, i8*, i64)
declare void @llvm.x86.tilestored64.internal(i16, i16, i8*, i64, x86_amx)
declare x86_amx @llvm.x86.tconjtcmmimfp16ps.internal(i16, i16, i16, x86_amx, x86_amx, x86_amx)
declare x86_amx @llvm.x86.tconjtfp16.internal(i16, i16, x86_amx)
declare x86_amx @llvm.x86.ttcmmimfp16ps.internal(i16, i16, i16, x86_amx, x86_amx, x86_amx)
declare x86_amx @llvm.x86.ttcmmrlfp16ps.internal(i16, i16, i16, x86_amx, x86_amx, x86_amx)
